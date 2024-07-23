#include "CommonPS.hlsli"

Texture2D renderTex : register(t0);
Texture2DMS<float4, SAMPLE_COUNT> positionTex : register(t1);

cbuffer FogConstantBuffer : register(b2)
{
    float fogDistMin;
    float fogDistMax;
    float fogStrength;
    float dummy;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float3 getFogColor(float3 sunDir, float3 eyeDir)
{
    float sunDirWeight = henyeyGreensteinPhase(sunDir, eyeDir, 0.625);
    
    float3 fogColor = lerp(normalHorizonColor, sunHorizonColor, sunDirWeight);
    if (isUnderWater)
        fogColor = float3(0.12, 0.26, 0.65);
    
    return fogColor;
}

float getFogFactor(float2 screenCoord, uint sampleIndex)
{
    //Beer-Lambert law
    float4 viewPos = positionTex.Load(screenCoord, sampleIndex);
    if (viewPos.w == -1.0)
        viewPos.xyz = float3(0.0, 0.0, 0.0);
    
    float dist = length(viewPos.xyz);
        
    float distFog = saturate((dist - fogDistMin) / (fogDistMax - fogDistMin));
    float fogFactor = exp(-fogStrength * distFog);
    
    return fogFactor;
}

float4 main(vsOutput input) : SV_TARGET
{
    float3 fogColor = getFogColor(sunDir, eyeDir);
    float3 renderColor = renderTex.Sample(linearClampSS, input.texcoord).rgb;
    
    float fogFactor = getFogFactor(input.posProj.xy, 0);
    
    float3 blendColor = lerp(fogColor, renderColor, fogFactor);
    
    return float4(blendColor, 1.0);
}

float4 mainMSAA(vsOutput input) : SV_TARGET
{
    float3 fogColor = getFogColor(sunDir, eyeDir);
    float3 renderColor = renderTex.Sample(linearClampSS, input.texcoord).rgb;
    
    float3 sumColor = float3(0.0, 0.0, 0.0);
    
    [unroll]
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        float fogFactor = getFogFactor(input.posProj.xy, i);
        
        float3 blendColor = lerp(fogColor, renderColor, fogFactor);
        
        sumColor += blendColor;
    }
    
    return float4(sumColor / SAMPLE_COUNT, 1.0);
}