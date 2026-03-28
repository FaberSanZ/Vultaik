struct InstanceData
{
    float4x4 worldMatrix;
    float4 color;
};

StructuredBuffer<InstanceData> instanceBuffer : register(t0);

struct VertexInputType
{
    float4 position : POSITION;
    uint instanceID : SV_InstanceID;
};

struct PixelInputType
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

cbuffer CameraBuffer : register(b0)
{
    float4x4 viewProjMatrix;
};

PixelInputType VS(VertexInputType input)
{
    PixelInputType output;
    
    InstanceData instance = instanceBuffer[input.instanceID];
    

    float4 worldPos = mul(input.position, instance.worldMatrix);
    
    output.Pos = mul(worldPos, viewProjMatrix);
    
    output.Color = instance.color;
    
    return output;
}