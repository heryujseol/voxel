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

Texture2D shadowTex : register(t4);

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

cbuffer ShadowConstantBuffer : register(b11)
{
    Matrix shadowView[3];
    Matrix shadowProj[3];
    Matrix shadowInvProj[3];
    float4 topLX;
    float4 viewPortW;
}


static const float2 poissonDisk[16] =
{
    float2(-0.94201624, -0.39906216), float2(0.94558609, -0.76890725),
            float2(-0.094184101, -0.92938870), float2(0.34495938, 0.29387760),
            float2(-0.91588581, 0.45771432), float2(-0.81544232, -0.87912464),
            float2(-0.38277543, 0.27676845), float2(0.97484398, 0.75648379),
            float2(0.44323325, -0.97511554), float2(0.53742981, -0.47373420),
            float2(-0.26496911, -0.41893023), float2(0.79197514, 0.19090188),
            float2(-0.24188840, 0.99706507), float2(-0.81409955, 0.91437590),
            float2(0.19984126, 0.78641367), float2(0.14383161, -0.14100790)
};

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
    float sunAltitude = sin(lightDir.y);
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

float N2V(float ndcDepth, matrix invProj)
{
    float4 pointView = mul(float4(0, 0, ndcDepth, 1), invProj);
    return pointView.z / pointView.w;
}

float PCF_Filter(float2 uv, float zReceiverNdc, float filterRadiusUV, Texture2D shadowMap)
{
    float sum = 0.0f;
    for (int i = 0; i < 16; ++i)
    {
        float2 offset = poissonDisk[i] * filterRadiusUV;
        sum += shadowMap.SampleCmpLevelZero(
            shadowCompareSS, uv + offset, zReceiverNdc);
    }
    return sum / 16;
}

void FindBlocker(out float avgBlockerDepthView, out float numBlockers, float2 uv,
                 float zReceiverView, Texture2D shadowMap, matrix invProj, float lightRadiusWorld, float frustumWidth)
{
    float lightRadiusUV = lightRadiusWorld / frustumWidth;
    
    float searchRadius = lightRadiusUV * (zReceiverView - 0.01) / zReceiverView;

    float blockerSum = 0;
    numBlockers = 0;
    for (int i = 0; i < 16; ++i)
    {
        float shadowMapDepth =
            shadowMap.SampleLevel(shadowPointSS, float2(uv + poissonDisk[i] * searchRadius), 0).r;

        shadowMapDepth = N2V(shadowMapDepth, invProj);
        
        if (shadowMapDepth < zReceiverView)
        {
            blockerSum += shadowMapDepth;
            numBlockers++;
        }
    }
    avgBlockerDepthView = blockerSum / numBlockers;
}

float PCSS(float2 uv, float zReceiverNdc, Texture2D shadowMap, matrix invProj, float lightRadiusWorld, float frustumWidth)
{
    float lightRadiusUV = lightRadiusWorld / frustumWidth;
    
    float zReceiverView = N2V(zReceiverNdc, invProj);
    
    // STEP 1: blocker search
    float avgBlockerDepthView = 0;
    float numBlockers = 0;

    FindBlocker(avgBlockerDepthView, numBlockers, uv, zReceiverView, shadowMap, invProj, lightRadiusWorld, frustumWidth);

    if (numBlockers < 1)
    {
        // There are no occluders so early out(this saves filtering)
        return 1.0f;
    }
    else
    {
        // STEP 2: penumbra size
        float penumbraRatio = (zReceiverView - avgBlockerDepthView) / avgBlockerDepthView;
        float filterRadiusUV = penumbraRatio * lightRadiusUV * 0.01 / zReceiverView;

        // STEP 3: filtering
        return PCF_Filter(uv, zReceiverNdc, filterRadiusUV, shadowMap);
    }
}

float getShadowFactor(float3 posWorld)
{
    float shadowFactor = 1.0;
    
    float width, height, numMips;
    shadowTex.GetDimensions(0, width, height, numMips);
    
    float g_topLX[3] =
    {
        topLX.x, topLX.y, topLX.z
    };

    float g_viewPortW[3] =
    {
        viewPortW.x, viewPortW.y, viewPortW.z
    };
    
    float frustumWidth[3] =
    {
        102.4, 307.2, 614.4
    };
    
    for (int i = 0; i < 3; ++i)
    {
        float4 shadowPos = mul(mul(float4(posWorld, 1.0), shadowView[i]), shadowProj[i]);
        shadowPos.xyz /= shadowPos.w;
        
        //NDC - > 텍스쳐 좌표계
        shadowPos.x = shadowPos.x * 0.5 + 0.5;
        shadowPos.y = shadowPos.y * -0.5 + 0.5;
        
        //텍스쳐 내에서 몇번째 텍스쳐인지 구분해내야함 텍스쳐
        //좌표계 안이 아니라면 패스
        if (shadowPos.x < 0.0 || shadowPos.x > 1.0 || shadowPos.y < 0.0 || shadowPos.y > 1.0)
        {
            continue;
        }
        
        //텍스쳐 내의 상대적 좌표로 변환
        shadowPos.x = (shadowPos.x * (g_viewPortW[i] / width)) + (g_topLX[i] / width);
        shadowPos.y = (shadowPos.y * (g_viewPortW[i] / height));
        
        float dx = 5.0 / width;
        shadowFactor = PCSS(float2(shadowPos.xy), shadowPos.z - 0.01, shadowTex, shadowInvProj[i], 0.01 /*radius*/, frustumWidth[i]);
    }
    return shadowFactor;
}

float getShadowFactor2(float3 posWorld)
{
    float width, height, numMips;
    shadowTex.GetDimensions(0, width, height, numMips);
    
    float g_topLX[3] =
    {
        topLX.x, topLX.y, topLX.z
    };

    float g_viewPortW[3] =
    {
        viewPortW.x, viewPortW.y, viewPortW.z
    };
    
    for (int i = 0; i < 3; ++i)
    {
        float4 shadowPos = mul(mul(float4(posWorld, 1.0), shadowView[i]), shadowProj[i]);
        shadowPos.xyz /= shadowPos.w;
        
        //NDC - > 텍스쳐 좌표계
        shadowPos.x = shadowPos.x * 0.5 + 0.5;
        shadowPos.y = shadowPos.y * -0.5 + 0.5;
        
        //텍스쳐 내에서 몇번째 텍스쳐인지 구분해내야함 텍스쳐
        //좌표계 안이 아니라면 패스
        if (shadowPos.x < 0.0 || shadowPos.x > 1.0 || shadowPos.y < 0.0 || shadowPos.y > 1.0)
        {
            continue;
        }
        
        //Depth in NDC space
        float depth = shadowPos.z - 0.001;
        if (depth < 0.0 || depth > 1.0)
        {
            continue;
        }
        
        float dx = 5.0 / width;
        float dy = 5.0 / height;

        float percentLit = 0.0;
        const float2 offsets[9] =
        {
            //경계선일때 조건 달아주기
            float2(-dx, -dy), float2(0.0, -dy), float2(dx, -dy),
            float2(-dx, 0.0), float2(0.0, 0.0), float2(dx, 0.0),
            float2(-dx, dy), float2(0.0, dy), float2(dx, dy)
        };
        
        //텍스쳐 내의 상대적 좌표로 변환
        shadowPos.x = (shadowPos.x * (g_viewPortW[i] / width)) + (g_topLX[i] / width);
        shadowPos.y = (shadowPos.y * (g_viewPortW[i] / height));
        
        [unroll]
        for (int j = 0; j < 9; ++j)
        {
            percentLit += shadowTex.SampleCmpLevelZero(shadowCompareSS, float2(shadowPos.xy + offsets[j]), depth).r;
        }
        return percentLit / 9.0;
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
    
    // todo
    float shadowFactor = getShadowFactor2(position);
    
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