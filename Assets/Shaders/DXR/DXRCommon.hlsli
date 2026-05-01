struct InstanceData
{
    float4x4 worldMatrix;
    float4 baseColor;
    float4 material;
};

struct DxrInstanceData
{
    InstanceData instance;
    uint shapeType;
    float3 padding;
};

struct RaytracingConstants
{
    float4x4 invViewProj;
    float4 cameraPosition;
    float4 lightDirection;
    float4 viewportSize;
};

StructuredBuffer<DxrInstanceData> g_Instances : register(t0);
Texture2D g_Textures[] : register(t100);
RaytracingAccelerationStructure g_Scene : register(t2);
RWTexture2D<float4> g_Output : register(u0);

cbuffer CameraBuffer : register(b0)
{
    RaytracingConstants g_Camera;
};

SamplerState textureSampler : register(s0);

static const uint SHAPE_TRIANGLE = 0;
static const uint SHAPE_CUAD = 1;
static const uint SHAPE_PENTAGON = 2;
static const uint SHAPE_HEXAGON = 3;
static const uint SHAPE_CIRCLE = 4;
static const uint SHAPE_CUBE = 5;
static const uint SHAPE_SPHERE = 6;
static const uint SHAPE_PLANE = 7;

float3 GetApproxNormal(uint shapeType, float3 localPos)
{
    if (shapeType == SHAPE_PLANE || shapeType == SHAPE_CUAD || shapeType == SHAPE_TRIANGLE || shapeType == SHAPE_PENTAGON || shapeType == SHAPE_HEXAGON || shapeType == SHAPE_CIRCLE)
        return float3(0.0f, 1.0f, 0.0f);

    if (shapeType == SHAPE_CUBE)
    {
        float3 absPos = abs(localPos);
        if (absPos.x >= absPos.y && absPos.x >= absPos.z)
            return float3(sign(localPos.x), 0.0f, 0.0f);
        if (absPos.y >= absPos.x && absPos.y >= absPos.z)
            return float3(0.0f, sign(localPos.y), 0.0f);
        return float3(0.0f, 0.0f, sign(localPos.z));
    }

    return normalize(localPos);
}

float3 Tonemap(float3 color)
{
    color = max(color, 0.0f);
    color = color / (color + 1.0f);
    return pow(color, 1.0f / 2.2f);
}
