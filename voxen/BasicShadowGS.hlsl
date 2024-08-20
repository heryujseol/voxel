cbuffer LightConstantBuffer : register(b0)
{
    Matrix viewProj[3];
    float4 topLX;
    float4 viewPortW;
}

struct vsOutput
{
    float4 posWorld : SV_POSITION;
};

struct gsOutput
{
    float4 pos : SV_POSITION;
    uint VPIndex : SV_ViewportArrayIndex;
};

[maxvertexcount(9)]
void main(triangle vsOutput input[3], inout TriangleStream<gsOutput> output)
{
    gsOutput element;
    
    for (int face = 0; face < 3; ++face)
    {
        element.VPIndex = face;
        
        for (int i = 0; i < 3; ++i)
        {
            float4 position = float4(input[i].posWorld.xyz, 1.0);
            element.pos = mul(position, viewProj[face]);
            output.Append(element);
        }
        output.RestartStrip();
    }
}