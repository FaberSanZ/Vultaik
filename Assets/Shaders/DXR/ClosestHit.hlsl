#include "DXRCommon.hlsli"

struct Payload
{
    float3 color;
};

[shader("closesthit")]
void ClosestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    const DxrInstanceData instance = g_Instances[InstanceID()];

    float3 localHit = ObjectRayOrigin() + RayTCurrent() * ObjectRayDirection();
    float3 localNormal = GetApproxNormal(instance.shapeType, localHit);

    float3x3 objectToWorld = (float3x3)ObjectToWorld3x4();
    float3 worldNormal = normalize(mul(localNormal, objectToWorld));

    float3 worldPos = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    float3 V = normalize(g_Camera.cameraPosition.xyz - worldPos);
    float3 L = normalize(-g_Camera.lightDirection.xyz);
    float3 H = normalize(V + L);

    float3 baseColor = saturate(instance.instance.baseColor.rgb);
    float metallic = saturate(instance.instance.material.x);
    float roughness = saturate(instance.instance.material.y);

    float2 uv = localHit.xz * 0.5f + 0.5f;
    if (instance.shapeType == SHAPE_PLANE || instance.shapeType == SHAPE_CUAD || instance.shapeType == SHAPE_TRIANGLE || instance.shapeType == SHAPE_PENTAGON || instance.shapeType == SHAPE_HEXAGON || instance.shapeType == SHAPE_CIRCLE)
        uv = localHit.xz * 0.5f + 0.5f;
    else if (instance.shapeType == SHAPE_CUBE)
        uv = localHit.zy * 0.5f + 0.5f;

    uint textureIndex = (uint)round(instance.instance.material.w);
    float3 sampled = g_Textures[textureIndex].SampleLevel(textureSampler, frac(uv), 0.0f).rgb;
    baseColor *= sampled;

    float NdotL = saturate(dot(worldNormal, L));
    float NdotV = saturate(dot(worldNormal, V));
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor, metallic);
    float3 F = F0 + (1.0f - F0) * pow(1.0f - saturate(dot(H, V)), 5.0f);
    float3 diffuse = baseColor * NdotL * (1.0f - metallic);
    float3 specular = F * pow(saturate(dot(worldNormal, H)), max(1.0f, 64.0f * (1.0f - roughness)));

    payload.color = diffuse * 1.2f + specular * 0.8f + baseColor * 0.04f;
}
