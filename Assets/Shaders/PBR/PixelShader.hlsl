static const float PI = 3.141592654f;

Texture2D g_Textures[] : register(t1);
SamplerState textureSampler : register(s0);

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

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
    return a2 / max(denom, 0.0001f);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    float denom = NdotV * (1.0f - k) + k;
    return NdotV / max(denom, 0.0001f);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 ACESFilm(float3 x)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float4 PS(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 V = normalize(input.cameraVector);
    float3 L = normalize(-lightDirection.xyz);
    float3 H = normalize(V + L);

    float3 baseColor = saturate(input.baseColor.rgb);
    float3 albedo = baseColor;

    uint textureIndex = (uint)round(input.material.w);
    float3 sampled = g_Textures[textureIndex].Sample(textureSampler, input.uv).rgb;
    albedo = saturate(sampled * baseColor);

    float metallic = saturate(input.material.x);
    float roughness = saturate(input.material.y);
    float ao = saturate(input.material.z);

    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 numerator = NDF * G * F;
    float denom = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.001f;
    float3 specular = numerator / denom;

    float3 kS = F;
    float3 kD = (1.0f - kS) * (1.0f - metallic);
    float NdotL = max(dot(N, L), 0.0f);
    float3 diffuse = kD * albedo / PI;

    float3 hemiSky = float3(0.48f, 0.58f, 0.72f);
    float3 hemiGround = float3(0.12f, 0.12f, 0.14f);
    float hemiFactor = saturate(N.y * 0.5f + 0.5f);
    float3 ambient = lerp(hemiGround, hemiSky, hemiFactor) * albedo * ao * 0.45f;

    float3 lightColor = float3(1.0f, 0.98f, 0.94f);
    float3 direct = (diffuse + specular) * NdotL * lightColor * 3.2f;

    float3 fillDir = normalize(float3(-L.z, 0.35f, L.x));
    float fillLight = saturate(dot(N, fillDir)) * 0.10f;
    float3 color = ambient + direct + albedo * fillLight;
    color = ACESFilm(color * 1.15f);
    color = pow(color, 1.0f / 2.2f);

    return float4(color, 1.0f);
}
