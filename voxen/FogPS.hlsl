#include "Common.hlsli"

Texture2D renderTex : register(t0); // Rendering results
Texture2D depthOnlyTex : register(t1); // DepthOnly

struct SamplingPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 TexcoordToView(float2 texcoord)
{
    float4 posProj;

    // [0, 1]x[0, 1] -> [-1, 1]x[-1, 1]
    posProj.xy = texcoord * 2.0 - 1.0;
    posProj.y *= -1;
    posProj.z = depthOnlyTex.Sample(linearClampSS, texcoord).r;
    posProj.w = 1.0;

    // ProjectSpace -> ViewSpace
    float4 posView = mul(posProj, invProj);
    posView.xyz /= posView.w; // homogeneous coordinates
    
    return posView;
}

float4 main(SamplingPixelShaderInput input) : SV_TARGET
{
    //Beer-Lambert law
    float3 fogColor = normalHorizonColor;
    float fogMin = lodRenderDistance;
    float fogMax = maxRenderDistance;
    float fogStrength = 3.0;
        
    float4 posView = TexcoordToView(input.texcoord);
    float dist = length(posView.xyz);
        
    float distFog = saturate((dist - fogMin) / (fogMax - fogMin));
    float fogFactor = exp(-distFog * fogStrength);
        
    float3 color = renderTex.Sample(linearClampSS, input.texcoord).rgb;
    
    if ((0 <= dateTime && dateTime <= 1000) || (11000 <= dateTime && dateTime <= 13700) || (22300 <= dateTime && dateTime <= 23999))
    {
        float sunDirWeight = henyeyGreensteinPhase(sunDir, eyeDir, 0.625);
        fogColor = lerp(normalHorizonColor, sunHorizonColor, sunDirWeight);
    }
        
    color = lerp(fogColor, color, fogFactor);
    
    return float4(color, 1.0);
}