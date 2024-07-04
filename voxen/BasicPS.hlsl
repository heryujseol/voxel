Texture2DArray atlasTextureArray : register(t0);
Texture2D grassColorMap : register(t1);
Texture2DArray shadowMap : register(t2);

SamplerState pointWrapSS : register(s0);
SamplerState linearWrapSS : register(s1);
SamplerState shadowPointSS : register(s2);
SamplerComparisonState shadowCompareSS : register(s3);

cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
    float3 eyePos;
    float dummy;
}

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
    float dummy3;
};

cbuffer LightConstantBuffer : register(b2)
{
    Matrix lightView[4];
    Matrix lightProj[4];
    float3 lightPos[4];
    float dummy1[4];
    float3 lightDir;
    float dummy2;
    Matrix invProj[4];
}

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION1;
    sample float3 posModel : POSITION2;
    uint face : FACE;
    uint type : TYPE;
};

#define NEAR_PLANE 0.1
#define LIGHT_FRUSTUM_WIDTH 0.34641

//static const float2 poissonDisk[16] =
//{
//    float2(-0.94201624, -0.39906216), float2(0.94558609, -0.76890725),
//            float2(-0.094184101, -0.92938870), float2(0.34495938, 0.29387760),
//            float2(-0.91588581, 0.45771432), float2(-0.81544232, -0.87912464),
//            float2(-0.38277543, 0.27676845), float2(0.97484398, 0.75648379),
//            float2(0.44323325, -0.97511554), float2(0.53742981, -0.47373420),
//            float2(-0.26496911, -0.41893023), float2(0.79197514, 0.19090188),
//            float2(-0.24188840, 0.99706507), float2(-0.81409955, 0.91437590),
//            float2(0.19984126, 0.78641367), float2(0.14383161, -0.14100790)
//};


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

//float N2V(float ndcDepth, matrix invProj)
//{
//    float4 pointView = mul(float4(0, 0, ndcDepth, 1), invProj);
//    return pointView.z / pointView.w;
//}

//float PCF_Filter(float2 uv, float zReceiverNdc, float filterRadiusUV, Texture2D shadowMap)
//{
//    float sum = 0.0f;
//    for (int i = 0; i < 16; ++i)
//    {
//        float2 offset = poissonDisk[i] * filterRadiusUV;
//        sum += shadowMap.SampleCmpLevelZero(
//            shadowCompareSS, uv + offset, zReceiverNdc);
//    }
//    return sum / 16;
//}

//void FindBlocker(out float avgBlockerDepthView, out float numBlockers, float2 uv,
//                 float zReceiverView, Texture2D shadowMap, matrix invProj, float lightRadiusWorld)
//{
//    float lightRadiusUV = lightRadiusWorld / LIGHT_FRUSTUM_WIDTH;
    
//    float searchRadius = lightRadiusUV * (zReceiverView - NEAR_PLANE) / zReceiverView;

//    float blockerSum = 0;
//    numBlockers = 0;
//    for (int i = 0; i < 16; ++i)
//    {
//        float shadowMapDepth =
//            shadowMap.SampleLevel(shadowPointSS, float2(uv + poissonDisk[i] * searchRadius), 0).r;

//        shadowMapDepth = N2V(shadowMapDepth, invProj);
        
//        if (shadowMapDepth < zReceiverView)
//        {
//            blockerSum += shadowMapDepth;
//            numBlockers++;
//        }
//    }
//    avgBlockerDepthView = blockerSum / numBlockers;
//}

//float PCSS(float2 uv, float zReceiverNdc, Texture2D shadowMap, matrix invProj, float lightRadiusWorld)
//{
//    float lightRadiusUV = lightRadiusWorld / LIGHT_FRUSTUM_WIDTH;
    
//    float zReceiverView = N2V(zReceiverNdc, invProj);
    
//    float avgBlockerDepthView = 0;
//    float numBlockers = 0;
    
//    FindBlocker(avgBlockerDepthView, numBlockers, uv, zReceiverView, shadowMap, invProj, lightRadiusWorld);
    
//    if (numBlockers < 1)
//    {
//        return 1.0;
//    }
//    else 
//    {
//        float penumbraRatio = (zReceiverView - avgBlockerDepthView) / avgBlockerDepthView;
//        float filterRadiusUV = penumbraRatio * lightRadiusUV * NEAR_PLANE / zReceiverView;

//        return PCF_Filter(uv, zReceiverNdc, filterRadiusUV, shadowMap);
//    }
//}

float getShadowFactor(float3 posWorld)
{
    //float shadowFactor = 1.0;
    
    //float4 lightScreen = mul(float4(posWorld, 1.0), viewProj);
    //lightScreen.xyz /= lightScreen.w;
    
    //float2 lightTexcoord = float2(lightScreen.x, -lightScreen.y);
    //lightTexcoord += 1.0;
    //lightTexcoord *= 0.5;
    
    ////uint width, height, numMips;
    ////shadowMap.GetDimensions(0, width, height, numMips);
    
    ////shadowFactor = PCSS(lightTexcoord, lightScreen.z - 0.01, shadowMap, invProj, radius);
    
    //return (shadowMap.Sample(shadowPointSS, lightTexcoord).r < 1.0);
    
    ////float depth = shadowMap.Sample(shadowPointSS, lightTexcoord).r;

    ////if (depth + 0.01 < lightScreen.z)
    ////    shadowFactor = 0.0;

    ////return shadowFactor;
    
    float width, height, numElements, numMips;
    shadowMap.GetDimensions(0, width, height, numElements, numMips);
    
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        float4 shadowPos = mul(mul(float4(posWorld, 1.0), lightView[i]), lightProj[i]);
        shadowPos.xyz /= shadowPos.w;
        
        // NDC -> 텍스쳐 좌표계
        shadowPos.x = shadowPos.x * 0.5f + 0.5f;
        shadowPos.y = shadowPos.y * -0.5f + 0.5f;

        // 텍스쳐 좌표계 안이 아니라면 패스
        if (shadowPos.x < 0 || shadowPos.x > 1 || shadowPos.y < 0 || shadowPos.y > 1)
        {
            continue;
        }


        // Depth in NDC space.
        float depth = shadowPos.z - 0.0001f;
        if (depth < 0.0f || depth > 1.0f)
        {
            continue;
        }


        // Texel size.
        float dx = 1.0f / (float) width;

        float percentLit = 0.0f;
        const float2 offsets[9] =
        {
            float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
            float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
            float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
        };

        [unroll]
        for (int j = 0; j < 9; ++j)
        {
            percentLit += shadowMap.SampleCmpLevelZero(shadowCompareSS, float3(shadowPos.xy + offsets[j], i), depth).r;
        }
        return percentLit / 9.0f;
    }
    return 1.0f;
}

float4 main(vsOutput input) : SV_TARGET
{

    //float temperature = 0.5;
    //float downfall = 1.0;
    //float4 biome = grassColorMap.SampleLevel(pointClampSS, float2(1 - temperature, 1 - temperature / downfall), 0.0);
    
    float2 texcoord = getVoxelTexcoord(input.posModel, input.face);
    uint index = (input.type - 1) * 6 + input.face;
    
    float3 color = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, index)).rgb;
    //color = getFaceColor(input.face, color);
    
    float3 normal = getNormal(input.face);
    float ndotl = max(dot(sunDir, normal), 0.3);
    
    float strength = clamp(sunStrength, 0.25, 1.0);
    
    float3 EyeDir = float3(eyePos - input.posWorld);
    float3 H = normalize(-EyeDir + -sunDir);
    float hdotn = dot(H, normal);
    
    float3 diff = strength * ndotl;
    float3 spec = pow(max(hdotn, 0.0), 4.0);
    
    float shadowFactor = getShadowFactor(input.posWorld);
    //if (shadowFactor == 1)
    //    color = float3(1.0, 0.0, 0.0);
    //else if (shadowFactor == 2)
    //    color = float3(0.0, 1.0, 0.0);
    //else if (shadowFactor == 3)
    //    color = float3(0.0, 0.0, 1.0);
    //else if (shadowFactor == 4)
    //    color = float3(1.0, 1.0, 0.0);
    
    return float4(color, 0.0);
    
        if (12700 <= dateTime && dateTime <= 13700)
        {
            float w = (dateTime - 12700) / 1000.0;
            color = lerp(color * (diff + spec), color * (strength * 0.3), w);
        }
        else if (22300 <= dateTime && dateTime <= 23300)
        {
            float w = (dateTime - 22300) / 1000.0;
            color = lerp(color * (strength * 0.3), color * (diff + spec), w);
        }
        else if (13700 < dateTime && dateTime < 22300)
        {
            color = color * strength * 0.3;
        }
        else
        {
            color = color * (diff + spec) * shadowFactor;
        }
    
    return float4(color, 0.0);
}