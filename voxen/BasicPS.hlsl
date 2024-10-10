#include "Common.hlsli"

Texture2DArray atlasTextureArray : register(t0);
Texture2D grassColorMap : register(t1);
Texture2D foliageColorMap : register(t2);
Texture2D mirrorDepthTex : register(t3);

struct psInput
{
    float4 posProj : SV_POSITION;
    sample float3 posWorld : POSITION;
    sample float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint texIndex : INDEX;
    uint biome : BIOME;
};

struct psOutput
{
    float4 normalEdge : SV_Target0;
    float4 position : SV_Target1;
    float4 albedo : SV_Target2;
    uint coverage : SV_Target3;
};

#define BIOME_TYPE_PLAINS   0
#define BIOME_TYPE_JUNGLE   1
#define BIOME_TYPE_DESERT   2
#define BIOME_TYPE_SWAMP    3
#define BIOME_TYPE_FOREST   4
#define BIOME_TYPE_SAVANNA  5
#define BIOME_TYPE_TAIGA    6
#define BIOME_TYPE_TUNDRA   7
#define BIOME_TYPE_BADLAND  8

void getTemperatureAndDownfall(uint biome, out float temperature, out float downfall)
{
    if (biome == BIOME_TYPE_PLAINS)
    {
        temperature = 0.8;
        downfall = 0.4;
    }
    else if (biome == BIOME_TYPE_JUNGLE)
    {
        temperature = 0.95;
        downfall = 0.9;
    }
    else if (biome == BIOME_TYPE_DESERT)
    {
        temperature = 1.0; // 2.0?
        downfall = 0.0;
    }
    else if (biome == BIOME_TYPE_SWAMP)
    {
        temperature = 0.8;
        downfall = 0.9;
    }
    else if (biome == BIOME_TYPE_FOREST)
    {
        temperature = 0.7;
        downfall = 0.8;
    }
    else if (biome == BIOME_TYPE_SAVANNA)
    {
        temperature = 1.0; // 2.0?
        downfall = 0.0;
    }
    else if (biome == BIOME_TYPE_TAIGA)
    {
        temperature = 0.25;
        downfall = 0.8;
    }
    else if (biome == BIOME_TYPE_TUNDRA)
    {
        temperature = 0.0;
        downfall = 0.5;
    }
    else if (biome == BIOME_TYPE_BADLAND)
    {
        temperature = 1.0;
        downfall = 0.0;
    }
    else
    {
        temperature = 0.5;
        downfall = 0.5;
    }
}

void getGrassColor(uint biome, float temperature, float downfall, out float3 grassColor)
{
    
}

void getFoliageColor(uint biome, float temperature, float downfall, out float3 foliageColor)
{
    
}

void getWaterColor(uint biome)
{
    
}

bool useColorMap(uint index)
{
    return true;
}

psOutput main(psInput input, uint coverage : SV_COVERAGE, uint sampleIndex : SV_SampleIndex)
{
#ifdef USE_ALPHA_CLIP 
    if (atlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, input.texIndex), 0.0).a != 1.0)
        discard;
    
    [unroll]
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        float2 offsets[SAMPLE_COUNT] = { float2(0, -1), float2(0, 1), float2(-1, 0), float2(1, 0) };
        // 주변이 alpha clip이라면 coverage를 직접 본인 샘플 인덱스로 설정
        // 정확한 coverage값은 아님
        // SSAO에서는 coverage에 따라 weight를 두고 연산하지만 weight가 모두 1인 상태라고 보면 됨
        // Lighting에서는 coverage 구분 없이 그냥 4번 연산함 -> albedo가 다르기 때문
        if (atlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, input.texIndex), 0.0, offsets[i]).a != 1.0)
        {
            coverage = (1 << sampleIndex);
        }
    }
#endif
    
    psOutput output;
    
    bool edge = (coverage != 0xf); // 0b1111 -> 1111은 모서리가 아닌 픽셀임
    
    output.normalEdge = float4(normalize(input.normal), float(edge));
    
    output.position = float4(input.posWorld, 1.0);
    
    output.coverage = coverage;
    
    float4 albedo = atlasTextureArray.Sample(pointWrapSS, float3(input.texcoord, input.texIndex));
    
    // check texIndex -> Grass와 같은 흑백 강도를 표현하는 인덱스
    if (useColorMap(input.texIndex))
    {
        float temperature, downfall;
        getTemperatureAndDownfall(input.biome, temperature, downfall);
        
        float4 grassColor = grassColorMap.SampleLevel(pointClampSS, float2(1.0 - temperature, 1.0 - temperature * downfall), 0.0);
        
        albedo *= grassColor;
    }
    else
    {
        
    }
    
    output.albedo = float4(albedo);
    
    return output;
}

float4 mainMirror(psInput input) : SV_TARGET
{
#ifdef USE_ALPHA_CLIP 
    if (atlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, input.texIndex), 0.0).a != 1.0)
        discard;
#endif
    
    float2 screenTexcoord = float2(input.posProj.x / mirrorWidth, input.posProj.y / mirrorHeight);
    float planeDepth = mirrorDepthTex.Sample(linearClampSS, screenTexcoord).r;
    float pixelDepth = input.posProj.z;

    if (pixelDepth <= planeDepth) // 거울보다 가까운 미러월드는 필요 없음
        discard;
    
    float4 albedo = atlasTextureArray.Sample(pointWrapSS, float3(input.texcoord, input.texIndex));
    float3 ambient = getAmbientLighting(1.0, albedo.rgb, input.normal);
    
    return float4(ambient, albedo.a);
}