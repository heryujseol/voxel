#include "Common.hlsli"

Texture2DArray atlasTextureArray : register(t0);
Texture2DMS<float4, SAMPLE_COUNT> msaaRenderTex : register(t1);
Texture2D mirrorWorldTex : register(t2);
Texture2DMS<float4, SAMPLE_COUNT> positionTex : register(t3);

struct psInput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint type : TYPE;
};

float3 schlickFresnel(float3 N, float3 E, float3 R)
{
    // https://en.wikipedia.org/wiki/Schlick%27s_approximation
    // [f0 ~ 1]
    // 90 -> dot(N,E)==0 -> f0+(1-f0)*1^5 -> 1
    //  0 -> dot(N,E)==1 -> f0+(1-f0)*0*5 -> f0
    return R + (1 - R) * pow((1 - max(dot(N, E), 0.0)), 5.0);
}

float4 main(psInput input, uint sampleIndex : SV_SampleIndex) : SV_TARGET
{
    float3 normal = input.normal;
    if (normal.y <= 0 || input.posWorld.y < 62.0 - 1e-4 || 62.0 + 1e-4 < input.posWorld.y)
        discard;
    
    // absorption color
    float3 textureColor = atlasTextureArray.Sample(pointWrapSS, float3(input.texcoord, 0)).rgb;
    float3 ambientLighting = getAmbientLighting(1.0, textureColor, input.normal);
    
    float3 directLighting = getDirectLighting(input.normal, input.posWorld, textureColor, 0.0, 0.05);
    
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
        float absorptionCoeff = 0.1;
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