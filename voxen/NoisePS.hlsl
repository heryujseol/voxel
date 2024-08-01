#include "Common.hlsli"

Texture2D samplingTexture : register(t0);

struct psInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(psInput input) : SV_TARGET
{
    float r = samplingTexture.SampleLevel(linearWrapSS, input.texcoord, 0.0).r;
    
    return float4(r, r, r, 1.0);
}