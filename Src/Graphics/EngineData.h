#pragma once
#include <cstdint>
#include <type_traits> // por si usas enums como bitflags
#include <DirectXMath.h>

namespace Graphics
{
    using namespace DirectX;

    enum class TextureFlags
    {
        None = 0,
        RenderTarget = 1 << 0,
        DepthStencil = 1 << 1
    };

    enum class VertexType
    {
        Position,
        Normal,
        TextureCoordinate,
        Color,
        Tangent,
        Bitangent
    };

    struct TextureData
    {
        uint32_t width = 0;
        uint32_t height = 0;
        TextureFlags flags = TextureFlags::None;
		bool iswapChain{ true }; // TODO: 
		//uint8_t* data = nullptr; 

        bool HasFlag(TextureFlags flag) const
        {
            return (static_cast<int>(flags) bitand static_cast<int>(flag)) != 0;
        }

        bool isRenderTarget() const { return HasFlag(TextureFlags::RenderTarget); }
        bool isDepthStencil() const { return HasFlag(TextureFlags::DepthStencil); }
    };


    struct VertexPosition
    {
        XMFLOAT3 Position;
    };

    struct VertexPositionTex
    {
        XMFLOAT3 Position;
        XMFLOAT2 TexCoord;
    };

    struct VertexPositionNormal
    {
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
    };

    struct VertexPositionNormalTex
    {
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT2 TexCoord;
    };

    struct VertexPositionNormalTangentTex
    {
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT3 Tangent;
        XMFLOAT2 TexCoord;
    };

    struct VertexPositionColor
    {
        XMFLOAT4 Position;
        XMFLOAT4 Color;
    };


} // namespace Graphics