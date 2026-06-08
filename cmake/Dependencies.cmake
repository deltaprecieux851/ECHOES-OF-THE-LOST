# DirectX 12 ships with the Windows SDK (MSVC toolchains).
if(WIN32)
    find_package(DirectX12 CONFIG QUIET)
endif()

find_package(Vulkan QUIET)
find_package(Bullet QUIET)
find_package(nlohmann_json CONFIG QUIET)

function(eol_link_optional target)
    if(TARGET Vulkan::Vulkan)
        target_link_libraries(${target} PRIVATE Vulkan::Vulkan)
        target_compile_definitions(${target} PRIVATE EOL_HAS_VULKAN=1)
    endif()

    if(TARGET Bullet::Bullet)
        target_link_libraries(${target} PRIVATE Bullet::Bullet)
        target_compile_definitions(${target} PRIVATE EOL_HAS_BULLET=1)
    elseif(TARGET Bullet3Common)
        target_link_libraries(${target} PRIVATE Bullet3Common LinearMath)
        target_compile_definitions(${target} PRIVATE EOL_HAS_BULLET=1)
    endif()

    if(TARGET nlohmann_json::nlohmann_json)
        target_link_libraries(${target} PRIVATE nlohmann_json::nlohmann_json)
        target_compile_definitions(${target} PRIVATE EOL_HAS_JSON=1)
    endif()

    if(WIN32)
        target_link_libraries(${target} PRIVATE
            d3d12
            dxgi
            d3dcompiler
            dxguid
        )
        target_compile_definitions(${target} PRIVATE EOL_HAS_DX12=1)
    endif()
endfunction()
