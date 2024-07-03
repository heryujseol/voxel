#include "Common.hlsli"

Texture2DArray atlasTextureArray : register(t0);
Texture2D grassColorMap : register(t1);

#ifdef USE_DEPTH_CLIP
    Texture2D depthTex : register(t2);
#endif

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION1;
    sample float3 posModel : POSITION2;
    uint face : FACE;
    uint type : TYPE;
};

float4 main(vsOutput input) : SV_TARGET
{
    //float temperature = 0.5;
    //float downfall = 1.0;
    //float4 biome = grassColorMap.SampleLevel(pointClampSS, float2(1 - temperature, 1 - temperature / downfall), 0.0);
    
    float2 texcoord = getVoxelTexcoord(input.posModel, input.face);
    uint index = (input.type - 1) * 6 + input.face;
    
#ifdef USE_ALPHA_CLIP 
    if (atlasTextureArray.SampleLevel(pointWrapSS, float3(texcoord, index), 0.0).a != 1.0)
        discard;
#endif
    
#ifdef USE_DEPTH_CLIP
    float width, height, lod;
    depthTex.GetDimensions(0, width, height, lod);
    
    float2 screenTexcoord = float2(input.posProj.x / width, input.posProj.y / height);
    float planeDepth = depthTex.Sample(linearClampSS, screenTexcoord).r;
    float pixelDepth = input.posProj.z;

    if (pixelDepth < planeDepth)
    {
        discard;
    }
#endif
    
    float3 color = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, index)).rgb * 0.3;
    
    float3 normal = getNormal(input.face);
    
    float ndotl = max(dot(sunDir, normal), 0.0);
    
    float strength = sunStrength;
    
    if (13700 <= dateTime && dateTime <= 14700)
    {
        float w = (dateTime - 13700) / 1000.0;
        color = lerp(color * (strength + 1.0) * (ndotl + 1.0), color, w);
    }
    else if (21300 <= dateTime && dateTime <= 22300)
    {
        float w = (dateTime - 21300) / 1000.0;
        color = lerp(color, color * (strength + 1.0) * (ndotl + 1.0), w);
    }
    else if (14700 < dateTime && dateTime < 21300)
    {
        color = color * (strength + 1.0);
    }
    else
    {
        color = color * (strength + 1.0) * (ndotl + 1.0);
    }

    return float4(color, 1.0);
}