cbuffer SceneConstants : register(b0)
{
    float4x4 viewProjection;
    float4x4 world;
    float4 color;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float4 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
    float3 normal   : NORMAL;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    float4 worldPos = mul(float4(input.position, 1.0f), world);
    output.position = mul(worldPos, viewProjection);
    output.color = input.color * color;
    output.normal = input.normal;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 lightDir = normalize(float3(0.4f, 0.9f, 0.2f));
    float ndotl = saturate(dot(normalize(input.normal), lightDir));
    float ambient = 0.25f;
    float3 lit = input.color.rgb * (ambient + ndotl * 0.75f);
    return float4(lit, input.color.a);
}
