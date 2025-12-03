#include "Buffer.h"
#include "Device.h"
#include <iostream>

namespace Graphics
{
	bool Buffer::Initialize(const Device& device, BufferType type, const void* data, uint32_t size, uint32_t stride)
	{
		m_Type = type;
		m_Stride = stride;

		ID3D11Device* d3dDevice = device.GetDevice();

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = size;
		desc.Usage = (type == BufferType::ConstantBuffer) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

		desc.BindFlags =
			(type == BufferType::VertexBuffer) ? D3D11_BIND_VERTEX_BUFFER :
			(type == BufferType::IndexBuffer) ? D3D11_BIND_INDEX_BUFFER :
			D3D11_BIND_CONSTANT_BUFFER;

		desc.CPUAccessFlags = (type == BufferType::ConstantBuffer) ? D3D11_CPU_ACCESS_WRITE : 0;

		if (type == BufferType::StructuredBuffer)
		{
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			desc.StructureByteStride = stride;  
		}


		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data;

		HRESULT hr = d3dDevice->CreateBuffer(&desc, data ? &initData : nullptr, &m_Buffer);
		if (FAILED(hr))
		{
			std::cerr << "[Buffer] Failed to create buffer.\n";
			return false;
		}


		if (type == BufferType::StructuredBuffer)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;;

			hr = d3dDevice->CreateShaderResourceView(m_Buffer, &srvDesc, &m_SRV);
			if (FAILED(hr))
			{
				std::cerr << "[Buffer] Failed to create ShaderResourceView for StructuredBuffer.\n";
				m_Buffer->Release();
				m_Buffer = nullptr;
				return false;
			}
		}

		return true;
	}

	void Buffer::Bind(ID3D11DeviceContext* context, uint32_t slot)
	{
		if (!m_Buffer || !context) return;

		uint32_t offset = 0;

		switch (m_Type)
		{
		case BufferType::VertexBuffer:
			context->IASetVertexBuffers(slot, 1, &m_Buffer, &m_Stride, &offset);
			break;

		case BufferType::IndexBuffer:
			context->IASetIndexBuffer(m_Buffer, DXGI_FORMAT_R32_UINT, 0);
			break;

		case BufferType::ConstantBuffer:
			context->VSSetConstantBuffers(slot, 1, &m_Buffer);

			//context->PSSetConstantBuffers(slot, 1, &m_Buffer);
			//context->CSSetConstantBuffers(slot, 1, &m_Buffer);
			break;

		case BufferType::StructuredBuffer:
			context->VSSetShaderResources(slot, 1, &m_SRV);
			//context->CSSetShaderResources(slot, 1, &m_SRV);
			//context->PSSetShaderResources(slot, 1, &m_SRV);

			break;

		default:
			std::cerr << "[Buffer] Unknown buffer type.\n";
			break;
		}
	}

	void Buffer::Update(ID3D11DeviceContext* context, const void* data, uint32_t size)
	{
		if (!m_Buffer || !context)
			return;

		// Para ConstantBuffers (dynamic)
		if (m_Type == BufferType::ConstantBuffer)
		{
			D3D11_MAPPED_SUBRESOURCE mapped = {};
			HRESULT hr = context->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			if (SUCCEEDED(hr))
			{
				memcpy(mapped.pData, data, size);
				context->Unmap(m_Buffer, 0);
			}
			else
			{
				std::cerr << "[Buffer] Failed to map constant buffer.\n";
			}
		}


		else if (m_Type == BufferType::StructuredBuffer)
		{

			D3D11_MAPPED_SUBRESOURCE mapped = {};
			context->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

			memcpy(mapped.pData, data, size);

			context->Unmap(m_Buffer, 0);

		}
	}

	void Buffer::Release()
	{
		if (m_SRV)
		{
			m_SRV->Release();
			m_SRV = nullptr;
		}

		if (m_Buffer)
		{
			m_Buffer->Release();
			m_Buffer = nullptr;
		}

		m_Type = BufferType::VertexBuffer; // Reset to default
		m_Stride = 0;
	}

	Buffer::~Buffer()
	{
		Release();
	}
}