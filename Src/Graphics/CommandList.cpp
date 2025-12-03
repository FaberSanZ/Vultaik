#include "SwapChain.h"
#include "Device.h"
#include "Adapter.h"
#include "CommandList.h"

#include <iostream>
#include <dxgi.h>

#include <d3d11.h>
#include <cstdint>

#pragma comment(lib, "d3d11.lib")


namespace Graphics
{
	void CommandList::Initialize(ID3D11DeviceContext* context)
	{

		m_Context = context;

	}
	void CommandList::Release()
	{
		if (m_Context)
		{
			m_Context->Release();
			m_Context = nullptr;
		}
	}


	void CommandList::ClearRenderPass(const RenderPass& pass, const float color[4])
	{
		// TODO: Check if pass is valid
		TextureData colorData = pass.GetColorTexture().GetData();
		TextureData depthData = pass.GetDepthTexture().GetData();


		if (colorData.isRenderTarget())
			m_Context->ClearRenderTargetView(pass.GetColorTexture().GetRenderTargetView(), color);

		if (depthData.isDepthStencil())
			m_Context->ClearDepthStencilView(pass.GetDepthTexture().GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}


	void CommandList::SetRenderPass(const RenderPass& pass)
	{
		// TODO: Check if pass is valid
		TextureData colorData = pass.GetColorTexture().GetData();
		TextureData depthData = pass.GetDepthTexture().GetData();

		ID3D11RenderTargetView* rtvs[8] = {};
		for (uint32_t i = 0; i < 1; ++i)
			rtvs[i] = pass.GetColorTexture().GetRenderTargetView();

		if (colorData.isRenderTarget() && depthData.isDepthStencil())
			m_Context->OMSetRenderTargets(1, rtvs, pass.GetDepthTexture().GetDepthStencilView());

		else if (colorData.isRenderTarget() && !depthData.isDepthStencil())
			m_Context->OMSetRenderTargets(1, rtvs, nullptr);

	}

	void CommandList::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		if (m_Context)
		{
			m_Context->IASetPrimitiveTopology(topology);
		}
	}

	void CommandList::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
	{
		if (m_Context)
		{
			m_Context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
		}
	}

	void CommandList::DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
	{
		if (m_Context)
		{
			m_Context->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
		}
	}


	void CommandList::SetPipelineState(const Pipeline& pipelineState)
	{
		if (pipelineState.GetInputLayout())
			m_Context->IASetInputLayout(pipelineState.GetInputLayout());
		if (pipelineState.GetVertexShader())
			m_Context->VSSetShader(pipelineState.GetVertexShader(), nullptr, 0);
		if (pipelineState.GetPixelShader())
			m_Context->PSSetShader(pipelineState.GetPixelShader(), nullptr, 0);
		if (pipelineState.GetDepthStencilState())
			m_Context->OMSetDepthStencilState(pipelineState.GetDepthStencilState(), 0);
		if (pipelineState.GetRasterizerState())
			m_Context->RSSetState(pipelineState.GetRasterizerState());

	}
	void CommandList::SetViewport(float width, float height)
	{
		D3D11_VIEWPORT viewport = {};
		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		if (m_Context)
		{
			m_Context->RSSetViewports(1, &viewport);
		}
	}


	void CommandList::SetVertexBuffer(ID3D11Buffer* buffer, uint32_t slot, uint32_t size, uint32_t stride, void* data)
	{
		
		if (m_Context and buffer and data)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (SUCCEEDED(m_Context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			{
				memcpy(mappedResource.pData, data, size);
				m_Context->Unmap(buffer, 0);
				m_Context->IASetVertexBuffers(slot, 1, &buffer, &stride, nullptr);
			}
		}
	}

	void CommandList::SetIndexBuffer(ID3D11Buffer* buffer, DXGI_FORMAT format, uint32_t offset)
	{
		if (m_Context and buffer)
		{
			m_Context->IASetIndexBuffer(buffer, format, offset);
		}
	}
	void CommandList::SetConstantBuffer(ID3D11Buffer* buffer, uint32_t slot, uint32_t size, uint32_t stride, void* data)
	{
		if (m_Context and buffer and data)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (SUCCEEDED(m_Context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			{
				memcpy(mappedResource.pData, data, size);
				m_Context->Unmap(buffer, 0);
				m_Context->VSSetConstantBuffers(slot, 1, &buffer);
			}
		}
	}

	void CommandList::UpdateBuffer(Buffer& buffer, const void* data, uint32_t size)
	{

		if (!buffer.GetBuffer() and data)
			return;

		if (buffer.GetType() == BufferType::ConstantBuffer or buffer.GetType() == BufferType::StructuredBuffer)
		{
			D3D11_MAPPED_SUBRESOURCE mapped = {};
			HRESULT hr = m_Context->Map(buffer.GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			if (SUCCEEDED(hr))
			{
				memcpy(mapped.pData, data, size);
				m_Context->Unmap(buffer.GetBuffer(), 0);
			}
			else
			{
				std::cerr << "[Buffer] Failed to map buffer.\n";
			}
		}
	}



} // namespace Graphics