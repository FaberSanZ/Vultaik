#include "Pipeline.h"
#include <d3dcompiler.h>
#include <iostream>


#pragma comment(lib, "D3DCompiler.lib")
namespace Graphics
{
    void Graphics::Pipeline::Initialize(const Device& device, const PipelineDesc desc)
    {
        auto* device_ = device.GetDevice();
        auto* context = device.GetContext();

        D3D11_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode = desc.fillMode;
        rasterDesc.CullMode = desc.cullMode; // No culling
        rasterDesc.FrontCounterClockwise = FALSE;

        device_->CreateRasterizerState(&rasterDesc, &m_RasterizerState);


        if (desc.depthEnabled)
        {
            D3D11_DEPTH_STENCIL_DESC depthDesc = {};
            depthDesc.DepthEnable = TRUE;
            depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
            depthDesc.StencilEnable = desc.stencilEnabled;
            depthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
            depthDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
            device_->CreateDepthStencilState(&depthDesc, &m_DepthStencilState);
        }



        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* psBlob = nullptr;


        CompileShaderFromFile_(L"../../../../../Vultaik/Assets/Shaders/VertexShader.hlsl", "VS", "vs_5_0", &vsBlob);
        CompileShaderFromFile_(L"../../../../../Vultaik/Assets/Shaders/PixelShader.hlsl", "PS", "ps_5_0", &psBlob);



        device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_VertexShader);
        device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_PixelShader);


        device_->CreateInputLayout(desc.vertexInputElement.GetInputElementDescriptions().data(), 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);


        vsBlob->Release();
        psBlob->Release();

    }

    HRESULT Pipeline::CompileShaderFromFile_(const wchar_t* filename, const char* entryPoint, const char* profile, ID3DBlob** blob)
    {
        ID3DBlob* errorBlob = nullptr;
        HRESULT hr = D3DCompileFromFile(filename, nullptr, nullptr, entryPoint, profile, 0, 0, blob, &errorBlob);

        if (FAILED(hr))
        {
            if (errorBlob)
            {
                std::cerr << "Shader Error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
                errorBlob->Release();
            }
            else
            {
                std::cerr << "Error." << std::endl;
            }
            return hr;
        }

        return S_OK;
    }

    void Pipeline::Release()
    {
        if (m_InputLayout)
        {
            m_InputLayout->Release();
            m_InputLayout = nullptr;
        }
        if (m_VertexShader)
        {
            m_VertexShader->Release();
            m_VertexShader = nullptr;
        }
        if (m_PixelShader)
        {
            m_PixelShader->Release();
            m_PixelShader = nullptr;
        }
        if (m_DepthStencilState)
        {
            m_DepthStencilState->Release();
            m_DepthStencilState = nullptr;
        }
        if (m_RasterizerState)
        {
            m_RasterizerState->Release();
            m_RasterizerState = nullptr;
        }
	}




} // namespace Graphics