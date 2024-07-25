#include "Common.hlsli"

Texture2D sunTexture : register(t0);
Texture2D moonTexture : register(t1);

struct psInput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
};

float4 main(psInput input) : SV_TARGET
{
    float3 color = float3(0.0, 0.0, 0.0);
    float3 posDir = normalize(input.posWorld);
#ifdef USE_MIRROR
    posDir.y *= -1;
#endif
    
    float sunAltitude = sin(lightDir.y);
    float showSectionAltitude = -PI * 0.5 * (1.7 / 6.0);
    
    // background sky
    float sunDirWeight = sunAltitude > showSectionAltitude ? henyeyGreensteinPhase(lightDir, eyeDir, 0.625) : 0.0;
    float posAltitude = sin(posDir.y);
   
    float3 horizonColor = lerp(normalHorizonColor, sunHorizonColor, sunDirWeight);
    float3 zenithColor = lerp(normalZenithColor, sunZenithColor, sunDirWeight);
    
    // zenith와 horizon 구별 고도 고려
    // 최대한 구별된 색 선택하도록 결정
    float3 mixColor = (horizonColor + zenithColor) * 0.5;
    float horizonAltitude = PI / 24.0;
    
    if (posAltitude <= horizonAltitude)
        color += lerp(horizonColor, mixColor, pow((posAltitude + 1.0) / (1.0 + horizonAltitude), 15.0));
    else
        color += lerp(mixColor, zenithColor, pow((posAltitude - horizonAltitude) / (1.0 - horizonAltitude), 0.5));
    
    return float4(color, 1.0);
}


