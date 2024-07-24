#include "CommonPS.hlsli"

Texture2D renderTex : register(t0);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float3 linearToneMapping(float3 color, float exposure)
{
    float3 invGamma = float3(1, 1, 1) / 2.2;
    
    color = clamp(exposure * color, 0.0, 1.0);
    color = pow(color, invGamma);
    
    return color;
}

float4 main(vsOutput input) : SV_TARGET
{
    float3 color = renderTex.Sample(pointClampSS, input.texcoord).rgb;
    
    return float4(linearToneMapping(color, 1.0), 1.0);
}