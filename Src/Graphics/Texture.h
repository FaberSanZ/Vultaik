#pragma once

#include <d3d11.h>
#include <cstdint>
#include <stdexcept>


#include "Device.h"
#include "EngineData.h" 

namespace Graphics
{
	class Device; // Forward declaration



	class Texture
	{

	public:
		Texture() = default;
		~Texture() = default;

		void InitializeFromSwapChain(const TextureData& data, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, ID3D11Texture2D* handle)
		{
			m_Data = data;
			m_Handle = handle;

			if (data.isRenderTarget())
				m_renderTargetView = renderTargetView;


			if (data.isDepthStencil())
				m_depthStencilView = depthStencilView;
		}

		ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView; }
		ID3D11DepthStencilView* GetDepthStencilView() const { return m_depthStencilView; }
		ID3D11Texture2D* GetHandle() const { return m_Handle; }
		const TextureData& GetData() const { return m_Data; }

		TextureData m_Data;

	private:
		ID3D11Texture2D* m_Handle = nullptr;
		ID3D11RenderTargetView* m_renderTargetView = nullptr;
		ID3D11DepthStencilView* m_depthStencilView = nullptr;
	};


} // namespace Graphics
