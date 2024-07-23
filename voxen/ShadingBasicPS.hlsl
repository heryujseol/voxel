#include "CommonPS.hlsli"

Texture2DMS<float4, SAMPLE_COUNT> normalTex : register(t0);
Texture2DMS<float4, SAMPLE_COUNT> positionTex : register(t1);
Texture2DMS<float4, SAMPLE_COUNT> albedoEdgeTex : register(t2);
Texture2D ssaoTex : register(t3);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(vsOutput input) : SV_TARGET
{
    float3 normal = normalTex.Load(input.posProj.xy, 0).xyz;
    float3 position = positionTex.Load(input.posProj.xy, 0).xyz;
    float3 albedo = float3(0.0, 0.0, 0.0);
    albedo += albedoEdgeTex.Load(input.posProj.xy, 0).rgb;
    albedo += albedoEdgeTex.Load(input.posProj.xy, 1).rgb;
    albedo += albedoEdgeTex.Load(input.posProj.xy, 2).rgb;
    albedo += albedoEdgeTex.Load(input.posProj.xy, 3).rgb;
    albedo /= SAMPLE_COUNT;
    float ao = ssaoTex.Sample(pointClampSS, input.texcoord).r;
    
    // todo
    float metallic = 0.0;
    float roughness = 0.8;
    
    float3 ambientLighting = getAmbientLighting(ao, albedo);
    float3 directLighting = getDirectLighting(normal, position, albedo, metallic, roughness);
    
    float3 lighting = ambientLighting + directLighting;
    float3 clampLighting = clamp(lighting, 0.0f, 1000.0f);
    
    return float4(clampLighting, 1.0);
}

float4 mainMSAA(vsOutput input) : SV_TARGET
{   
    float3 sumClampLighting = float3(0.0, 0.0, 0.0);
    
    [unroll]
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        // point clamp를 이용해서 albedo를 구성했기 때문에 다른 샘플사이에서 다른 컬러를 사용할 가능성이 높음
        // sampleWeight를 사용하지 않음
        float3 normal = normalTex.Load(input.posProj.xy, i).xyz;
        float3 position = positionTex.Load(input.posProj.xy, i).xyz;
        float3 albedo = albedoEdgeTex.Load(input.posProj.xy, i).rgb; 
        float ao = ssaoTex.Sample(pointClampSS, input.texcoord).r;
        
        // todo
        float metallic = 0.0;
        float roughness = 0.8;
        
        float3 ambientLighting = getAmbientLighting(ao, albedo);
        float3 directLighting = getDirectLighting(normal, position, albedo, metallic, roughness);
        
        float3 lighting = ambientLighting + directLighting;
        float3 clampLighting = clamp(lighting, 0.0f, 1000.0f);
        
        sumClampLighting += clampLighting;
    }
    
    return float4(sumClampLighting / SAMPLE_COUNT, 1.0);
}