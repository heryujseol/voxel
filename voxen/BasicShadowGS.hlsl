cbuffer ShadowConstantBuffer : register(b0)
{
    Matrix viewProj[4];
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

[maxvertexcount(12)]
void main(triangle vsOutput input[3], inout TriangleStream<gsOutput> output)
{
    gsOutput element;
    
    for (int cascade = 0; cascade < 4; ++cascade)
    {
        element.VPIndex = cascade;
        
        for (int i = 0; i < 3; ++i)
        {
            float4 position = float4(input[i].posWorld.xyz, 1.0);
            element.pos = mul(position, viewProj[cascade]);
            output.Append(element);
        }
        output.RestartStrip();
    }
}