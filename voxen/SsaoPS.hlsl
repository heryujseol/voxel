#include "Common.hlsli"

Texture2DMS<float4, 4> normalMapTex : register(t0);
Texture2DMS<float, 4> depthMapTex : register(t1);

cbuffer SsaoConstantBuffer : register(b2)
{
    float4 sampleKernel[64];
}

cbuffer SsaoNoiseConstantBuffer : register(b3)
{
    float4 rotationNoise[16];
}

cbuffer TMPConstantBuffer : register(b4)
{
    float radius;
    float bias;
    float edgeBias;
    float dummy2;
}

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(vsOutput input) : SV_TARGET
{   
    float3 normal = normalMapTex.Load(input.posProj.xy, 0).xyz;
    if (length(normal) == 0)
        return float4(1, 1, 1, 1);
    normal = normalize(normal);
    
    float depth = depthMapTex.Load(input.posProj.xy, 0).r;
    
    float3 viewPos = convertViewPos(input.texcoord, depth);

    // 200배 확대한 것을 frac연산으로 다시 하나씩 200개로 쪼갬 -> 4로 곱하여 인덱스로 사용
    // PointWrap 샘플러라고 생각
    float fx = frac(input.texcoord.x * 200.0) * 4.0; // [0,4]
    float fy = frac(input.texcoord.y * 200.0) * 4.0; // [0,4]
    uint ix = uint(floor(fx)) % 4;
    uint iy = uint(floor(fy)) % 4;
    float3 randomVec = rotationNoise[ix + 4 * iy].xyz;
 
    float3 T = normalize(randomVec - normal * dot(normal, randomVec)); // R - proj.n(R)
    float3 B = cross(normal, T);
    float3x3 TBN = float3x3(T, B, normal);
    
    float occlusionFactor = 0.0;
    float radius = 1.0;
    float bias = 0.025;
    uint sampledCount = 0;
    [unroll]
    for (uint i = 0; i < 64; ++i)
    {
        float3 sampleOffset = mul(sampleKernel[i].xyz, TBN);
        float3 sampleViewPos = viewPos + sampleOffset * radius;
        
        float4 sampleProjPos = float4(sampleViewPos, 1.0);
        sampleProjPos = mul(sampleProjPos, proj);
        sampleProjPos.xyz /= sampleProjPos.w; // [-1, 1]
        
        float2 sampleTexcoord = sampleProjPos.xy;
        sampleTexcoord.x = sampleTexcoord.x * 0.5 + 0.5; // [-1, 1] -> [0, 1]
        sampleTexcoord.y = -(sampleTexcoord.y * 0.5) + 0.5; // [-1, 1] -> [1, 0]
        if (0.0 <= sampleTexcoord.x && sampleTexcoord.x <= 1.0 && 0.0 <= sampleTexcoord.y && sampleTexcoord.y <= 1.0)
        {
            float2 screenCoord = float2(sampleTexcoord.x * 1920.0 - 0.5, sampleTexcoord.y * 1080.0 - 0.5);
            float storedDepth = depthMapTex.Load(screenCoord, 0).r;
        
            float3 storedViewPos = convertViewPos(sampleTexcoord, storedDepth);
            float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - storedViewPos.z));
            occlusionFactor += (storedViewPos.z < sampleViewPos.z - bias ? 1.0 : 0.0) * rangeCheck;
            sampledCount++;
        }
    }
    
    if (sampledCount == 0)
    {
        return float4(1, 1, 1, 1);
    }
    float ret = 1.0 - (occlusionFactor / sampledCount);
    return float4(ret, ret, ret, 1.0);
}