#include "Common.hlsli"

Texture2DArray atlasTextureArray : register(t0);
#ifdef USE_DEPTH_CLIP
    Texture2D depthTex : register(t2);
#endif

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint type : TYPE;
};

struct psOutput
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float depth : SV_TARGET2;
};

psOutput main(vsOutput input)
{
    float alpha = atlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, (float) input.type), 0.0).a;
    if (alpha != 1.0)
        discard;
    
#ifdef USE_DEPTH_CLIP
    float width, height, lod;
    depthTex.GetDimensions(0, width, height, lod);
    
    float2 screenTexcoord = float2(input.posProj.x / width, input.posProj.y / height);
    float planeDepth = depthTex.Sample(linearClampSS, screenTexcoord).r;
    float pixelDepth = input.posProj.z;

    if (pixelDepth < planeDepth)
    {
        discard;
    }
    
    float4 color = atlasTextureArray.Sample(linearWrapSS, float3(input.texcoord, (float) input.type));
#else
    float4 color = atlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, (float) input.type), 0.0);
#endif
    
    psOutput output;
    
    output.albedo = color;
    
    // 스프라이트의 노멀벡터인 경우 보이는 쪽으로 설정
    float3 toEye = eyePos - input.posWorld;
    input.normal *= (dot(toEye, input.normal) < 0) ? -1 : 1; 
    
    float4 normalView = mul(float4(input.normal, 0.0), view); // must be [Normal * ITWorld * ITView]
    output.normal = normalView;
    
    output.depth = input.posProj.z;
    
    return output;
}