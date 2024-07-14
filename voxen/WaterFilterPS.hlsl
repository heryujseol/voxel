#include "Common.hlsli"

Texture2DMS<float4, 4> msaaRenderTex : register(t0);

cbuffer WaterFilterConstantBuffer : register(b2)
{
    float3 filterColor;
    float blendStrength;
}

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
    uint sampleIndex : SV_SampleIndex;
};

float4 main(vsOutput input) : SV_TARGET
{
    float3 renderColor = msaaRenderTex.Load(input.posProj.xy, input.sampleIndex).rgb;

    return float4(lerp(renderColor, filterColor, blendStrength), 1.0);
}