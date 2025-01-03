#include "Common.hlsli"

Texture2DArray atlasTextureArray : register(t0);
Texture2DMS<float4, SAMPLE_COUNT> msaaRenderTex : register(t1);
Texture2D mirrorWorldTex : register(t2);
Texture2DMS<float4, SAMPLE_COUNT> positionTex : register(t3);
Texture2D waterColorMapTex : register(t4);
Texture2D climateNoiseMapTex : register(t5);

struct psInput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint texIndex : INDEX;
};

float3 schlickFresnel(float3 N, float3 E, float3 R)
{
    // https://en.wikipedia.org/wiki/Schlick%27s_approximation
    // [f0 ~ 1]
    // 90 -> dot(N,E)==0 -> f0+(1-f0)*1^5 -> 1
    //  0 -> dot(N,E)==1 -> f0+(1-f0)*0*5 -> f0
    return R + (1 - R) * pow((1 - max(dot(N, E), 0.0)), 5.0);
}

float3 getWaterAlbedo(float2 texcoord, uint texIndex, float3 worldPos, float3 normal)
{
    float3 albedo = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, texIndex)).rgb;
    
    float3 faceBiasPos = -normal * 1e-4;
    
    float2 diffOffsetPos = floor(worldPos.xz + faceBiasPos.xz) - floor(eyePos.xz);
    
    float texelSize = 1.0 / (CHUNK_COUNT * CHUNK_SIZE);
    float2 climateTexcoord = float2(0.5 + diffOffsetPos.x * texelSize, 0.5 - diffOffsetPos.y * texelSize);
    climateTexcoord += float2(texelSize * 0.5, texelSize * 0.5);
    
    float2 th = climateNoiseMapTex.SampleLevel(pointClampSS, climateTexcoord, 0.0);
    
    float3 waterColor = waterColorMapTex.SampleLevel(pointClampSS, float2(th.x, 1.0 - th.y), 0.0).rgb;
    
    albedo *= waterColor;
    
    return albedo;
}

float4 main(psInput input, uint sampleIndex : SV_SampleIndex) : SV_TARGET
{
    float3 normal = input.normal;
    if (normal.y <= 0 || input.posWorld.y < 64.0 - 1e-4 || 64.0 + 1e-4 < input.posWorld.y)
        discard;
    
    // absorption color
    float3 albedo = getWaterAlbedo(input.texcoord, input.texIndex, input.posWorld, normal);
    
    float3 ambientLighting = getAmbientLighting(1.0, albedo, input.normal);
    
    float3 directLighting = getDirectLighting(input.normal, input.posWorld, albedo, 0.0, 0.05, true);
    
    float3 waterColor = ambientLighting + directLighting;
    
    float2 screenTexcoord = float2(input.posProj.x / appWidth, input.posProj.y / appHeight);
        
    // origin render color
    float3 originColor = msaaRenderTex.Load(input.posProj.xy, sampleIndex).rgb;
   
    if (isUnderWater)
    {
        return float4(lerp(originColor, waterColor, 0.5), 1.0);
    }
    else
    {
        // absorption factor
        float3 originPosition = positionTex.Load(input.posProj.xy, sampleIndex).xyz;
        float objectDistance = length(eyePos - originPosition);
        float planeDistance = length(eyePos - input.posWorld);
        float diffDistance = abs(objectDistance - planeDistance);
        float absorptionCoeff = 0.125;
        float absorptionFactor = 1.0 - exp(-absorptionCoeff * diffDistance); // beer-lambert
    
        float3 projColor = lerp(originColor, waterColor, absorptionFactor);
    
        // reflect color
        float3 mirrorColor = mirrorWorldTex.Sample(linearClampSS, screenTexcoord).rgb;
        
        // fresnel factor
        float3 toEye = normalize(eyePos - input.posWorld);
        float3 reflectCoeff = float3(0.2, 0.2, 0.2);
        float3 fresnelFactor = schlickFresnel(normal, toEye, reflectCoeff);
        
        // blending 3 colors
        projColor *= (1.0 - fresnelFactor);
        float3 blendColor = lerp(projColor, mirrorColor, fresnelFactor);
        
        return float4(blendColor, 1.0);
    }
}