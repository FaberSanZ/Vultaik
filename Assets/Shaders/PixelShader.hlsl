cbuffer constants : register(b0)
{
    float2 rn_screensize;
    float2 r_atlassize;
}

struct sprite
{
    int2 screenpos;
    int2 size;
    int2 atlaspos;
    float scale;
    float rotation; // A�adir rotaci�n
    float2 pivot; // A�adir pivot
};

struct pixel
{
    float4 xy : SV_POSITION;
    float2 uv : UV;
};

StructuredBuffer<sprite> spritebuffer : register(t0);
Texture2D<float4> atlastexture : register(t1);
SamplerState pointsampler : register(s0);

pixel vs(uint spriteid : SV_INSTANCEID, uint vertexid : SV_VERTEXID)
{
    sprite spr = spritebuffer[spriteid];

    // Tu t�cnica original para generar �ndices
    uint2 i = { vertexid & 2, (vertexid << 1 & 2) ^ 3 };
    
    // Calcular tama�o escalado
    float2 scaledSize = spr.size * spr.scale;
    
    // Coordenadas UV (igual que antes - no cambian)
    float4 tex = float4(spr.atlaspos, spr.atlaspos + spr.size);
    float2 uv = float2(tex[i.x], tex[i.y]) * r_atlassize;
    
    // Posiciones de v�rtice originales (sin rotaci�n)
    float4 originalPos = float4(spr.screenpos, spr.screenpos + scaledSize);
    float2 originalVertex = float2(originalPos[i.x], originalPos[i.y]);
    
    // Aplicar rotaci�n si es necesaria
    float2 finalVertex = originalVertex;
    if (spr.rotation != 0.0f)
    {
        // Calcular el centro de rotaci�n en coordenadas de mundo
        float2 pivotWorld = spr.screenpos + (spr.pivot * scaledSize);
        
        // Vector desde el pivot al v�rtice actual
        float2 offset = originalVertex - pivotWorld;
        
        // Aplicar rotaci�n
        float cosA = cos(spr.rotation);
        float sinA = sin(spr.rotation);
        float2 rotatedOffset = float2(
            offset.x * cosA - offset.y * sinA,
            offset.x * sinA + offset.y * cosA
        );
        
        // Posici�n final rotada
        finalVertex = pivotWorld + rotatedOffset;
    }
    
    pixel p;
    p.xy = float4(finalVertex * rn_screensize - float2(1, -1), 0, 1);
    p.uv = uv;

    return p;
}

float4 ps(pixel p) : SV_TARGET
{
    float4 color = atlastexture.Sample(pointsampler, p.uv);

    if (color.a == 0)
        discard;

    return color;
}