#include "Common.hlsli"

struct vsInput
{
    float3 posModel : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    matrix instanceWorld : WORLD;
    uint type : TYPE;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint type : TYPE;
};

vsOutput main(vsInput input)
{
    vsOutput output;
    
    output.posWorld = mul(float4(input.posModel, 1.0), input.instanceWorld).xyz;
    
    output.posProj = mul(float4(output.posWorld, 1.0), view);
    output.posProj = mul(output.posProj, proj);
    
    output.normal = input.normal; // invTranspose 고려하지 않음 -> ununiform scaling X
    float3 toEye = normalize(eyePos - output.posWorld);
    if (dot(output.normal, toEye) < 0.0)
        output.normal *= -1;
        
    output.texcoord = input.texcoord;
    
    output.type = input.type;
    
    return output;
}