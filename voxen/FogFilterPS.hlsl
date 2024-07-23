#include "CommonPS.hlsli"

Texture2DMS<float4, SAMPLE_COUNT> renderTex : register(t0);
Texture2DMS<float, SAMPLE_COUNT> depthTex : register(t1);

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

float getFogFactor(float3 pos)
{
    //Beer-Lambert law
    float dist = length(pos.xyz);
        
    float distFog = saturate((dist - fogDistMin) / (fogDistMax - fogDistMin));
    float fogFactor = exp(-fogStrength * distFog);
    
    return fogFactor;
} 

float4 main(vsOutput input, uint sampleIndex : SV_SampleIndex) : SV_TARGET
{
    float3 fogColor = getFogColor(sunDir, eyeDir);
    float3 renderColor = renderTex.Load(input.posProj.xy, sampleIndex).rgb;
    
    float depth = depthTex.Load(input.posProj.xy, sampleIndex).r;
    float3 viewPos = texcoordToViewPos(input.texcoord, depth);
    
    float fogFactor = getFogFactor(viewPos);
    
    float3 blendColor = lerp(fogColor, renderColor, fogFactor);
    
    return float4(blendColor, 1.0);
}
