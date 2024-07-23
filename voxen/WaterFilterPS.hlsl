#include "CommonPS.hlsli"

Texture2D renderTex : register(t0);

cbuffer WaterFilterConstantBuffer : register(b2)
{
    float3 filterColor;
    float filterStrength;
}

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(vsOutput input) : SV_TARGET
{
    float3 renderColor = renderTex.Sample(linearClampSS, input.texcoord).rgb;

    float3 blendColor = lerp(renderColor, filterColor, filterStrength);
    
    return float4(blendColor, 1.0);
}