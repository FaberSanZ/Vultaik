#include "DXRCommon.hlsli"

struct Payload
{
    float3 color;
};

[shader("raygeneration")]
void RayGen()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchSize = DispatchRaysDimensions().xy;

    float2 pixel = (float2(launchIndex) + 0.5f) / float2(launchSize);
    float2 ndc = pixel * 2.0f - 1.0f;
    ndc.y = -ndc.y;

    float4 worldNear = mul(float4(ndc, 0.0f, 1.0f), g_Camera.invViewProj);
    float4 worldFar = mul(float4(ndc, 1.0f, 1.0f), g_Camera.invViewProj);
    worldNear.xyz /= max(worldNear.w, 0.0001f);
    worldFar.xyz /= max(worldFar.w, 0.0001f);

    float3 rayOrigin = g_Camera.cameraPosition.xyz;
    float3 rayDirection = normalize(worldFar.xyz - rayOrigin);

    RayDesc ray;
    ray.Origin = rayOrigin;
    ray.Direction = rayDirection;
    ray.TMin = 0.001f;
    ray.TMax = 10000.0f;

    Payload payload;
    payload.color = float3(0.0f, 0.0f, 0.0f);

    TraceRay(g_Scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);
    g_Output[launchIndex] = float4(Tonemap(payload.color), 1.0f);
}
