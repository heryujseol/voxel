#include "CommonPS.hlsli"

Texture2DMS<float4, SAMPLE_COUNT> normalEdgeTex : register(t0);
Texture2DMS<float4, SAMPLE_COUNT> positionTex : register(t1);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(vsOutput input) : SV_Target
{
    float sumEdge = 0;
    float sumW = 0;
    
    [unroll]
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        sumEdge += normalEdgeTex.Load(input.posProj.xy, i).w;
        sumW += positionTex.Load(input.posProj.xy, i).w;
    }
    
    if (!sumEdge)
        discard;
    if (sumW == -1.0 * SAMPLE_COUNT)
        discard;
    
    return float4(0, 0, 0, 0);
}