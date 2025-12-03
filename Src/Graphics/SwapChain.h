#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <cstdint>
#include "Device.h"
#include "EngineData.h"
#include "RenderPass.h"

#pragma comment(lib, "dxgi.lib")

namespace Graphics
{
	class Device;
	class RenderPass;

	class SwapChain
	{
	public:
		SwapChain() = default;
		~SwapChain();

		bool Initialize(const Device& device, HWND hwnd, uint32_t width, uint32_t height);
		void Present(bool vsync);
		void Release();
		ID3D11Texture2D* GetBackBuffer() const;
		ID3D11Texture2D* GetDepthBuffer() const;

		IDXGISwapChain* GetSwapChain() const { return m_SwapChain; }
		RenderPass& GetRenderPass() { return m_RenderPass; }



	private:
		IDXGISwapChain* m_SwapChain = nullptr;

		ID3D11Texture2D* backBuffer = nullptr;
		ID3D11Texture2D* depthBuffer = nullptr;

		ID3D11RenderTargetView* renderTargetView = nullptr;
		ID3D11DepthStencilView* depthStencilView = nullptr;

		RenderPass m_RenderPass;
	};
}

