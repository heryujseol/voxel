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
};

struct psOutput
{
    float4 normalEdge : SV_Target0;
    float4 position : SV_Target1;
    float4 albedo : SV_Target2;
    uint coverage : SV_Target3;
};

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