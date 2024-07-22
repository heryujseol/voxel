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

float3 getAmbientLighting(float2 texcoord, float3 albedo)
{
    float ambientOcclusion = ssaoTex.Sample(pointClampSS, texcoord);

    return ambientOcclusion * albedo * 0.3f;
}

float3 getShadowFactor()
{
    return float3(1.0, 1.0, 1.0);
}

float3 getDirectLighting(float2 screencoord, uint sampleIndex, float3 albedo)
{
    float3 normal = normalTex.Load(screencoord, sampleIndex).xyz;
    float3 position = positionTex.Load(screencoord, sampleIndex).xyz;
    
    float3 pixelToEye = normalize(-position);
    float3 lightVec = normalize(mul(float4(sunDir, 0.0), view).xyz);
    float3 halfway = normalize(pixelToEye + lightVec);
    
    float NdotI = max(0.0, dot(normal, lightVec));
    float NdotH = max(0.0, dot(normal, halfway));
    float NdotO = max(0.0, dot(normal, pixelToEye));
    
    // need to sample on tex
    float metallic = 0.0;
    float roughness = 0.8;
    
    const float3 Fdielectric = 0.04; // ��ݼ�(Dielectric) ������ F0
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye))); // HoV
    float D = NdfGGX(NdotH, roughness);
    float3 G = SchlickGGX(NdotI, NdotO, roughness);
    float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
    float3 diffuseBRDF = kd * albedo;
    
    float3 shadowFactor = getShadowFactor();
    float3 radiance = normalZenithColor * shadowFactor;

    return (diffuseBRDF + specularBRDF) * radiance * NdotI;
}

float3 linearToneMapping(float3 color)
{
    float3 invGamma = float3(1, 1, 1) / 2.2;

    float exposure = 1.0f;
    
    color = clamp(exposure * color, 0.0, 1.0);
    color = pow(color, invGamma);
    
    return color;
}

float4 main(vsOutput input) : SV_TARGET
{
    float3 albedo = float3(0.0, 0.0, 0.0);
    albedo += albedoEdgeTex.Load(input.posProj.xy, 0).rgb;
    albedo += albedoEdgeTex.Load(input.posProj.xy, 1).rgb;
    albedo += albedoEdgeTex.Load(input.posProj.xy, 2).rgb;
    albedo += albedoEdgeTex.Load(input.posProj.xy, 3).rgb;
    albedo /= SAMPLE_COUNT;
    
    albedo = pow(albedo, 2.2); // temp albedo gamma correction
    
    float3 ambientLighting = getAmbientLighting(input.texcoord, albedo);
    float3 directLighting = getDirectLighting(input.posProj.xy, 0, albedo);
    
    float3 lighting = ambientLighting + directLighting;
    float3 clampLighting = clamp(lighting, 0.0f, 1000.0f);
    
    //return float4(clampLighting, 1.0);
    
    return float4(linearToneMapping(clampLighting), 1.0);
}

float4 mainMSAA(vsOutput input) : SV_TARGET
{
    if (cameraDummyData.x == 0)
        return main(input);
    
    float3 sumClampLighting = float3(0.0, 0.0, 0.0);
    
    [unroll]
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        // point clamp�� �̿��ؼ� albedo�� �����߱� ������ �ٸ� ���û��̿��� �ٸ� �÷��� ����� ���ɼ��� ����
        // sampleWeight�� ������� ����
        float3 albedo = albedoEdgeTex.Load(input.posProj.xy, i); 
        albedo = pow(albedo, 2.2); // temp albedo gamma correction
        
        float3 ambientLighting = getAmbientLighting(input.texcoord, albedo);
        float3 directLighting = getDirectLighting(input.posProj.xy, i, albedo);
        
        float3 lighting = ambientLighting + directLighting;
        float3 clampLighting = clamp(lighting, 0.0f, 1000.0f);
        
        sumClampLighting += clampLighting;
    }
    
    //return float4(sumClampLighting / SAMPLE_COUNT, 1.0);
    
    return float4(linearToneMapping(sumClampLighting / SAMPLE_COUNT), 1.0);
}

/*
1. ���ø��� �߰� ������ -> �ؽ��� ���� �ʿ䰡 ����
roughness
metalic

2. �¾��� �� ����
- �¾��� ���� GUI �����Ͽ� ����
 -> Lighting ������ � ���� radiance�� ������� �����ϸ� ��

3. linearToneMapping�� ����� ������
��ó�� �� ����ؾ������� ������ ���� �׽���
- exposure �� 1.0
- ������ 2.2 ����

4. ���� albedo�� unorm Texture�� f16��� �����ϰ� �ӽ� gamma correction�� ���� ��
- ������ �ؽ��İ� f16 ������ HDR �ؽ��Ķ�� ���� �� �ʿ� ����
 -> �ӽ� ����: unorm -> gamma correction���� fp�� �ٿ� -> ���� ��� ���� �� tone mapping���� �ٽ� gamma correction

*/