cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
}

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
    sample float4 posProj : SV_POSITION;
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
    output.texcoord = input.texcoord;
    output.type = input.type;
    
    return output;
}