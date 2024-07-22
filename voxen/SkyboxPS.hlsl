#include "CommonPS.hlsli"

Texture2D sunTexture : register(t0);
Texture2D moonTexture : register(t1);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;

#ifdef USE_RENDER_TARGET_ARRAY_INDEX
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
#endif
};

bool getPlanetTexcoord(float3 posDir, float3 planetDir, float size, out float2 texcoord)
{   
    texcoord = float2(0.0, 0.0);
    bool ret = false;
    
    if (length(posDir.xy) != 0.0)
    {
        float3 posDirHorizontal = normalize(float3(planetDir.xy, posDir.z));
        float3 posDirVertical = normalize(float3(posDir.xy, 0.0));
    
        float dotSH = max(dot(posDirHorizontal, planetDir), 0.0);
        float dotSV = max(dot(posDirVertical, planetDir), 0.0);
    
        float width = max(tan(acos(dotSH)), 0.0) * skyScale;
        float height = max(tan(acos(dotSV)), 0.0) * skyScale;
    
        if (width <= size && height <= size) // 0 ~ size
        {
            // horizontal tex_x 
            float sign = posDirHorizontal.z > 0 ? -1 : 1;
            float tex_x = (sign * width + size) * 0.5 / size;
        
            // vertical tex_y
            float3 crossSV = cross(planetDir, posDirVertical);
            sign = crossSV.z > 0 ? -1 : 1;
            float tex_y = (sign * height + size) * 0.5 / size;
        
            texcoord = float2(tex_x, tex_y);
            ret = true;
        }
    }
    
    return ret;
}

float3 getSkyColor(float3 posDir, float sunDirWeight)
{
    // ([0, pi] - pi/2) * -2/pi -> [1, -1]
    float posAltitude = sin(posDir.y);
   
    float3 horizonColor = lerp(normalHorizonColor, sunHorizonColor, sunDirWeight);
    float3 zenithColor = lerp(normalZenithColor, sunZenithColor, sunDirWeight);
    
    // zenith�� horizon ���� �� ���
    // �ִ��� ������ �� �����ϵ��� ����
    float3 mixColor = (horizonColor + zenithColor) * 0.5;
    float horizonAltitude = PI / 24.0;
    if (posAltitude <= horizonAltitude)
    {
        return lerp(horizonColor, mixColor, pow((posAltitude + 1.0) / (1.0 + horizonAltitude), 15.0));
    }
    else
    {
        return lerp(mixColor, zenithColor, pow((posAltitude - horizonAltitude) / (1.0 - horizonAltitude), 0.5));
    }
}

float4 main(vsOutput input) : SV_TARGET
{
    float3 color = float3(0.0, 0.0, 0.0);
    float3 posDir = normalize(input.posWorld);
    
    float sunAltitude = sin(sunDir.y);
    float showSectionAltitude = -PI * 0.5 * (1.7 / 6.0);
    
    // sun
    float maxSunSize = 220.0f;
    float minSunSize = 50.0f;
    float sunSize = lerp(minSunSize, maxSunSize, pow(max(dot(sunDir, eyeDir), 0.0), 3.0));
    float2 sunTexcoord;
    if (sunAltitude > showSectionAltitude && getPlanetTexcoord(posDir, sunDir, sunSize, sunTexcoord))
    {
#ifdef USE_RENDER_TARGET_ARRAY_INDEX
        color += sunTexture.SampleLevel(linearClampSS, sunTexcoord, 0.0).rgb * sunStrength;
#else
        color += sunTexture.SampleLevel(pointWrapSS, sunTexcoord, 0.0).rgb * sunStrength;
    #endif
        
    }
    
    // moon
    float moonSize = lerp(minSunSize, maxSunSize * 0.5f, pow(max(dot(-sunDir, eyeDir), 0.0), 3.0));
    float2 moonTexcoord;
    if (-sunAltitude > showSectionAltitude && getPlanetTexcoord(posDir, -sunDir, moonSize, moonTexcoord))
    {
        uint col = 4;
        uint row = 2;
        
        uint day = 0;
        uint index = day % 8; // 0 ~ 7

        uint2 indexUV = uint2(index % col, index / col); // [0,0]~[3,1]
        
        moonTexcoord += indexUV; // moonTexcoord : [0,0]~[4,2] 
        moonTexcoord = float2(moonTexcoord.x / col, moonTexcoord.y / row); // [4,2]->[1,1]
        
#ifdef USE_RENDER_TARGET_ARRAY_INDEX
        color += moonTexture.SampleLevel(linearClampSS, moonTexcoord, 0.0).rgb * moonStrength;
#else
        color += moonTexture.SampleLevel(pointWrapSS, moonTexcoord, 0.0).rgb * moonStrength;
#endif
    }
   
    // background sky
    float sunDirWeight = sunAltitude > showSectionAltitude ? henyeyGreensteinPhase(sunDir, eyeDir, 0.625) : 0.0;
    //float sunDirWeight = sunAltitude > showSectionAltitude ? max(dot(sunDir, eyeDir), 0.0) : 0.0;
    color += getSkyColor(posDir, sunDirWeight);

    return float4(color, 1.0);
}


