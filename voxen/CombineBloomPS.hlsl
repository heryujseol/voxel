#include "Common.hlsli"

Texture2D renderTex : register(t0);
Texture2D bloomTex : register(t1);

struct psInput
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

float4 main(psInput input) : SV_TARGET
{
    float3 renderColor = renderTex.Sample(linearClampSS, input.texcoord).rgb;
    float3 bloomColor = bloomTex.Sample(linearClampSS, input.texcoord).rgb;
    
    float scattering = min(henyeyGreensteinPhase(lightDir, eyeDir, 0.95), 1.0) * 0.3;
    float bloomStrength = scattering + (isUnderWater ? 0.4 : 0.1);
    
    float3 combineColor = lerp(renderColor, bloomColor, bloomStrength);
    
    return float4(linearToneMapping(combineColor, 1.0), 1.0);
}