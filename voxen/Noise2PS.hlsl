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
    
    if (r <= 63.0)
        return float4(0, 0, r / 63.0, 1);
    
    return float4(r / 192.0, r / 192.0, r / 192.0, 1.0);
}