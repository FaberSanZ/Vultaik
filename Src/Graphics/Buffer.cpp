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
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.Usage = (type == BufferType::ConstantBuffer) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

		desc.BindFlags =
			(type == BufferType::VertexBuffer) ? D3D11_BIND_VERTEX_BUFFER :
			(type == BufferType::IndexBuffer) ? D3D11_BIND_INDEX_BUFFER :
			D3D11_BIND_CONSTANT_BUFFER;

		desc.CPUAccessFlags = (type == BufferType::ConstantBuffer) ? D3D11_CPU_ACCESS_WRITE : 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data;

		HRESULT hr = d3dDevice->CreateBuffer(&desc, &initData, &m_Buffer);
		if (FAILED(hr))
		{
			std::cerr << "[Buffer] Failed to create buffer.\n";
			return false;
		}

		return true;
	}

	void Buffer::Bind(ID3D11DeviceContext* context, uint32_t slot)
	{
		if (!m_Buffer || !context) return;

		uint32_t offset = { 0 };

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
			break;
		}
	}

	void Buffer::Update(ID3D11DeviceContext* context, const void* data, uint32_t size)
	{
		if (!m_Buffer || !context || m_Type != BufferType::ConstantBuffer)
			return;

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

	void Buffer::Release()
	{
		if (m_Buffer)
		{
			m_Buffer->Release();
			m_Buffer = nullptr;
		}
	}

	Buffer::~Buffer()
	{
		Release();
	}
}