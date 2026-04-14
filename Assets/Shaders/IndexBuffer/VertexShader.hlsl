struct VSInput
{
    float4 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer CameraBuffer : register(b0)
{
    float4x4 viewProjMatrix;
};

PSInput VS(VSInput input)
{
    PSInput output;
    output.position = mul(input.position, viewProjMatrix);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}