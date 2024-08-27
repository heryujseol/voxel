#ifndef COMMON_HLSLI
#define COMMON_HLSLI

#define PI 3.14159265
#define SAMPLE_COUNT 4

SamplerState pointWrapSS : register(s0);
SamplerState linearWrapSS : register(s1);
SamplerState linearClampSS : register(s2);
SamplerState shadowPointSS : register(s3);
SamplerComparisonState shadowCompareSS : register(s4);
SamplerState pointClampSS : register(s5);

Texture2D shadowTex : register(t11);

cbuffer CameraConstantBuffer : register(b7)
{
    Matrix view;
    Matrix proj;
    Matrix invProj;
    float3 eyePos;
    float maxRenderDistance;
    float3 eyeDir;
    float lodRenderDistance;
    bool isUnderWater;
    float3 cameraDummyData;
};

cbuffer SkyboxConstantBuffer : register(b8)
{
    float3 normalHorizonColor;
    float skyScale;
    float3 normalZenithColor;
    float skyboxDummyData1;
    float3 sunHorizonColor;
    float skyboxDummyData2;
    float3 sunZenithColor;
    float skyboxDummyData3;
};

cbuffer LightConstantBuffer : register(b9)
{
    float3 lightDir;
    float radianceWeight;
    float3 radianceColor;
    float maxRadianceWeight;
}

cbuffer AppConstantBuffer : register(b10)
{
    float appWidth;
    float appHeight;
    float mirrorWidth;
    float mirrorHeight;
}

float3 sRGB2Linear(float3 color)
{
    return pow(clamp(color, 0.0, 1.0), 2.2);
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

float getFaceAmbient(float3 normal)
{
    float faceAmbient = 1.0; // top or else
    
    if (normal.y == 0.0 && normal.z == 0.0) // left or right
    {
        faceAmbient = 0.90;
    }
    else if (normal.x == 0.0 && normal.y == 0.0) // front or back
    {
        faceAmbient = 0.83;
    }
    else if (normal.x == 0.0 && normal.z == 0.0 && normal.y < 0.0) // bottom
    {
        faceAmbient = 0.75;
    }
    
    return faceAmbient;
}

float3 getAmbientLighting(float ao, float3 albedo, float3 normal)
{
    // skycolor ambient (envMap을 가정함)
    float sunAniso = max(dot(lightDir, eyeDir), 0.0);
    float3 eyeHorizonColor = lerp(normalHorizonColor, sunHorizonColor, sunAniso);
    
    float3 ambientColor = float3(1.0, 1.0, 1.0);
    float sunAltitude = lightDir.y;
    float dayAltitude = PI / 12.0;
    float maxHorizonAltitude = -PI / 24.0;
    if (sunAltitude <= dayAltitude)
    {        
        float w = smoothstep(maxHorizonAltitude, dayAltitude, sunAltitude);
        ambientColor = lerp(eyeHorizonColor, ambientColor, w);
    }
    
    // face ambient
    float faceAmbient = getFaceAmbient(normal);
    
    return ao * albedo * ambientColor * faceAmbient;
}

float3 SchlickFresnel(float3 F0, float NdotH)
{
    return F0 + (1 - F0) * pow(2, (-5.55473 * (NdotH) - 6.98316) * NdotH);
}

float NdfGGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    return alpha2 / (3.141592 * pow(NdotH * NdotH * (alpha2 - 1) + 1, 2));
}

float SchlickGGX(float NdotI, float NdotO, float roughness)
{
    float k = (roughness + 1) * (roughness + 1) / 8;
    float gl = NdotI / (NdotI * (1 - k) + k);
    float gv = NdotO / (NdotO * (1 - k) + k);
    return gl * gv;
}

float3 getShadowFactor()
{
    float width, height, numMips;
    shadowTex.GetDimensions(0, width, height, numMips);
    
    float g_topLX[3] = { topLX.x, topLX.y, topLX.z };
    float g_viewPortW[3] = { viewPortW.x, viewPortW.y, viewPortW.z };
    float biasA[3] = { 0.002, 0.0025, 0.00275 };
    
    for (int i = 0; i < 3; ++i)
    {
        float4 shadowPos = mul(float4(posWorld, 1.0), shadowViewProj[i]);
        shadowPos.xyz /= shadowPos.w;
        
        shadowPos.x = shadowPos.x * 0.5 + 0.5;
        shadowPos.y = shadowPos.y * -0.5 + 0.5;
        
        if (shadowPos.x < 0.0 || shadowPos.x > 1.0 || shadowPos.y < 0.0 || shadowPos.y > 1.0)
        {
            continue;
        }
        
        float bias = biasA[i];
        
        float depth = shadowPos.z - bias;
        if (depth < 0.0 || depth > 1.0)
        {
            continue;
        }
        
        float dx = 5.0 / width;
        float dy = 5.0 / height;

        float percentLit = 0.0;
        const float2 offsets[9] =
        {
            float2(-dx, -dy), float2(0.0, -dy), float2(dx, -dy),
            float2(-dx, 0.0), float2(0.0, 0.0), float2(dx, 0.0),
            float2(-dx, dy), float2(0.0, dy), float2(dx, dy)
        };
        
        shadowPos.x = (shadowPos.x * (g_viewPortW[i] / width)) + (g_topLX[i] / width);
        shadowPos.y = (shadowPos.y * (g_viewPortW[i] / height));
        
        float2 texcoord;
        float denom = 9.0;
        [unroll]
        for (int j = 0; j < 9; ++j)
        {
            texcoord = shadowPos.xy + offsets[j];
            texcoord = clamp(texcoord, 0.0, 1.0);
            percentLit += shadowTex.SampleCmpLevelZero(shadowCompareSS, texcoord, depth).r;
        }
        return percentLit / denom;
    }
    return 1.0;
}

float3 getDirectLighting(float3 normal, float3 position, float3 albedo, float metallic, float roughness)
{
    float3 pixelToEye = normalize(eyePos - position);
    float3 halfway = normalize(pixelToEye + lightDir);
    
    float NdotI = max(0.0, dot(normal, lightDir));
    float NdotH = max(0.0, dot(normal, halfway));
    float NdotO = max(0.0, dot(normal, pixelToEye));
    
    const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye))); // HoV
    float D = NdfGGX(NdotH, roughness);
    float3 G = SchlickGGX(NdotI, NdotO, roughness);
    float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
    float3 diffuseBRDF = kd * albedo;
    
    float3 shadowFactor = getShadowFactor();
    
    float3 radiance = radianceColor * shadowFactor; // radiance 값 수정\
    
    return (diffuseBRDF + specularBRDF) * radiance * NdotI;
}

float3 getNormal(uint face)
{
    if (face == 0)
    {
        return float3(-1.0, 0.0, 0.0);
    }
    else if (face == 1)
    {
        return float3(1.0, 0.0, 0.0);
    }
    else if (face == 2)
    {
        return float3(0.0, -1.0, 0.0);
    }
    else if (face == 3)
    {
        return float3(0.0, 1.0, 0.0);
    }
    else if (face == 4)
    {
        return float3(0.0, 0.0, -1.0);
    }
    else
    {
        return float3(0.0, 0.0, 1.0);
    }
}

float2 getVoxelTexcoord(float3 pos, uint face)
{
    float2 texcoord = float2(0.0, 0.0);
    
    if (face == 0) // left
    {
        texcoord = float2(-pos.z + 32.0, -pos.y + 32.0);
    }
    else if (face == 1) // right
    {
        texcoord = float2(pos.z, -pos.y + 32.0);
    }
    else if (face == 2) // bottom
    {
        texcoord = float2(pos.x, pos.z);
    }
    else if (face == 3) // top
    {
        texcoord = float2(pos.x, -pos.z + 32.0);
    }
    else if (face == 4) // front
    {
        texcoord = float2(pos.x, -pos.y + 32.0);
    }
    else // back
    {
        texcoord = float2(-pos.x + 32.0, -pos.y + 32.0);
    }

    return texcoord;
}

#endif