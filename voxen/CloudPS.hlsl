#include "Common.hlsli"

cbuffer CloudConstantBuffer : register(b0)
{
    matrix world;
    float3 volumeColor;
    float cloudScale;
}

struct psInput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    uint face : FACE;
};

float4 main(psInput input) : SV_TARGET
{   
    // �Ÿ��� �ָ� horizon color ���� 
    float distance = length(input.posWorld.xz - eyePos.xz);
    float horizonWeight = smoothstep(maxRenderDistance, cloudScale, clamp(distance, maxRenderDistance, cloudScale));
    float3 albedo = volumeColor;
    
    // �ٶ󺸴� ���⿡ ���� anisotropy 
    float sunAniso = max(dot(lightDir, eyeDir), 0.0);
    float3 eyeHorizonColor = lerp(normalHorizonColor, sunHorizonColor, sunAniso);
    albedo = lerp(albedo, eyeHorizonColor, horizonWeight);
    
    // ambient lighting
    float3 normal = getNormal(input.face);
    float3 ambientLighting = getAmbientLighting(1.0, albedo);
    
    // direct lighting
    float3 directLighting = getDirectLighting(normal, input.posWorld, albedo, 0.0, 0.96, false);
    
    // distance alpha
    float alphaWeight = smoothstep(maxRenderDistance, cloudScale, clamp(distance, maxRenderDistance, cloudScale));
    float alpha = (1.0 - alphaWeight) * 0.75; // [0, 0.75]
    
    return float4(ambientLighting + directLighting, alpha);
}