#include "DXRCommon.hlsli"

struct Payload
{
    float3 color;
};

[shader("miss")]
void Miss(inout Payload payload)
{
    payload.color = float3(0.67f, 0.82f, 0.94f);
}
