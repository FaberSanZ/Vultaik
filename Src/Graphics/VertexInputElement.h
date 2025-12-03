#pragma once
#include <d3d11.h>
#include <vector>
#include <string>
#include "EngineData.h"


namespace Graphics
{
    class VertexInputElement
    {
    public:
        VertexInputElement() = default;

        void Add(VertexType type, UINT slot = 0)
        {
            D3D11_INPUT_ELEMENT_DESC desc{};
            desc.InputSlot = slot;
            desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            desc.InstanceDataStepRate = 0;

            switch (type)
            {
            case VertexType::Position:
                desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                desc.SemanticName = "POSITION";
                desc.SemanticIndex = 0;
                desc.AlignedByteOffset = currentOffset; // 4 * 4 bytes
            case VertexType::Normal:
            case VertexType::Tangent:
            case VertexType::Bitangent:
                desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                break;
            case VertexType::TextureCoordinate:
                desc.Format = DXGI_FORMAT_R32G32_FLOAT;
                break;
            case VertexType::Color:
                desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                desc.SemanticName = "COLOR";
                desc.SemanticIndex = 0;
                desc.AlignedByteOffset = currentOffset; // 4 * 4 bytes
                break;
            }

            inputElementDescriptions.push_back(desc);
            currentOffset += Size(type);
        }

        void Reset()
        {
            inputElementDescriptions.clear();
            semanticStorage.clear();
            currentOffset = 0;
        }

        const std::vector<D3D11_INPUT_ELEMENT_DESC>& GetInputElementDescriptions() const
        {
            return inputElementDescriptions;
        }

    private:
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescriptions;
        std::vector<std::string> semanticStorage;
        UINT currentOffset = 0;

        uint32_t Size(VertexType type)
        {
            switch (type)
            {
            case VertexType::Position: return 16;
            case VertexType::Normal: return 16;
			case VertexType::TextureCoordinate: return 8; // TODO:
            case VertexType::Color: return 16;
            case VertexType::Tangent: return 16;
            case VertexType::Bitangent: return 16;
            default: return 0;
            }
        }

    };
}


