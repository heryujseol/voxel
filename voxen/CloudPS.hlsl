#include "Common.hlsli"

struct vsOutput
{
    float4 posProj : SV_Position;
    float3 posWorld : POSITION;
    uint face : FACE;
};

cbuffer CloudConstantBuffer : register(b2)
{
    matrix world;
    float3 volumeColor;
    float cloudScale;
}

float3 getFaceColor(uint face)
{
    if (face == 0 || face == 1)
    {
        return float3(0.95, 0.95, 0.95);
    }
    else if (face == 4 || face == 5)
    {
        return float3(0.9, 0.9, 0.9);
    }
    else if (face == 3)
    {
        return float3(1.0, 1.0, 1.0);
    }
    else
    {
        return float3(0.75, 0.75, 0.75);
    }
}

float4 main(vsOutput input) : SV_TARGET
{
    float distance = length(input.posWorld.xz - eyePos.xz);
    
    float sunDirWeight = henyeyGreensteinPhase(sunDir, eyeDir, 0.625);
    float3 horizonColor = lerp(normalHorizonColor, sunHorizonColor, sunDirWeight);
    
    // 거리가 멀면 horizon color 선택 
    float horizonWeight = smoothstep(maxRenderDistance, cloudScale, clamp(distance, maxRenderDistance, cloudScale));
    float3 color = volumeColor * getFaceColor(input.face);
    color = lerp(color, horizonColor, horizonWeight);
    
    float sunAltitude = sin(sunDir.y);
    float dayAltitude = PI / 12.0;
    float nightAltitude = -PI * 0.5 * (1.7 / 6.0);
    float maxHorizonAltitude = -PI / 24.0;
    if (dayAltitude < sunAltitude)
    {
        color *= float3(1.0, 1.0, 1.0);
    }
    else if (maxHorizonAltitude < sunAltitude && sunAltitude <= dayAltitude)
    {
        color *= lerp(horizonColor, float3(1, 1, 1), smoothstep(maxHorizonAltitude, dayAltitude, sunAltitude));
    }
    else if (nightAltitude < sunAltitude && sunAltitude <= maxHorizonAltitude)
    {
        color *= lerp(float3(0.04, 0.05, 0.09), horizonColor, smoothstep(nightAltitude, maxHorizonAltitude, sunAltitude));
    }
    else // nightAltitude
    {
        color *= float3(0.04, 0.05, 0.09);
    }
    
    // distance alpha
    float alphaWeight = smoothstep(maxRenderDistance, cloudScale, clamp(distance, maxRenderDistance, cloudScale));
    float alpha = (1.0 - alphaWeight) * 0.75; // [0, 0.8]
    
    return float4(color, alpha);
}