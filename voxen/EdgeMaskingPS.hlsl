#include "CommonPS.hlsli"

Texture2D albedoEdgeTex : register(t0);

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(vsOutput input) : SV_Target
{
    bool isEdge = albedoEdgeTex.Sample(pointClampSS, input.texcoord).a;
    if (!isEdge)
        discard;
    
    return float4(0, 0, 0, 0);
}