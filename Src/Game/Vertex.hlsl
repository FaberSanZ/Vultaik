cbuffer CameraBuffer : register(b0)
{
    float4x4 View;
    float4x4 Projection;
};

cbuffer ObjectBuffer : register(b1)
{
    float4x4 World;
};

struct VertexInput
{
    float2 Position : POSITION;
    float4 Color : COLOR;
};

struct PixelInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

PixelInput VS(VertexInput input)
{
    float4 worldPosition = mul(float4(input.Position, 0.0f, 1.0f), World);

    PixelInput output;
    output.Position = mul(worldPosition, View);
    output.Position = mul(output.Position, Projection);
    output.Color = input.Color;

    return output;
}