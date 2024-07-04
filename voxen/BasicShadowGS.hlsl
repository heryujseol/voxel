cbuffer LightConstantBuffer : register(b0)
{
    Matrix view[4];
    Matrix proj[4];
    float4 lightPos[4];
    float3 lightDir;
    float dummy2;
    Matrix invProj[4];
}

struct vsOutput
{
    float4 posWorld : SV_POSITION;
};

struct gsOutput
{
    float4 pos : SV_POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
    //uint VPIndex : SV_ViewportArrayIndex;
};

[maxvertexcount(12)]
void main(triangle vsOutput input[3], inout TriangleStream<gsOutput> output)
{
    gsOutput element;
    
    for (int face = 0; face < 4; ++face)
    {
        element.RTIndex = face;
        //element.VPIndex = face;
        
        for (int i = 0; i < 3; ++i)
        {
            float4 position = float4(input[i].posWorld.x, input[i].posWorld.y, input[i].posWorld.z, 1.0);
            element.pos = mul(position, view[face]);
            element.pos = mul(element.pos, proj[face]);
            output.Append(element);
        }
        output.RestartStrip();
    }
}