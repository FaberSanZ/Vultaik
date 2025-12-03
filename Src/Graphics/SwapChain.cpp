
#include <d3d11.h>
#include <dxgi.h>
#include <cstdint>
#include "Device.h"
#include "EngineData.h"
#include "SwapChain.h"

namespace Graphics
{
	bool SwapChain::Initialize(const Device& device, HWND hwnd, uint32_t width, uint32_t height)
	{
		ID3D11Device* d3dDevice = device.GetDevice();
		if (!d3dDevice)
		{
			//std::cerr << "[SwapChain] Device is null.\n";
			return false;
		}

		// Obtener el IDXGIFactory a trav?s del dispositivo
		IDXGIDevice* dxgiDevice = nullptr;
		if (FAILED(d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice)))
		{
			//std::cerr << "[SwapChain] Failed to query IDXGIDevice.\n";
			return false;
		}

		IDXGIAdapter* dxgiAdapter = nullptr;
		dxgiDevice->GetAdapter(&dxgiAdapter);

		IDXGIFactory* dxgiFactory = nullptr;
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

		dxgiDevice->Release();
		dxgiAdapter->Release();

		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferCount = 1;
		desc.BufferDesc.Width = width;
		desc.BufferDesc.Height = height;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.OutputWindow = hwnd;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		HRESULT hr = dxgiFactory->CreateSwapChain(d3dDevice, &desc, &m_SwapChain);
		dxgiFactory->Release();

		if (FAILED(hr))
		{
			//std::cerr << "[SwapChain] Failed to create swap chain.\n";
			return false;
		}

		//std::cout << "[SwapChain] Created successfully.\n";


		// Create a render target view for the back buffer

		m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
		auto result = d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
		backBuffer->Release();









		// Initialize and set up the description of the depth buffer.
		D3D11_TEXTURE2D_DESC depthbufferDesc = {};
		depthbufferDesc.Width = width;
		depthbufferDesc.Height = height;
		depthbufferDesc.MipLevels = 1;
		depthbufferDesc.ArraySize = 1;
		depthbufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthbufferDesc.SampleDesc.Count = 1;
		depthbufferDesc.SampleDesc.Quality = 0;
		depthbufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthbufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthbufferDesc.CPUAccessFlags = 0;
		depthbufferDesc.MiscFlags = 0;

		d3dDevice->CreateTexture2D(&depthbufferDesc, NULL, &depthBuffer);
		d3dDevice->CreateDepthStencilView(depthBuffer, NULL, &depthStencilView);


		// --- Inicializar FrameBuffer ---
		TextureData data = {};
		data.width = static_cast<float>(width);
		data.height = static_cast<float>(height);
		data.flags = static_cast<TextureFlags>(static_cast<int>(TextureFlags::RenderTarget) bitor static_cast<int>(TextureFlags::DepthStencil));
		data.iswapChain = true;


		// RederPass initialization
		m_RenderPass.m_Color.InitializeFromSwapChain(data, renderTargetView, depthStencilView, backBuffer);
		m_RenderPass.m_Depth.InitializeFromSwapChain(data, renderTargetView, depthStencilView, depthBuffer);




		return true;
	}





	void SwapChain::Present(bool vsync)
	{
		if (m_SwapChain)
			m_SwapChain->Present(vsync ? 1 : 0, 0);
	}

	void SwapChain::Release()
	{
		if (m_SwapChain)
		{
			m_SwapChain->Release();
			m_SwapChain = nullptr;
		}
	}

	ID3D11Texture2D* SwapChain::GetBackBuffer() const
	{
		return backBuffer;
	}

	ID3D11Texture2D* SwapChain::GetDepthBuffer() const
	{
		return depthBuffer;
	}


	SwapChain::~SwapChain()
	{
		Release();
	}
}