#include "Common.hlsli"

cbuffer ChunkConstantBuffer : register(b0)
{
    matrix world;
}

struct vsOutput
{
#ifdef USE_SHADOW
    float4 posProj : SV_POSITION;
#else
    float4 posProj : SV_POSITION;
    sample float3 posWorld : POSITION;
    sample float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint type : TYPE;
#endif
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
    
    output.posProj = mul(float4(position, 1.0), world);
    
#ifdef USE_SHADOW
    return output;
#else
    output.posWorld = output.posProj.xyz;
    
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    
    output.normal = getNormal(face);
    
    output.texcoord = getVoxelTexcoord(position, face);
    
    output.type = (type - 1) * 6 + face;

    return output;
#endif
}