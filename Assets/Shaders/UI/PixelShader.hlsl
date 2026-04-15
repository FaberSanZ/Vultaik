Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 PS(PSInput input) : SV_TARGET
{
    if (input.uv.x == 0.0f && input.uv.y == 0.0f)
        return input.color;
    else
        return g_texture.Sample(g_sampler, input.uv) * input.color;
}