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

struct psOutput
{
    float4 color : SV_Target0;
    float depth : SV_Target1;
};

psOutput main(vsOutput input)
{
    float3 normal = getNormal(input.face);
    if (normal.y <= 0 || input.posWorld.y < 62.0 - 1e-4 || 62.0 + 1e-4 < input.posWorld.y)
        discard;
    
    float3 toEye = normalize(eyePos - input.posWorld);
    float3 color = envMapTexture.Sample(linearClampSS, reflect(-toEye, normal)).rgb;
  
    psOutput output;
    output.color = float4(color, 1.0);
    output.depth = input.posProj.z;
    
    return output;
}