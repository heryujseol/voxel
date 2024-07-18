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

struct psOutput
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float depth : SV_TARGET2;
};

psOutput main(vsOutput input)
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
    
    psOutput output;
    
    float3 color = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, index)).rgb;
    output.albedo = float4(color, 1.0);
    
    // must be [Normal * ITWorld * ITView]
    float3 normalWorld = getNormal(input.face);
    float3 normalView = mul(float4(normalWorld, 0.0), invTrasposeView).xyz;
    output.normal = float4(normalize(normalView), 0.0);
    
    output.depth = input.posProj.z;
    
    return output;
}