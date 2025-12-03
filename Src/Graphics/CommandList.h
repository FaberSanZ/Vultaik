#pragma once
#include "SwapChain.h"
#include "Device.h"
#include "Adapter.h"
#include "CommandList.h"
#include "Texture.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Buffer.h"

#include <iostream>
#include <dxgi.h>

#include <d3d11.h>
#include <cstdint>

#pragma comment(lib, "d3d11.lib")


namespace Graphics
{
	class RenderPass;
	class Pipeline;
	class Buffer;

	class CommandList
	{
	public:
		CommandList() = default;
		~CommandList() = default;
		void Initialize(ID3D11DeviceContext* context);
		void Release();
		void ClearRenderPass(const RenderPass& pass, const float color[4]);
		void SetViewport(float width, float height);

		void SetVertexBuffer(ID3D11Buffer* buffer, uint32_t slot, uint32_t size, uint32_t stride, void* data);
		void SetIndexBuffer(ID3D11Buffer* buffer, DXGI_FORMAT format, uint32_t offset);
		void SetConstantBuffer(ID3D11Buffer* buffer, uint32_t slot, uint32_t size, uint32_t stride, void* data);

		void SetRenderPass(const RenderPass& pass);
		void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
		void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation);
		void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation);

		void SetPipelineState(const Pipeline& pipelineState);
		void UpdateBuffer(Buffer& buffer, const void* data, uint32_t size);



	private:
		ID3D11DeviceContext* m_Context = nullptr; // Direct3D device context for executing commands
	};
}
