#include "Common.hlsli"

Texture2DMS<float4, SAMPLE_COUNT> positionTex : register(t0);

struct psInput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint type : TYPE;
};

float main(psInput input) : SV_Target0
{
    if (input.normal.y <= 0 || input.posWorld.y < 62.0 - 1e-4 || 62.0 + 1e-4 < input.posWorld.y)
        discard;
    
    float pixelDepth = input.posProj.z;
    
    float2 texcoord = float2(input.posProj.x / mirrorWidth, input.posProj.y / mirrorHeight);
    float2 appScreenCoord = float2(texcoord.x * appWidth, texcoord.y * appHeight);
    
    float4 viewPos = positionTex.Load(appScreenCoord, 0);
    if (viewPos.w == -1.0)
        viewPos.z = 1000.0f;
    
    float4 projPos = mul(float4(viewPos.xyz, 1.0), proj);
    projPos.xyz /= projPos.w;
    
    if (projPos.z <= pixelDepth) // 저장되어 있는 값이 더 작은 depth라면 무시함
        discard;
    
    return pixelDepth;
}