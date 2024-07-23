#ifndef LIGHTING_HLSLI
    #define LIGHTING_HLSLI

#include "CommonPS.hlsli"

float3 SchlickFresnel(float3 F0, float NdotH)
{
    return F0 + (1 - F0) * pow(2, (-5.55473 * (NdotH) - 6.98316) * NdotH);
}

float NdfGGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    return alpha2 / (3.141592 * pow(NdotH * NdotH * (alpha2 - 1) + 1, 2));
}

float SchlickGGX(float NdotI, float NdotO, float roughness)
{
    float k = (roughness + 1) * (roughness + 1) / 8;
    float gl = NdotI / (NdotI * (1 - k) + k);
    float gv = NdotO / (NdotO * (1 - k) + k);
    return gl * gv;
}

float3 getAmbientLighting(float ao, float3 albedo)
{
    // todo
    // 라이팅에 무관하게 "시간"에 의존적
    return ao * albedo * 0.3f;
}

float3 getShadowFactor()
{
    return float3(1.0, 1.0, 1.0);
}

float3 getDirectLighting(float3 normal, float3 position, float3 albedo)
{
    float3 pixelToEye = normalize(-position);
    float3 lightVec = normalize(mul(float4(sunDir, 0.0), view).xyz);
    float3 halfway = normalize(pixelToEye + lightVec);
    
    float NdotI = max(0.0, dot(normal, lightVec));
    float NdotH = max(0.0, dot(normal, halfway));
    float NdotO = max(0.0, dot(normal, pixelToEye));
    
    // todo
    float metallic = 0.0;
    float roughness = 0.8;
    
    const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye))); // HoV
    float D = NdfGGX(NdotH, roughness);
    float3 G = SchlickGGX(NdotI, NdotO, roughness);
    float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
    float3 diffuseBRDF = kd * albedo;
    
    // todo
    float3 shadowFactor = getShadowFactor();
    float3 radiance = normalHorizonColor * shadowFactor; // radiance 값 수정

    return (diffuseBRDF + specularBRDF) * radiance * NdotI;
}

#endif