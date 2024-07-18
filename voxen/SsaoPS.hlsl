#include "Common.hlsli"

Texture2D depthMapTex : register(t0);
Texture2D normalMapTex : register(t1);

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

float main(vsOutput input) : SV_TARGET
{   
    float3 normal = normalMapTex.Sample(pointClampSS, input.texcoord).xyz;
    if (length(normal) == 0)
        return 1.0;
    normal = normalize(normal);
    
    float depth = depthMapTex.Sample(pointClampSS, input.texcoord).r;
    
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
    
    [unroll]
    for (uint i = 0; i < 64; ++i)
    {
        float3 sampleOffset = mul(sampleKernel[i].xyz, TBN);
        float3 samplePos = viewPos + sampleOffset * radius;
        
        float4 sampleProjPos = float4(samplePos, 1.0);
        sampleProjPos = mul(sampleProjPos, proj);
        sampleProjPos.xyz /= sampleProjPos.w; // [-1, 1]
        
        float2 sampleTexcoord = sampleProjPos.xy;
        sampleTexcoord.x = sampleTexcoord.x * 0.5 + 0.5; // [-1, 1] -> [0, 1]
        sampleTexcoord.y = -(sampleTexcoord.y * 0.5) + 0.5; // [-1, 1] -> [1, 0]
        float storedDepth = depthMapTex.Sample(linearClampSS, sampleTexcoord).r;
        
        float3 storedViewPos = convertViewPos(sampleTexcoord, storedDepth);
        
        //float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - storedViewPos.z));
        if (length(viewPos - storedViewPos) <= radius)
        {
            occlusionFactor += (storedViewPos.z < samplePos.z - bias ? 1.0 : 0.0);// * rangeCheck;
        }
    }
    
    return 1.0 - (occlusionFactor / 64);
}