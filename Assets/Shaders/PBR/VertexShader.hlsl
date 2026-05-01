struct InstanceData
{
    float4x4 worldMatrix;
    float4 baseColor;
    float4 material;
};

StructuredBuffer<InstanceData> instanceBuffer : register(t0);

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    uint instanceID : SV_InstanceID;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLDPOSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float4 baseColor : COLOR0;
    float4 material : COLOR1;
    float3 cameraVector : TEXCOORD1;
};

cbuffer CameraBuffer : register(b0)
{
    float4x4 viewProjMatrix;
    float4 cameraPosition;
    float4 lightDirection;
};

PSInput VS(VSInput input)
{
    PSInput output;
    InstanceData instance = instanceBuffer[input.instanceID];

    float4 worldPos = mul(float4(input.position, 1.0f), instance.worldMatrix);
    float3x3 normalMatrix = transpose(inverse((float3x3)instance.worldMatrix));
    float3 worldNormal = normalize(mul(input.normal, normalMatrix));

    output.position = mul(worldPos, viewProjMatrix);
    output.worldPosition = worldPos.xyz;
    output.uv = input.uv;
    output.normal = worldNormal;
    output.baseColor = instance.baseColor;
    output.material = instance.material;
    output.cameraVector = cameraPosition.xyz - worldPos.xyz;

    return output;
}
