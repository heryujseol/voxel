#include "CommonPS.hlsli"

Texture2D renderTex : register(t0);
Texture2D bloomTex : register(t1);

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
    float3 renderColor = renderTex.Sample(linearClampSS, input.texcoord).rgb;
    float3 bloomColor = bloomTex.Sample(linearClampSS, input.texcoord).rgb;
    
    float3 combineColor = lerp(renderColor, bloomColor, isUnderWater ? 0.5 : 0.1); // strength Á¶Àý    
    
    float3 retColor = linearToneMapping(combineColor, 1.0);

    return float4(retColor, 1.0);
}