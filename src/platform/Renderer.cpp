#include "platform/Renderer.h"

#include "core/Logger.h"
#include "platform/Window.h"

#ifdef EOL_HAS_DX12

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <filesystem>
#include <cstring>

using Microsoft::WRL::ComPtr;

namespace echoes::platform {

namespace {

constexpr UINT kFrameCount = 2;
constexpr UINT kCubeIndexCount = 36;

struct Vertex {
    float position[3];
    float normal[3];
    float color[4];
};

struct alignas(256) SceneConstants {
    float viewProjection[16];
    float world[16];
    float color[4];
};

struct Dx12State {
    ComPtr<IDXGIFactory6> factory;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12Resource> renderTargets[kFrameCount];
    ComPtr<ID3D12CommandAllocator> commandAllocators[kFrameCount];
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    D3D12_INDEX_BUFFER_VIEW indexBufferView{};
    ComPtr<ID3D12Resource> constantBuffers[kFrameCount];
    void* constantBufferMapped[kFrameCount]{};
    ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent{nullptr};
    UINT64 fenceValues[kFrameCount]{};
    UINT rtvDescriptorSize{0};
    UINT frameIndex{0};
};

std::wstring GetShaderPath() {
    wchar_t exePath[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::filesystem::path path(exePath);
    path = path.parent_path() / "assets" / "shaders" / "basic.hlsl";
    return path.wstring();
}

void CopyMat4(float* dst, const math::Mat4& src) {
    std::memcpy(dst, src.m, sizeof(src.m));
}

void SetVertex(Vertex& v, float px, float py, float pz,
               float nx, float ny, float nz,
               float r, float g, float b) {
    v.position[0] = px;
    v.position[1] = py;
    v.position[2] = pz;
    v.normal[0] = nx;
    v.normal[1] = ny;
    v.normal[2] = nz;
    v.color[0] = r;
    v.color[1] = g;
    v.color[2] = b;
    v.color[3] = 1.0f;
}

}  // namespace

Renderer::Renderer(Window& window) : window_(window), dx_(new Dx12State()) {}
Renderer::~Renderer() { Shutdown(); }

float Renderer::GetAspectRatio() const {
    const int h = window_.GetHeight();
    if (h <= 0) return 16.0f / 9.0f;
    return static_cast<float>(window_.GetWidth()) / static_cast<float>(h);
}

bool Renderer::Initialize() {
    if (!CreateDevice()) return false;
    if (!CreateSwapChain()) return false;
    if (!CreateRenderTargets()) return false;
    if (!CreatePipeline()) return false;
    if (!CreateCubeGeometry()) return false;

    if (FAILED(dx_->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dx_->fence)))) {
        return false;
    }

    dx_->fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!dx_->fenceEvent) {
        core::Logger::Log(core::LogLevel::Error, "Failed to create GPU fence event.");
        return false;
    }

    initialized_ = true;
    core::Logger::Log(core::LogLevel::Info, "DirectX 12 renderer initialized.");
    return true;
}

bool Renderer::CreateDevice() {
    UINT flags = 0;
#ifdef _DEBUG
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    if (FAILED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&dx_->factory)))) {
        core::Logger::Log(core::LogLevel::Error, "CreateDXGIFactory2 failed.");
        return false;
    }

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; dx_->factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc{};
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dx_->device)))) {
            break;
        }
    }

    if (!dx_->device) {
        core::Logger::Log(core::LogLevel::Error, "D3D12CreateDevice failed.");
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (FAILED(dx_->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&dx_->commandQueue)))) {
        return false;
    }

    return true;
}

bool Renderer::CreateSwapChain() {
    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = static_cast<UINT>(window_.GetWidth());
    desc.Height = static_cast<UINT>(window_.GetHeight());
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = kFrameCount;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(dx_->factory->CreateSwapChainForHwnd(
            dx_->commandQueue.Get(),
            window_.GetHandle(),
            &desc,
            nullptr,
            nullptr,
            &swapChain1))) {
        return false;
    }

    dx_->factory->MakeWindowAssociation(window_.GetHandle(), DXGI_MWA_NO_ALT_ENTER);
    swapChain1.As(&dx_->swapChain);
    dx_->frameIndex = dx_->swapChain->GetCurrentBackBufferIndex();
    return true;
}

bool Renderer::CreateRenderTargets() {
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc{};
    rtvDesc.NumDescriptors = kFrameCount;
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(dx_->device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&dx_->rtvHeap)))) {
        return false;
    }

    dx_->rtvDescriptorSize = dx_->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dx_->rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < kFrameCount; ++i) {
        if (FAILED(dx_->swapChain->GetBuffer(i, IID_PPV_ARGS(&dx_->renderTargets[i])))) {
            return false;
        }
        dx_->device->CreateRenderTargetView(dx_->renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += dx_->rtvDescriptorSize;

        if (FAILED(dx_->device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&dx_->commandAllocators[i])))) {
            return false;
        }

        D3D12_HEAP_PROPERTIES heapProps{};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        D3D12_RESOURCE_DESC cbDesc{};
        cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        cbDesc.Width = 256;
        cbDesc.Height = 1;
        cbDesc.DepthOrArraySize = 1;
        cbDesc.MipLevels = 1;
        cbDesc.SampleDesc.Count = 1;
        cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        if (FAILED(dx_->device->CreateCommittedResource(
                &heapProps, D3D12_HEAP_FLAG_NONE, &cbDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&dx_->constantBuffers[i])))) {
            return false;
        }

        D3D12_RANGE readRange{0, 0};
        dx_->constantBuffers[i]->Map(0, &readRange, &dx_->constantBufferMapped[i]);
    }

    if (FAILED(dx_->device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            dx_->commandAllocators[dx_->frameIndex].Get(),
            nullptr, IID_PPV_ARGS(&dx_->commandList)))) {
        return false;
    }
    dx_->commandList->Close();
    return true;
}

bool Renderer::CreatePipeline() {
    const std::wstring shaderPath = GetShaderPath();
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> errorBlob;

    const UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS
#ifdef _DEBUG
        | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION
#endif
        ;

    if (FAILED(D3DCompileFromFile(
            shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "VSMain", "vs_5_0", compileFlags, 0,
            &vsBlob, &errorBlob))) {
        if (errorBlob) {
            core::Logger::Log(core::LogLevel::Error, static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        return false;
    }

    if (FAILED(D3DCompileFromFile(
            shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "PSMain", "ps_5_0", compileFlags, 0,
            &psBlob, &errorBlob))) {
        return false;
    }

    D3D12_ROOT_PARAMETER rootParam{};
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParam.Descriptor.ShaderRegister = 0;
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootDesc{};
    rootDesc.NumParameters = 1;
    rootDesc.pParameters = &rootParam;
    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> sigBlob;
    ComPtr<ID3DBlob> sigError;
    if (FAILED(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &sigError))) {
        return false;
    }

    if (FAILED(dx_->device->CreateRootSignature(
            0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(),
            IID_PPV_ARGS(&dx_->rootSignature)))) {
        return false;
    }

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = dx_->rootSignature.Get();
    psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
    psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.InputLayout = {inputLayout, _countof(inputLayout)};
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    if (FAILED(dx_->device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&dx_->pipelineState)))) {
        return false;
    }

    return true;
}

bool Renderer::CreateCubeGeometry() {
    Vertex vertices[24]{};
    const float r = 0.8f, g = 0.8f, b = 0.85f;

    SetVertex(vertices[0],  -0.5f, -0.5f,  0.5f,  0,  0,  1, r, g, b);
    SetVertex(vertices[1],   0.5f, -0.5f,  0.5f,  0,  0,  1, r, g, b);
    SetVertex(vertices[2],   0.5f,  0.5f,  0.5f,  0,  0,  1, r, g, b);
    SetVertex(vertices[3],  -0.5f,  0.5f,  0.5f,  0,  0,  1, r, g, b);

    SetVertex(vertices[4],   0.5f, -0.5f, -0.5f,  0,  0, -1, r, g, b);
    SetVertex(vertices[5],  -0.5f, -0.5f, -0.5f,  0,  0, -1, r, g, b);
    SetVertex(vertices[6],  -0.5f,  0.5f, -0.5f,  0,  0, -1, r, g, b);
    SetVertex(vertices[7],   0.5f,  0.5f, -0.5f,  0,  0, -1, r, g, b);

    SetVertex(vertices[8],  -0.5f, -0.5f, -0.5f, -1,  0,  0, r, g, b);
    SetVertex(vertices[9],  -0.5f, -0.5f,  0.5f, -1,  0,  0, r, g, b);
    SetVertex(vertices[10], -0.5f,  0.5f,  0.5f, -1,  0,  0, r, g, b);
    SetVertex(vertices[11], -0.5f,  0.5f, -0.5f, -1,  0,  0, r, g, b);

    SetVertex(vertices[12],  0.5f, -0.5f,  0.5f,  1,  0,  0, r, g, b);
    SetVertex(vertices[13],  0.5f, -0.5f, -0.5f,  1,  0,  0, r, g, b);
    SetVertex(vertices[14],  0.5f,  0.5f, -0.5f,  1,  0,  0, r, g, b);
    SetVertex(vertices[15],  0.5f,  0.5f,  0.5f,  1,  0,  0, r, g, b);

    SetVertex(vertices[16], -0.5f,  0.5f,  0.5f,  0,  1,  0, r, g, b);
    SetVertex(vertices[17],  0.5f,  0.5f,  0.5f,  0,  1,  0, r, g, b);
    SetVertex(vertices[18],  0.5f,  0.5f, -0.5f,  0,  1,  0, r, g, b);
    SetVertex(vertices[19], -0.5f,  0.5f, -0.5f,  0,  1,  0, r, g, b);

    SetVertex(vertices[20], -0.5f, -0.5f, -0.5f,  0, -1,  0, r, g, b);
    SetVertex(vertices[21],  0.5f, -0.5f, -0.5f,  0, -1,  0, r, g, b);
    SetVertex(vertices[22],  0.5f, -0.5f,  0.5f,  0, -1,  0, r, g, b);
    SetVertex(vertices[23], -0.5f, -0.5f,  0.5f,  0, -1,  0, r, g, b);

    const UINT16 indices[kCubeIndexCount] = {
        0,1,2, 0,2,3,
        4,6,5, 4,7,6,
        8,10,9, 8,11,10,
        12,13,14, 12,14,15,
        16,18,17, 16,19,18,
        20,21,22, 20,22,23
    };

    const UINT vbSize = sizeof(vertices);
    const UINT ibSize = sizeof(indices);

    D3D12_HEAP_PROPERTIES uploadHeap{};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC vbDesc{};
    vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vbDesc.Width = vbSize;
    vbDesc.Height = 1;
    vbDesc.DepthOrArraySize = 1;
    vbDesc.MipLevels = 1;
    vbDesc.SampleDesc.Count = 1;
    vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    if (FAILED(dx_->device->CreateCommittedResource(
            &uploadHeap, D3D12_HEAP_FLAG_NONE, &vbDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&dx_->vertexBuffer)))) {
        return false;
    }

    void* mapped = nullptr;
    D3D12_RANGE readRange{0, 0};
    dx_->vertexBuffer->Map(0, &readRange, &mapped);
    std::memcpy(mapped, vertices, vbSize);
    dx_->vertexBuffer->Unmap(0, nullptr);

    dx_->vertexBufferView.BufferLocation = dx_->vertexBuffer->GetGPUVirtualAddress();
    dx_->vertexBufferView.SizeInBytes = vbSize;
    dx_->vertexBufferView.StrideInBytes = sizeof(Vertex);

    D3D12_RESOURCE_DESC ibDesc = vbDesc;
    ibDesc.Width = ibSize;
    if (FAILED(dx_->device->CreateCommittedResource(
            &uploadHeap, D3D12_HEAP_FLAG_NONE, &ibDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&dx_->indexBuffer)))) {
        return false;
    }

    dx_->indexBuffer->Map(0, &readRange, &mapped);
    std::memcpy(mapped, indices, ibSize);
    dx_->indexBuffer->Unmap(0, nullptr);

    dx_->indexBufferView.BufferLocation = dx_->indexBuffer->GetGPUVirtualAddress();
    dx_->indexBufferView.SizeInBytes = ibSize;
    dx_->indexBufferView.Format = DXGI_FORMAT_R16_UINT;

    return true;
}

void Renderer::WaitForGpu() {
    const UINT64 signalValue = dx_->fenceValues[dx_->frameIndex]++;
    dx_->commandQueue->Signal(dx_->fence.Get(), signalValue);

    if (dx_->fence->GetCompletedValue() < signalValue) {
        dx_->fence->SetEventOnCompletion(signalValue, dx_->fenceEvent);
        WaitForSingleObject(dx_->fenceEvent, INFINITE);
    }
}

void Renderer::MoveToNextFrame() {
    const UINT64 currentFence = dx_->fenceValues[dx_->frameIndex];
    dx_->commandQueue->Signal(dx_->fence.Get(), currentFence);

    dx_->frameIndex = dx_->swapChain->GetCurrentBackBufferIndex();

    if (dx_->fence->GetCompletedValue() < dx_->fenceValues[dx_->frameIndex]) {
        dx_->fence->SetEventOnCompletion(dx_->fenceValues[dx_->frameIndex], dx_->fenceEvent);
        WaitForSingleObject(dx_->fenceEvent, INFINITE);
    }

    dx_->fenceValues[dx_->frameIndex] = currentFence + 1;
}

void Renderer::BeginFrame(const math::Mat4& viewProjection) {
    if (!initialized_) return;

    viewProjection_ = viewProjection;
    drawQueue_.clear();

    dx_->commandAllocators[dx_->frameIndex]->Reset();
    dx_->commandList->Reset(dx_->commandAllocators[dx_->frameIndex].Get(), dx_->pipelineState.Get());

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = dx_->renderTargets[dx_->frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    dx_->commandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = dx_->rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtv.ptr += dx_->frameIndex * dx_->rtvDescriptorSize;

    const float clearColor[] = {0.05f, 0.08f, 0.12f, 1.0f};
    dx_->commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    dx_->commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

    D3D12_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(window_.GetWidth());
    viewport.Height = static_cast<float>(window_.GetHeight());
    viewport.MaxDepth = 1.0f;
    D3D12_RECT scissor{0, 0, window_.GetWidth(), window_.GetHeight()};
    dx_->commandList->RSSetViewports(1, &viewport);
    dx_->commandList->RSSetScissorRects(1, &scissor);

    dx_->commandList->SetGraphicsRootSignature(dx_->rootSignature.Get());
    dx_->commandList->SetPipelineState(dx_->pipelineState.Get());
    dx_->commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dx_->commandList->IASetVertexBuffers(0, 1, &dx_->vertexBufferView);
    dx_->commandList->IASetIndexBuffer(&dx_->indexBufferView);
}

void Renderer::DrawBox(const math::Vec3& pos, const math::Vec3& scale, const RenderColor& color) {
    drawQueue_.push_back({pos, scale, color});
}

void Renderer::DrawWaterPlane(float waterLevel, float extent) {
    DrawBox({0.0f, waterLevel, 0.0f}, {extent, 0.15f, extent}, {0.1f, 0.35f, 0.55f, 0.85f});
}

void Renderer::DrawTempleScene() {
    DrawBox({0.0f, -0.5f, 0.0f}, {40.0f, 1.0f, 40.0f}, {0.45f, 0.38f, 0.30f, 1.0f});

    for (int i = -3; i <= 3; ++i) {
        DrawBox({static_cast<float>(i * 4), 2.0f, -8.0f}, {1.2f, 4.0f, 1.2f}, {0.62f, 0.55f, 0.42f, 1.0f});
        DrawBox({static_cast<float>(i * 4), 2.0f, 8.0f}, {1.2f, 4.0f, 1.2f}, {0.62f, 0.55f, 0.42f, 1.0f});
    }

    DrawBox({0.0f, 3.5f, 0.0f}, {18.0f, 0.8f, 18.0f}, {0.55f, 0.48f, 0.38f, 1.0f});
    DrawBox({0.0f, 5.0f, -12.0f}, {8.0f, 6.0f, 2.0f}, {0.50f, 0.42f, 0.35f, 1.0f});

    DrawBox({-10.0f, 0.3f, 5.0f}, {6.0f, 0.6f, 20.0f}, {0.30f, 0.45f, 0.55f, 1.0f});
    DrawBox({10.0f, 0.3f, 5.0f}, {6.0f, 0.6f, 20.0f}, {0.30f, 0.45f, 0.55f, 1.0f});

    DrawBox({-6.0f, 3.0f, -2.0f}, {0.5f, 6.0f, 8.0f}, {0.40f, 0.35f, 0.30f, 1.0f});
    DrawBox({6.0f, 3.0f, -2.0f}, {0.5f, 6.0f, 8.0f}, {0.40f, 0.35f, 0.30f, 1.0f});

    DrawBox({0.0f, 1.5f, 14.0f}, {4.0f, 3.0f, 2.0f}, {0.35f, 0.30f, 0.28f, 1.0f});
}

void Renderer::FlushDrawQueue() {
    if (!initialized_ || drawQueue_.empty()) return;

    for (const DrawInstance& inst : drawQueue_) {
        const math::Mat4 world =
            math::Mat4::Translation(inst.position.x, inst.position.y, inst.position.z) *
            math::Mat4::Scale(inst.scale.x, inst.scale.y, inst.scale.z);

        SceneConstants cb{};
        CopyMat4(cb.viewProjection, viewProjection_);
        CopyMat4(cb.world, world);
        cb.color[0] = inst.color.r;
        cb.color[1] = inst.color.g;
        cb.color[2] = inst.color.b;
        cb.color[3] = inst.color.a;

        std::memcpy(dx_->constantBufferMapped[dx_->frameIndex], &cb, sizeof(cb));

        D3D12_GPU_VIRTUAL_ADDRESS cbAddress =
            dx_->constantBuffers[dx_->frameIndex]->GetGPUVirtualAddress();
        dx_->commandList->SetGraphicsRootConstantBufferView(0, cbAddress);
        dx_->commandList->DrawIndexedInstanced(kCubeIndexCount, 1, 0, 0, 0);
    }
}

void Renderer::EndFrame() {
    if (!initialized_) return;

    FlushDrawQueue();

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = dx_->renderTargets[dx_->frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    dx_->commandList->ResourceBarrier(1, &barrier);

    dx_->commandList->Close();
    ID3D12CommandList* lists[] = {dx_->commandList.Get()};
    dx_->commandQueue->ExecuteCommandLists(1, lists);
    dx_->swapChain->Present(1, 0);
    MoveToNextFrame();
}

void Renderer::Shutdown() {
    if (!dx_) return;

    if (initialized_) {
        WaitForGpu();
    }

    for (UINT i = 0; i < kFrameCount; ++i) {
        if (dx_->constantBuffers[i] && dx_->constantBufferMapped[i]) {
            dx_->constantBuffers[i]->Unmap(0, nullptr);
            dx_->constantBufferMapped[i] = nullptr;
        }
    }

    if (dx_->fenceEvent) {
        CloseHandle(dx_->fenceEvent);
        dx_->fenceEvent = nullptr;
    }

    delete dx_;
    dx_ = nullptr;
    initialized_ = false;
}

#else

Renderer::Renderer(Window& window) : window_(window) {}
Renderer::~Renderer() { Shutdown(); }

float Renderer::GetAspectRatio() const { return 16.0f / 9.0f; }

bool Renderer::Initialize() {
    core::Logger::Log(core::LogLevel::Warning, "DirectX 12 not available on this platform.");
    return false;
}

void Renderer::Shutdown() { initialized_ = false; }
void Renderer::BeginFrame(const math::Mat4&) {}
void Renderer::EndFrame() {}
void Renderer::DrawBox(const math::Vec3&, const math::Vec3&, const RenderColor&) {}
void Renderer::DrawWaterPlane(float, float) {}
void Renderer::DrawTempleScene() {}
void Renderer::FlushDrawQueue() {}

#endif

}  // namespace echoes::platform
