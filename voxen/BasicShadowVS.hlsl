cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
}

cbuffer ChunkConstantBuffer : register(b1)
{
    matrix world;
}

//cbuffer LightConstantBuffer : register(b2)
//{
//    Matrix Lview[4];
//    Matrix Lproj[4];
//}

struct vsOutput
{
    float4 posWorld : SV_POSITION;
};

vsOutput main(uint data : DATA, uint vertexID : SV_VertexID)
{
    vsOutput output;
    
    int x = (data >> 23) & 63;
    int y = (data >> 17) & 63;
    int z = (data >> 11) & 63;
    uint face = (data >> 8) & 7;
    uint type = data & 255;
    
    float3 position = float3(float(x), float(y), float(z));
    
    output.posWorld = mul(float4(position, 1.0), world);
    //output.posWorld = mul(float4(position, 1.0), Lview[3]);
    //output.posWorld = mul(output.posWorld, Lproj[3]);
    
    return output;
}