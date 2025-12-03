#pragma once
#include <d3d11.h>
#include "Device.h"
#include "VertexInputElement.h"


#pragma comment(lib, "D3DCompiler.lib")

namespace Graphics
{
	class Device;

	struct PipelineDesc
	{
		D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID;
		D3D11_CULL_MODE cullMode = D3D11_CULL_BACK;
		bool depthEnabled = true;
		bool stencilEnabled = false;

		Graphics::VertexInputElement vertexInputElement; // Vertex input element description
	};

	class Pipeline
	{
	public:
		Pipeline() = default;
		~Pipeline() = default;
		void Initialize(const Device& device, const PipelineDesc desc);
		HRESULT CompileShaderFromFile_(const wchar_t* filename, const char* entryPoint, const char* profile, ID3DBlob** blob);

		void Release();
		ID3D11InputLayout* GetInputLayout() const { return m_InputLayout; }
		ID3D11VertexShader* GetVertexShader() const { return m_VertexShader; }
		ID3D11PixelShader* GetPixelShader() const { return m_PixelShader; }
		ID3D11DepthStencilState* GetDepthStencilState() const { return m_DepthStencilState; }
		ID3D11RasterizerState* GetRasterizerState() const { return m_RasterizerState; }
	private:
		ID3D11Device* m_Device = nullptr;
		ID3D11DeviceContext* m_Context = nullptr;
		ID3D11VertexShader* m_VertexShader = nullptr;
		ID3D11PixelShader* m_PixelShader = nullptr;
		ID3D11InputLayout* m_InputLayout = nullptr;
		ID3D11DepthStencilState* m_DepthStencilState = nullptr;
		ID3D11RasterizerState* m_RasterizerState = nullptr;

	};
}
