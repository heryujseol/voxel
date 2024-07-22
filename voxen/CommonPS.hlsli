#ifndef COMMON_HLSLI
    #define COMMON_HLSLI

SamplerState pointWrapSS : register(s0);
SamplerState linearWrapSS : register(s1);
SamplerState linearClampSS : register(s2);
SamplerState shadowPointSS : register(s3);
SamplerComparisonState shadowCompareSS : register(s4);
SamplerState pointClampSS : register(s5);

cbuffer CameraConstantBuffer : register(b0)
{
    Matrix view;
    Matrix proj;
    float3 eyePos;
    float maxRenderDistance;
    float3 eyeDir;
    float lodRenderDistance;
    Matrix invProj;
    bool isUnderWater;
    float3 cameraDummyData;
};

cbuffer SkyboxConstantBuffer : register(b1)
{
    float3 sunDir;
    float skyScale;
    float3 normalHorizonColor;
    uint dateTime;
    float3 normalZenithColor;
    float sunStrength;
    float3 sunHorizonColor;
    float moonStrength;
    float3 sunZenithColor;
    float skyboxDummyData;
};

#define PI 3.14159265
#define SAMPLE_COUNT 4

float henyeyGreensteinPhase(float3 L, float3 V, float aniso)
{
	// L: toLight
	// V: eyeDir
	// https://www.shadertoy.com/view/7s3SRH
    float cosT = dot(L, V);
    float g = aniso;
    return (1.0 - g * g) / (4.0 * PI * pow(abs(1.0 + g * g - 2.0 * g * cosT), 3.0 / 2.0));
}

float3 texcoordToViewPos(float2 texcoord, float projDepth)
{
    float4 posProj;
    
    posProj.xy = texcoord * 2.0 - 1.0;
    posProj.y *= -1;
    posProj.z = projDepth;
    posProj.w = 1.0;

    float4 posView = mul(posProj, invProj);
    posView.xyz /= posView.w;
    
    return posView.xyz;
}

uint4 coverageAnalysis(uint4 coverage)
{
    uint4 sampleWeight = uint4(1, 1, 1, 1);
    
    if (coverage.x == coverage.y)
    {
        ++sampleWeight.x;
        coverage.y = 0;
    }
    if (coverage.x == coverage.z)
    {
        ++sampleWeight.x;
        coverage.z = 0;
    }
    if (coverage.x == coverage.w)
    {
        ++sampleWeight.x;
        coverage.w = 0;
    }
    if (coverage.y == coverage.z)
    {
        ++sampleWeight.y;
        coverage.z = 0;
    }
    if (coverage.y == coverage.w)
    {
        ++sampleWeight.y;
        coverage.w = 0;
    }
    if (coverage.z == coverage.w)
    {
        ++sampleWeight.z;
        coverage.w = 0;
    }

    // 재조정 : coverage가 0인 것은 마스킹이 안된 샘플이거나, 같은 마스킹이 있는 경우
    sampleWeight.x = (coverage.x > 0) ? sampleWeight.x : 0;
    sampleWeight.y = (coverage.y > 0) ? sampleWeight.y : 0;
    sampleWeight.z = (coverage.z > 0) ? sampleWeight.z : 0;
    sampleWeight.w = (coverage.w > 0) ? sampleWeight.w : 0;
    
    return sampleWeight;
}

#endif