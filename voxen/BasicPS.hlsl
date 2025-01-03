#include "Common.hlsli"

Texture2DArray atlasTextureArray : register(t0);
Texture2D grassColorMap : register(t1);
Texture2D foliageColorMap : register(t2);
Texture2D climateNoiseMap : register(t3);
Texture2D mirrorDepthTex : register(t4);

struct psInput
{
    float4 posProj : SV_POSITION;
    sample float3 posWorld : POSITION;
    sample float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint texIndex : INDEX;
};

struct psOutput
{
    float4 normalEdge : SV_Target0;
    float4 position : SV_Target1;
    float4 albedo : SV_Target2;
    uint coverage : SV_Target3;
};

// TODO : Ư���� TEXTURE ���� -> grass, foliage, side overlay
bool useGrassColor(uint texIndex)
{
    return texIndex <= 2;
}

bool useDirtOverlay(uint texIndex)
{
    return texIndex == 2;
}

bool useFoliageColor(uint texIndex)
{
    // TODO
    return texIndex == 128;
}

float4 getAlbedo(float2 texcoord, uint texIndex, float3 worldPos, float3 normal)
{
    float4 albedo = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, texIndex));
    
    if (useGrassColor(texIndex))
    {
        float3 faceBiasPos = -normal * 1e-4; 
        // normal vector�� �ݴ�������� shrink
        // bias�� ������� ������ depthFighting���� ȿ���� ��Ÿ��
        // 1.0000001, 0.99999999�� ���� ���� �ٸ� ����̱� ����
        float2 diffOffsetPos = floor(worldPos.xz + faceBiasPos.xz) - floor(eyePos.xz);
                
        float texelSize = 1.0 / (CHUNK_COUNT * CHUNK_SIZE);
        float2 climateTexcoord = float2(0.5 + diffOffsetPos.x * texelSize, 0.5 - diffOffsetPos.y * texelSize);
        climateTexcoord += float2(texelSize * 0.5, texelSize * 0.5);
        // texelSize * 0.5 ��ŭ �����ִ� ����
        // �ؽ��İ� 4x4�� ���¿��� �ؽ��� ��ǥ�� 0.5, 0.5��� (2, 2)�� �߰����� ���ø��ؾ� ��
        // �׷��� ������ diffOffsetPos�� ���� ������ ���ؼ� ���ݸ� ���ص� �ٸ� �ؼ��� ���ø��ϰ� ��
        
        float2 th = climateNoiseMap.SampleLevel(pointClampSS, climateTexcoord, 0.0).rg;
        
        float3 grassColor = grassColorMap.SampleLevel(pointClampSS, float2(th.x, 1.0 - th.y), 0.0).rgb;
        albedo.rgb *= grassColor;
    }
    
    if (useDirtOverlay(texIndex))
    {
        float4 dirt = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, 3));
        albedo = lerp(dirt, albedo, albedo.a);
    }
    
    if (useFoliageColor(texIndex))
    {
        // TODO
    }
    
    return albedo;
}

psOutput
    main(psInput
    input, 
    uint coverage : SV_COVERAGE, uint sampleIndex : SV_SampleIndex)
{
#ifdef USE_ALPHA_CLIP 
    if (atlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, input.texIndex), 0.0).a != 1.0)
        discard;
    
    [unroll]
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        float2 offsets[SAMPLE_COUNT] = { float2(0, -1), float2(0, 1), float2(-1, 0), float2(1, 0) };
        // �ֺ��� alpha clip�̶�� coverage�� ���� ���� ���� �ε����� ����
        // ��Ȯ�� coverage���� �ƴ�
        // SSAO������ coverage�� ���� weight�� �ΰ� ���������� weight�� ��� 1�� ���¶�� ���� ��
        // Lighting������ coverage ���� ���� �׳� 4�� ������ -> albedo�� �ٸ��� ����
        if (atlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, input.texIndex), 0.0, offsets[i]).a != 1.0)
        {
            coverage = (1 << sampleIndex);
        }
    }
#endif
    
    psOutput output;
    
    bool edge = (coverage != 0xf); // 0b1111 -> 1111�� �𼭸��� �ƴ� �ȼ���
    
    output.normalEdge = float4(normalize(input.normal), float(edge));
    
    output.position = float4(input.posWorld, 1.0);
    
    output.coverage = coverage;
    
    output.albedo = getAlbedo(input.texcoord, input.texIndex, input.posWorld, input.normal);
    
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

    if (pixelDepth <= planeDepth) // �ſﺸ�� ����� �̷������ �ʿ� ����
        discard;
    
    float4 albedo = getAlbedo(input.texcoord, input.texIndex, input.posWorld, input.normal);
    
    float3 ambient = getAmbientLighting(1.0, albedo.rgb, input.normal);
    
    return float4(ambient, albedo.a);
}