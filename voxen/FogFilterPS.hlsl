#include "Common.hlsli"

Texture2DMS<float4, 4> msaaRenderTex : register(t0);
Texture2DMS<float, 4> msaaDepthTex : register(t1);

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
    uint sampleIndex : SV_SampleIndex;
};

float4 texcoordToView(float2 texcoord, float2 screenCoord, uint sampleIndex)
{
    float4 posProj;

    // [0, 1]x[0, 1] -> [-1, 1]x[-1, 1]
    posProj.xy = texcoord * 2.0 - 1.0;
    posProj.y *= -1;
    posProj.z = msaaDepthTex.Load(screenCoord, sampleIndex).r;
    posProj.w = 1.0;

    // ProjectSpace -> ViewSpace
    float4 posView = mul(posProj, invProj);
    posView.xyz /= posView.w; // homogeneous coordinates
    
    return posView;
}

float4 main(vsOutput input) : SV_TARGET
{
    float sunDirWeight = henyeyGreensteinPhase(sunDir, eyeDir, 0.625);
    float3 fogColor = lerp(normalHorizonColor, sunHorizonColor, sunDirWeight);
    if (isUnderWater)
    {
        fogColor = float3(0.12, 0.26, 0.65);
    }
    
    /*
    float3 fogColor = isUnderWater ? float3(1.0, 1.0, 1.0) : normalHorizonColor;
    if (!isUnderWater && (0 <= dateTime && dateTime <= 1000) || (11000 <= dateTime && dateTime <= 13700) || (22300 <= dateTime && dateTime <= 23999))
    {
        float sunDirWeight = henyeyGreensteinPhase(sunDir, eyeDir, 0.625);
        fogColor = lerp(normalHorizonColor, sunHorizonColor, sunDirWeight);
    }*/
    
    //Beer-Lambert law
    float4 posView = texcoordToView(input.texcoord, input.posProj.xy, input.sampleIndex);
    float dist = length(posView.xyz);
        
    float distFog = saturate((dist - fogDistMin) / (fogDistMax - fogDistMin));
    float fogFactor = exp(-fogStrength * distFog);
        
    float3 color = msaaRenderTex.Load(input.posProj.xy, input.sampleIndex).rgb;
    
    color = lerp(fogColor, color, fogFactor);
    
    return float4(color, 1.0);
}