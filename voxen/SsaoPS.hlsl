#include "CommonPS.hlsli"

Texture2DMS<float4, SAMPLE_COUNT> normalTex : register(t0);
Texture2DMS<float4, SAMPLE_COUNT> positionTex : register(t1);
Texture2DMS<uint, SAMPLE_COUNT> coverageTex : register(t2);

cbuffer SsaoConstantBuffer : register(b2)
{
    float4 sampleKernel[64];
}

cbuffer SsaoNoiseConstantBuffer : register(b3)
{
    float4 rotationNoise[16];
}

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

uint getClosestSampleIndex(float2 screenCoord)
{
    float2 fracCoord = frac(screenCoord);
    
    return fracCoord.x < 0.5 ? ((fracCoord.y < 0.5) ? 0 : 1) : ((fracCoord.y < 0.5) ? 2 : 3);
}

uint4 coverageAnalysis(uint4 coverage)
{
    uint4 sampleWeight = uint4(1, 1, 1, 1);
    
    if (coverage.x == coverage.y) { ++sampleWeight.x; coverage.y = 0;}
    if (coverage.x == coverage.z) { ++sampleWeight.x; coverage.z = 0;}
    if (coverage.x == coverage.w) { ++sampleWeight.x; coverage.w = 0;}
    if (coverage.y == coverage.z) { ++sampleWeight.y; coverage.z = 0;}
    if (coverage.y == coverage.w) { ++sampleWeight.y; coverage.w = 0;}
    if (coverage.z == coverage.w) { ++sampleWeight.z; coverage.w = 0;}

    // 재조정 : coverage가 0인 것은 마스킹이 안된 샘플이거나, 같은 마스킹이 있는 경우
    sampleWeight.x = (coverage.x > 0) ? sampleWeight.x : 0;
    sampleWeight.y = (coverage.y > 0) ? sampleWeight.y : 0;
    sampleWeight.z = (coverage.z > 0) ? sampleWeight.z : 0;
    sampleWeight.w = (coverage.w > 0) ? sampleWeight.w : 0;
    
    return sampleWeight;
}

float getOcclusionFactor(float2 pos, float3 viewPos, float3 normal)
{
    // 200배 확대한 것을 frac연산으로 다시 하나씩 200개로 쪼갬 -> 4로 곱하여 인덱스로 사용
    // PointWrap 샘플러라고 생각
    float fx = frac(pos.x * 200.0) * 4.0; // [0,4]
    float fy = frac(pos.y * 200.0) * 4.0; // [0,4]
    uint ix = uint(floor(fx)) % 4;
    uint iy = uint(floor(fy)) % 4;
    float3 randomVec = rotationNoise[ix + 4 * iy].xyz;
 
    float3 T = normalize(randomVec - normal * dot(normal, randomVec)); // R - proj.n(R)
    float3 B = cross(normal, T);
    float3x3 TBN = float3x3(T, B, normal);
    
    float occlusionFactor = 0.0;
    float radius = 1.0;
    float bias = 0.025;
    
    [unroll]
    for (uint i = 0; i < 64; ++i)
    {
        float3 sampleOffset = mul(sampleKernel[i].xyz, TBN);
        float3 samplePos = viewPos + sampleOffset * radius;
        
        float4 sampleProjPos = float4(samplePos, 1.0);
        sampleProjPos = mul(sampleProjPos, proj);
        sampleProjPos.xyz /= sampleProjPos.w; // [-1, 1]
        
        float2 sampleTexcoord = sampleProjPos.xy;
        sampleTexcoord.x = saturate(sampleTexcoord.x * 0.5 + 0.5); // [-1, 1] -> [0, 1]
        sampleTexcoord.y = saturate(-(sampleTexcoord.y * 0.5) + 0.5); // [-1, 1] -> [1, 0]
        
        float width, height, sampleCount;
        positionTex.GetDimensions(width, height, sampleCount);
        
        float2 sampleScreenCoord = float2(sampleTexcoord.x * (width - 1.0) + 0.5, sampleTexcoord.y * (height - 1.0) + 0.5);
        float closestSampleIndex = getClosestSampleIndex(sampleScreenCoord);
        
        float4 storedViewPos = positionTex.Load(sampleScreenCoord, closestSampleIndex);
        
        float w = smoothstep(0.0, 1.0, radius / length(viewPos - storedViewPos.xyz));
        float rangeCheck = pow(w, 3.0);
        
        occlusionFactor += (storedViewPos.z < samplePos.z - bias ? 1.0 : 0.0) * rangeCheck;
    }
    
    return occlusionFactor / 64.0;
}

float main(vsOutput input) : SV_TARGET
{   
    float3 normal = normalTex.Load(input.posProj.xy, 0).xyz;
    if (length(normal) == 0)
        return 1.0;
    normal = normalize(normal);
    
    float3 viewPos = positionTex.Load(input.posProj.xy, 0).xyz;
    
    float occlusionFactor = getOcclusionFactor(input.texcoord, viewPos, normal);
    
    return 1.0 - occlusionFactor;
}

float mainMSAA(vsOutput input) : SV_TARGET
{
    uint4 coverage;
    coverage.x = coverageTex.Load(input.posProj.xy, 0);
    coverage.y = coverageTex.Load(input.posProj.xy, 1);
    coverage.z = coverageTex.Load(input.posProj.xy, 2);
    coverage.w = coverageTex.Load(input.posProj.xy, 3);
    
    uint4 sampleWeight = coverageAnalysis(coverage);
    uint sampleWeightArray[4] = { sampleWeight.x, sampleWeight.y, sampleWeight.z, sampleWeight.w };
    
    float sumOcclusionFactor = 0.0;
   
    // dont use [unroll] -> continue statement
    [loop]
    for (uint i = 0; i < SAMPLE_COUNT; ++i) // loop max 4
    {
        if (sampleWeightArray[i] == 0)
            continue;
        
        float3 normal = normalTex.Load(input.posProj.xy, i).xyz;
        if (length(normal) == 0)
            return 1.0;
        normal = normalize(normal);

        float3 viewPos = positionTex.Load(input.posProj.xy, i).xyz;
        
        sumOcclusionFactor += getOcclusionFactor(input.texcoord, viewPos, normal) * sampleWeightArray[i];
    }
    
    sumOcclusionFactor *= 0.25;
    
    return 1.0 - sumOcclusionFactor;
}