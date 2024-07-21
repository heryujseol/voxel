#include "Common.hlsli"

Texture2DArray atlasTextureArray : register(t0);
Texture2D grassColorMap : register(t1);

#ifdef USE_DEPTH_CLIP
    Texture2D depthTex : register(t2);
#endif

Texture2D ssaoTex : register(t3);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION1;
    sample float3 posModel : POSITION2;
    uint face : FACE;
    uint type : TYPE;
    uint sampleIndex : SV_SampleIndex;
};

float3 getFaceColor(uint face)
{
    if (face == 0 || face == 1)
    {
        return float3(0.81, 0.81, 0.81);
    }
    else if (face == 4 || face == 5)
    {
        return float3(0.87, 0.87, 0.87);
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
    
    float2 MSAAOffsets[4] =
    {
        float2(-0.25, -0.25),
        float2(0.25, -0.25),
        float2(-0.25, 0.25),
        float2(0.25, 0.25)
    };
    
    input.posProj.xy += MSAAOffsets[input.sampleIndex];
    float2 msaaScreenTex = float2(input.posProj.x / 1920.0, input.posProj.y / 1080.0);
    float ao = ssaoTex.Sample(pointClampSS, msaaScreenTex);
    
    float3 albedo = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, index)).rgb;

    float3 normal = getNormal(input.face);
    normal = mul(float4(normal, 0.0), view);
    
    float3 ambient = 0.3 * albedo * ao;
    float3 lighting = ambient;
    
    float3 viewPos = mul(float4(input.posWorld, 1.0), view); // sample?
    float3 viewDir = normalize(-viewPos);
    
    // diffuse
    float3 lightDir = sunDir;
    lightDir = mul(float4(lightDir, 0.0), view);
    float3 diffuse = max(dot(normal, lightDir), 0.0) * albedo;
    
    // specular
    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    float3 specular = float3(spec, spec, spec);
    
    lighting += diffuse + specular;
    
    return float4(lighting, 1.0);
}