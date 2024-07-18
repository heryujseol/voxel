#include "Common.hlsli"

Texture2DArray atlasTextureArray : register(t0);
Texture2DMS<float4, 4> msaaRenderTex : register(t1);
Texture2D mirrorWorldTex : register(t2);
Texture2D depthMapTex : register(t3);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION1;
    sample float3 posModel : POSITION2;
    uint face : FACE;
    uint type : TYPE;
    uint sampleIndex : SV_SampleIndex;
};

float3 schlickFresnel(float3 N, float3 E, float3 R)
{
    // https://en.wikipedia.org/wiki/Schlick%27s_approximation
    // [f0 ~ 1]
    // 90 -> dot(N,E)==0 -> f0+(1-f0)*1^5 -> 1
    //  0 -> dot(N,E)==1 -> f0+(1-f0)*0*5 -> f0
    return R + (1 - R) * pow((1 - max(dot(N, E), 0.0)), 5.0);
}

float4 main(vsOutput input) : SV_TARGET
{
    float3 normal = getNormal(input.face);
    if (normal.y <= 0 || input.posWorld.y < 62.0 - 1e-4 || 62.0 + 1e-4 < input.posWorld.y)
        discard;
        
    float2 texcoord = getVoxelTexcoord(input.posModel, input.face);
    uint index = (input.type - 1) * 6 + input.face;
    
    // absorption color
    float4 textureColor = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, index));
    
    float width, height, lod;
    depthMapTex.GetDimensions(0, width, height, lod);
    float2 screenTexcoord = float2(input.posProj.x / width, input.posProj.y / height);
        
    // origin render color
    float3 originColor = msaaRenderTex.Load(input.posProj.xy, input.sampleIndex).rgb;
   
    if (isUnderWater)
    {
        return float4(lerp(originColor, textureColor.rgb, 0.5), 1.0);
    }
    else
    {
        // absorption factor
        float depth = depthMapTex.Sample(linearClampSS, screenTexcoord).r;
        float objectDistance = length(convertViewPos(screenTexcoord, depth));
        float planeDistance = length(eyePos - input.posWorld);
        float diffDistance = abs(objectDistance - planeDistance);
        float absorptionCoeff = 0.1;
        float absorptionFactor = 1.0 - exp(-absorptionCoeff * diffDistance); // beer-lambert
    
        float3 projColor = lerp(originColor, textureColor.rgb, absorptionFactor);
    
    // reflect color
        float4 mirrorColor = mirrorWorldTex.Sample(linearClampSS, screenTexcoord);
        
    // fresnel factor
        float3 toEye = normalize(eyePos - input.posWorld);
        float3 reflectCoeff = float3(0.2, 0.2, 0.2);
        float3 fresnelFactor = schlickFresnel(normal, toEye, reflectCoeff);
        
    // blending 3 colors
        projColor *= (1.0 - fresnelFactor);
        float3 blendColor = lerp(projColor, mirrorColor.rgb, fresnelFactor);
        
    // alpha blend
        float fresnelAvg = dot(fresnelFactor, float3(1, 1, 1)) / 3.0;
        float alpha = lerp(1.0, mirrorColor.a, fresnelAvg);
        
        return float4(blendColor, alpha);
    }
}