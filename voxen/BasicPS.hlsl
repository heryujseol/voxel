#include "Common.hlsli"

Texture2DArray blockAtlasTextureArray : register(t0);
Texture2DArray normalAtlasTextureArray : register(t1);
Texture2DArray merAtlasTextureArray : register(t2);
Texture2D grassColorMap : register(t3);
Texture2D foliageColorMap : register(t4);
Texture2D climateNoiseMap : register(t5);
Texture2D mirrorDepthTex : register(t6);

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
    float4 mer : SV_Target4;
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
    float4 albedo = blockAtlasTextureArray.Sample(pointWrapSS, float3(texcoord, texIndex));
    
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
        float4 dirt = blockAtlasTextureArray.Sample(pointWrapSS, float3(texcoord, 3));
        albedo = lerp(dirt, albedo, albedo.a);
    }
    
    if (useFoliageColor(texIndex))
    {
        // TODO
    }
    
    return albedo;
}

float3 getTangent(float3 normal)
{
    if (normal.x > 0 && normal.y == 0 && normal.z == 0)
    {
        return float3(0.0, 0.0, 1.0);
    }
    else if (normal.x < 0 && normal.y == 0 && normal.z == 0)
    {
        return float3(0.0, 0.0, -1.0);
    }
    else if (normal.y < 0 && normal.x == 0 && normal.z == 0)
    {
        return float3(1.0, 0.0, 0.0);
    }
    else if (normal.y > 0 && normal.x == 0 && normal.z == 0)
    {
        return float3(1.0, 0.0, 0.0);
    }
    else if (normal.z < 0 && normal.x == 0 && normal.y == 0)
    {
        return float3(1.0, 0.0, 0.0);

    }
    else if (normal.z > 0 && normal.x == 0 && normal.y == 0)
    {
        return float3(-1.0, 0.0, 0.0);
    }
    else
    {
        return float3(0.0, 0.0, 0.0);
    }
}

float3 normalMapping(float2 texcoord, uint texIndex, float3 normal)
{
    float3 normalTex = normalAtlasTextureArray.Sample(pointWrapSS, float3(texcoord, texIndex)).rgb;
    normalTex = normalize(2.0 * normalTex - 1.0); // TBN �����̽��� �����ϴ� TBN ��ǥ
    
    // ���� ��ǥ�� �������� TBN���� ����
    float3 N = normal; // N` 
    float3 T = getTangent(normal); // T`
    float3 B = cross(N, T); // B`
        
    float3x3 TBN = float3x3(T, B, N); // T`B`N`
    
    // Review
    // ���� ��ǥ�� �������� ������ TBN�࿡ TBN�����̽� ��ǥ�� ���ϸ� ���� ��ǥ
    // ���������� ���� �غ��� ��
    return normalize(mul(normalTex, TBN)); // TS * TBN * M --> W
}

psOutput
    main(psInput
    input, 
    uint coverage : SV_COVERAGE, uint sampleIndex : SV_SampleIndex)
{
#ifdef USE_ALPHA_CLIP 
    if (blockAtlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, input.texIndex), 0.0).a != 1.0)
        discard;
    
    [unroll]
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        float2 offsets[SAMPLE_COUNT] = { float2(0, -1), float2(0, 1), float2(-1, 0), float2(1, 0) };
        // �ֺ��� alpha clip�̶�� coverage�� ���� ���� ���� �ε����� ����
        // ��Ȯ�� coverage���� �ƴ�
        // SSAO������ coverage�� ���� weight�� �ΰ� ���������� weight�� ��� 1�� ���¶�� ���� ��
        // Lighting������ coverage ���� ���� �׳� 4�� ������ -> albedo�� �ٸ��� ����
        if (blockAtlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, input.texIndex), 0.0, offsets[i]).a != 1.0)
        {
            coverage = (1 << sampleIndex);
        }
    }
#endif
    
    psOutput output;
    
    bool edge = (coverage != 0xf); // 0b1111 -> 1111�� �𼭸��� �ƴ� �ȼ���
    
    float3 normal = normalMapping(input.texcoord, input.texIndex, input.normal);
    if (cameraDummyData.x == 0)
    {
        normal = normalize(input.normal);
    }
    
    output.normalEdge = float4(normalize(normal), float(edge));
    
    output.position = float4(input.posWorld, 1.0);
    
    output.coverage = coverage;
    
    output.albedo = getAlbedo(input.texcoord, input.texIndex, input.posWorld, input.normal);
    
    output.mer = merAtlasTextureArray.Sample(pointWrapSS, float3(input.texcoord, input.texIndex));
    //output.mer = float4(1.0, 0.0, 0.2, 1);
    return output;
}

float4 mainMirror(psInput input) : SV_TARGET
{
#ifdef USE_ALPHA_CLIP 
    if (blockAtlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, input.texIndex), 0.0).a != 1.0)
        discard;
#endif
    
    float2 screenTexcoord = float2(input.posProj.x / mirrorWidth, input.posProj.y / mirrorHeight);
    float planeDepth = mirrorDepthTex.Sample(linearClampSS, screenTexcoord).r;
    float pixelDepth = input.posProj.z;

    if (pixelDepth <= planeDepth) // �ſﺸ�� ����� �̷������ �ʿ� ����
        discard;
    
    float4 albedo = getAlbedo(input.texcoord, input.texIndex, input.posWorld, input.normal);
    
    float3 mer = merAtlasTextureArray.Sample(pointWrapSS, float3(input.texcoord, input.texIndex)).rgb;
    
    float3 ambient = getAmbientLighting(1.0, albedo.rgb, input.posWorld, input.normal, mer.r, mer.b);
    
    return float4(ambient, albedo.a);
}