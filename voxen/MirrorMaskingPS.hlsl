#include "Common.hlsli"

TextureCube envMapTexture : register(t0);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION1;
    sample float3 posModel : POSITION2;
    uint face : FACE;
    uint type : TYPE;
};

float4 main(vsOutput input) : SV_TARGET
{
    float3 normal = getNormal(input.face);
    float3 toEye = normalize(eyePos - input.posWorld);
    
    if (normal.y <= 0 || input.posWorld.y < 62.0 - 1e-4 || 62.0 + 1e-4 < input.posWorld.y)
        discard;
    
    float3 color = envMapTexture.Sample(linearClampSS, reflect(-toEye, normal)).rgb;
    
    return float4(color, 1.0);
}