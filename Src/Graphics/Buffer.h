#pragma once


#include <d3d11.h>
#include <cstdint>


namespace Graphics
{
	enum class BufferType
	{
		VertexBuffer,
		IndexBuffer,
		ConstantBuffer,
		StructuredBuffer
	};

	class Device;

	class Buffer
	{
	public:
		Buffer() = default;
		~Buffer();

		bool Initialize(const Device& device, BufferType type, const void* data, uint32_t size, uint32_t stride = 0);
		void Bind(ID3D11DeviceContext* context, uint32_t slot = 0);
		void SetData(ID3D11DeviceContext* context, const void* newData, uint32_t size);

		void Release();

		ID3D11Buffer* GetBuffer() const { return m_Buffer; }
		BufferType GetType() const { return m_Type; }
		uint32_t GetStride() const { return m_Stride; }

	private:
		ID3D11Buffer* m_Buffer = nullptr;
		ID3D11ShaderResourceView* m_SRV = nullptr;  

		BufferType m_Type = BufferType::VertexBuffer;
		uint32_t m_Stride = 0; // Needed only for vertex buffers
	};
}
