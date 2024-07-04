cbuffer ChunkConstantBuffer : register(b1)
{
    matrix world;
}

cbuffer LightConstantData : register(b2)
{
    Matrix view;
    Matrix proj;
}

struct vsInput
{
    float3 position : POSITION;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
};

vsOutput main(vsInput input)
{
    vsOutput output;
    
    float3 position = input.position;
    
    output.posProj = mul(float4(position, 1.0), world);

    output.posProj = mul(float4(output.posProj.xyz, 0.0), view);
    output.posProj = mul(float4(output.posProj.xyz, 1.0), proj);
    
    return output;
}