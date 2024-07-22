#include "CommonVS.hlsli"

cbuffer ChunkConstantBuffer : register(b1)
{
    matrix world;
}

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint type : TYPE;
};

vsOutput main(uint data : DATA)
{
    vsOutput output;
    
    int x = (data >> 23) & 63;
    int y = (data >> 17) & 63;
    int z = (data >> 11) & 63;
    uint face = (data >> 8) & 7;
    uint type = data & 255;
    
    float3 position = float3(float(x), float(y), float(z));
    
    output.posWorld = mul(float4(position, 1.0), world).xyz;
    
    output.posProj = float4(output.posWorld, 1.0);
    output.posProj = mul(output.posProj, view); 
    output.posProj = mul(output.posProj, proj);
    
    output.normal = getNormal(face);
    
    output.texcoord = getVoxelTexcoord(position, face);
    
    output.type = (type - 1) * 6;
    
    return output;
}