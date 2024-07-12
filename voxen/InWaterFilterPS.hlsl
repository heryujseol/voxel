#include "Common.hlsli"

Texture2DMS<float4, 4> msaaRenderTex : register(t0);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
    uint sampleIndex : SV_SampleIndex;
};

float4 main(vsOutput input) : SV_TARGET
{
    float4 renderColor = msaaRenderTex.Load(input.posProj.xy, input.sampleIndex);

    return lerp(renderColor, float4(0, 0, 0.7, 1), 0.5);

}