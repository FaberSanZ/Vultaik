#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "ShaderCompiler.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

struct Adapter
{
    IDXGIAdapter3* adapter = nullptr;
	std::wstring name;
    uint32_t memory;
	uint32_t gpuCount;
};

struct Device
{
	uint32_t id; // Unique identifier for the device (could be index or custom ID)
    ID3D12Device* device = nullptr;
	uint32_t nodeIndex = 0; // For multi-GPU setups, this indicates which GPU node the device is associated with
};

struct SwapChain
{
    IDXGISwapChain3* swapChain = nullptr;
    uint32_t bufferCount = 0;
};

class DescriptorHeap
{
public:
    ID3D12DescriptorHeap* m_Heap = nullptr;
    uint32_t m_DescriptorSize = 0;

    void Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible = false)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = type;
        heapDesc.NumDescriptors = numDescriptors;

        if (shaderVisible)
        {
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        }
        else
        {
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        }
        device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap));
        m_DescriptorSize = device->GetDescriptorHandleIncrementSize(type);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_Heap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_DescriptorSize;
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = m_Heap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_DescriptorSize;
        return handle;
    }

    void Destroy() { if (m_Heap) { m_Heap->Release(); m_Heap = nullptr; } }
};

class VertexBuffer
{
public:
    ID3D12Resource* m_Resource = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_View = {};
    uint32_t m_Size = 0; // in bytes

    void Destroy() { if (m_Resource) { m_Resource->Release(); m_Resource = nullptr; } }
};

class IndexBuffer
{
public:
    ID3D12Resource* m_Resource = nullptr;
    D3D12_INDEX_BUFFER_VIEW m_View = {};
    uint32_t m_Size = 0; // in bytes
    uint32_t m_IndexCount = 0;

    void Destroy() { if (m_Resource) { m_Resource->Release(); m_Resource = nullptr; } }
};

class StructuredBuffer
{
public:
    ID3D12Resource* m_Resource = nullptr;
    void Destroy() { if (m_Resource) { m_Resource->Release(); m_Resource = nullptr; } }
};

// ------------------------------------------------------------------
// Vertex structures
// ------------------------------------------------------------------
struct Vertex
{
    float position[4];
};

struct uiVertex
{
    float position[4];
    float color[4];
    float uv[2];
};

struct InstanceData
{
    DirectX::XMMATRIX worldMatrix;
    DirectX::XMFLOAT4 color;
};

// ------------------------------------------------------------------
// Mesh (game objects)
// ------------------------------------------------------------------
struct Mesh
{
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    StructuredBuffer instanceBuffer;
    uint32_t maxInstanceCount = 128 * 128;
    uint32_t currentInstanceCount = 0;

    void Draw(ID3D12GraphicsCommandList* cmdList)
    {
        cmdList->IASetVertexBuffers(0, 1, &vertexBuffer.m_View);
        cmdList->IASetIndexBuffer(&indexBuffer.m_View);
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->SetGraphicsRootShaderResourceView(1, instanceBuffer.m_Resource->GetGPUVirtualAddress());
        cmdList->DrawIndexedInstanced(indexBuffer.m_IndexCount, currentInstanceCount, 0, 0, 0);
    }

    void Destroy()
    {
        vertexBuffer.Destroy();
        indexBuffer.Destroy();
        instanceBuffer.Destroy();
    }
};

// ------------------------------------------------------------------
// UIMesh (UI elements) with dynamic resizing (like ImGui)
// ------------------------------------------------------------------
struct UIMesh
{
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    StructuredBuffer instanceBuffer;
    uint32_t maxInstanceCount = 128 * 128;
    uint32_t currentInstanceCount = 0;

    void Draw(ID3D12GraphicsCommandList* cmdList)
    {
        cmdList->IASetVertexBuffers(0, 1, &vertexBuffer.m_View);
        cmdList->IASetIndexBuffer(&indexBuffer.m_View);
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->DrawIndexedInstanced(indexBuffer.m_IndexCount, currentInstanceCount, 0, 0, 0);
    }

    void Destroy()
    {
        vertexBuffer.Destroy();
        indexBuffer.Destroy();
        instanceBuffer.Destroy();
    }
};

// ------------------------------------------------------------------
// Main Render class (D3D12)
// ------------------------------------------------------------------
class Render
{
public:
    Render() = default;

    bool Initialize(HWND hwnd, uint32_t width, uint32_t height);
    void Cleanup();


    DescriptorHeap CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count, bool shaderVisible = false);
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const DescriptorHeap& heap, uint32_t index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const DescriptorHeap& heap, uint32_t index);


    // Frame management
    void Reset();
    void Clear();
    void BeginFrame();   // sets game PSO and root signature
    void BeginGame();     // switches to game PSO and root signature
    void BeginUI();      // switches to UI PSO and root signature
    void Loop();         // closes command list, executes, presents


    // Mesh creation
    Mesh CreateMesh(Vertex* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount);
    UIMesh CreateUIMesh(uiVertex* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount);

    // Dynamic updates (for game instances)
    void UpdateInstanceBuffer(Mesh& mesh, InstanceData* data, uint32_t count);
    void UpdateUIInstanceBuffer(UIMesh& mesh, InstanceData* data, uint32_t count);

    // Dynamic updates for UI geometry (with auto-resize)
    void UpdateUIGeometry(UIMesh& mesh, uiVertex* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount);

    // Command list access (for external draws if needed)
    ID3D12GraphicsCommandList* commandList = nullptr;

private:
    // Device & swapchain
    ID3D12Device* m_Device = nullptr;
    ID3D12CommandQueue* m_CommandQueue = nullptr;
    IDXGISwapChain3* m_SwapChain = nullptr;
    ID3D12CommandAllocator* m_CommandAlloc = nullptr;
    ID3D12Resource* m_RenderTargets[2] = {};
    ID3D12Resource* m_DepthStencilBuffer = nullptr;

    // Descriptor heaps
    DescriptorHeap m_RTVHeap;
    DescriptorHeap m_DSVHeap;
    DescriptorHeap m_SRVHeap;

    // Pipelines and root signatures
    ID3D12RootSignature* m_GameRootSig = nullptr;
    ID3D12PipelineState* m_GamePSO = nullptr;
    ID3D12RootSignature* m_UIRootSig = nullptr;
    ID3D12PipelineState* m_UIPSO = nullptr;

    // Matrices
    DirectX::XMMATRIX m_GameProjMatrix;
    DirectX::XMMATRIX m_UIProjMatrix;

    // Dimensions
    uint32_t m_Width = 0, m_Height = 0;
    uint32_t m_FrameCount = 2;

    // Shader compiler
    Core::ShaderCompiler m_ShaderCompiler;

    // Helpers
    void CreateDepthBuffer();
    void CreateGamePipeline();
    void CreateUIPipeline();
    void SetGameRootConstants();
    void SetUIRootConstants();
    StructuredBuffer CreateStructuredBuffer(uint32_t size, uint32_t stride);
    void EnsureUIMeshSize(UIMesh& mesh, uint32_t neededVertexCount, uint32_t neededIndexCount);
    void UpdateUIVertexBuffer(UIMesh& mesh, uiVertex* vertices, uint32_t count);
    void UpdateUIIndexBuffer(UIMesh& mesh, uint32_t* indices, uint32_t count);
};

// ------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------

bool Render::Initialize(HWND hwnd, uint32_t width, uint32_t height)
{
    m_Width = width;
    m_Height = height;

    IDXGIFactory4* factory = nullptr;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = m_FrameCount;
    swapChainDesc.Width = m_Width;
    swapChainDesc.Height = m_Height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    IDXGISwapChain1* tempSwapChain = nullptr;
    factory->CreateSwapChainForHwnd(m_CommandQueue, hwnd, &swapChainDesc, nullptr, nullptr, &tempSwapChain);
    tempSwapChain->QueryInterface(IID_PPV_ARGS(&m_SwapChain));
    factory->Release();

    m_RTVHeap.Initialize(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_FrameCount);
    for (uint32_t i = 0; i < m_FrameCount; ++i)
    {
        m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[i]));
        m_Device->CreateRenderTargetView(m_RenderTargets[i], nullptr, m_RTVHeap.GetCPUHandle(i));
    }

    m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAlloc));
    m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAlloc, nullptr, IID_PPV_ARGS(&commandList));
    commandList->Close();

    CreateDepthBuffer();
    CreateGamePipeline();
    CreateUIPipeline();

    // Orthographic projection for game (centered, Y up)
    float aspect = (float)width / height;
    m_GameProjMatrix = DirectX::XMMatrixOrthographicOffCenterLH(-2.0f * aspect, 2.0f * aspect, -2.0f, 2.0f, 0.1f, 100.0f);

    // UI orthographic: pixel-perfect (0,0) top-left, (width,height) bottom-right
    m_UIProjMatrix = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (float)width, (float)height, 0.0f, 0.0f, 1.0f);
    m_UIProjMatrix = DirectX::XMMatrixTranspose(m_UIProjMatrix);



    m_SRVHeap.Initialize(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024, true);


    return true;
}

void Render::CreateDepthBuffer()
{
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = m_Width;
    depthDesc.Height = m_Height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    clearValue.DepthStencil.Depth = 1.0f;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_DepthStencilBuffer));

    m_DSVHeap.Initialize(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    m_Device->CreateDepthStencilView(m_DepthStencilBuffer, &dsvDesc, m_DSVHeap.GetCPUHandle(0));
}

void Render::CreateGamePipeline()
{
    auto vertexShaderBlob = m_ShaderCompiler.Compile(L"../../../Assets/Shaders/DepthTests/VertexShader.hlsl", L"VS", L"vs_6_0");
    auto pixelShaderBlob = m_ShaderCompiler.Compile(L"../../../Assets/Shaders/DepthTests/PixelShader.hlsl", L"PS", L"ps_6_0");

    D3D12_ROOT_PARAMETER rootParams[2];

    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParams[0].Constants.Num32BitValues = 16;
    rootParams[0].Constants.ShaderRegister = 0;
    rootParams[0].Constants.RegisterSpace = 0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParams[1].Descriptor.ShaderRegister = 0;
    rootParams[1].Descriptor.RegisterSpace = 0;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 2;
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* sigBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob);
    m_Device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_GameRootSig));
    sigBlob->Release();

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_GameRootSig;
    psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_GamePSO));
}

void Render::CreateUIPipeline()
{
    auto vertexShaderBlob = m_ShaderCompiler.Compile(L"../../../Assets/Shaders/UI/VertexShader.hlsl", L"VS", L"vs_6_0");
    auto pixelShaderBlob = m_ShaderCompiler.Compile(L"../../../Assets/Shaders/UI/PixelShader.hlsl", L"PS", L"ps_6_0");

    // Root parameters: constants (matriz) + SRV (textura)
    D3D12_ROOT_PARAMETER rootParams[2];
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParams[0].Constants.Num32BitValues = 16; // matriz 4x4
    rootParams[0].Constants.ShaderRegister = 0;
    rootParams[0].Constants.RegisterSpace = 0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_DESCRIPTOR_RANGE range = {};
    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors = 1;
    range.BaseShaderRegister = 0;
    range.RegisterSpace = 0;
    range.OffsetInDescriptorsFromTableStart = 0;

    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[1].DescriptorTable.pDescriptorRanges = &range;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 2;
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.NumStaticSamplers = 1;               // ? ahora s?, un sampler
    rootSigDesc.pStaticSamplers = &sampler;          // ? apuntar al sampler
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* sigBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob);
    if (FAILED(hr)) {
        // Imprimir errorBlob si es necesario
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        return;
    }
    m_Device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_UIRootSig));
    sigBlob->Release();
    if (errorBlob) errorBlob->Release();

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Blend state para transparencia
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_UIRootSig;
    psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_UIPSO));
}

void Render::SetGameRootConstants()
{
    commandList->SetGraphicsRoot32BitConstants(0, 16, &m_GameProjMatrix, 0);
}

void Render::SetUIRootConstants()
{
    commandList->SetGraphicsRoot32BitConstants(0, 16, &m_UIProjMatrix, 0);
}

StructuredBuffer Render::CreateStructuredBuffer(uint32_t size, uint32_t stride)
{
    StructuredBuffer buffer;
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = size;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer.m_Resource));
    return buffer;
}

void Render::EnsureUIMeshSize(UIMesh& mesh, uint32_t neededVertexCount, uint32_t neededIndexCount)
{
    bool needRecreate = false;
    if (!mesh.vertexBuffer.m_Resource || mesh.vertexBuffer.m_Size < neededVertexCount * sizeof(uiVertex))
        needRecreate = true;
    if (!mesh.indexBuffer.m_Resource || mesh.indexBuffer.m_Size < neededIndexCount * sizeof(uint32_t))
        needRecreate = true;

    if (!needRecreate) return;

    // Destroy old buffers
    mesh.vertexBuffer.Destroy();
    mesh.indexBuffer.Destroy();
    mesh.instanceBuffer.Destroy();

    // Create with extra margin (like ImGui)
    uint32_t newVertexCapacity = neededVertexCount + 1024;
    uint32_t newIndexCapacity = neededIndexCount + 2048;

    // Vertex buffer
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = newVertexCapacity * sizeof(uiVertex);
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mesh.vertexBuffer.m_Resource));
    mesh.vertexBuffer.m_Size = newVertexCapacity * sizeof(uiVertex);
    mesh.vertexBuffer.m_View.BufferLocation = mesh.vertexBuffer.m_Resource->GetGPUVirtualAddress();
    mesh.vertexBuffer.m_View.StrideInBytes = sizeof(uiVertex);
    mesh.vertexBuffer.m_View.SizeInBytes = mesh.vertexBuffer.m_Size;

    // Index buffer
    bufferDesc.Width = newIndexCapacity * sizeof(uint32_t);
    m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mesh.indexBuffer.m_Resource));
    mesh.indexBuffer.m_Size = newIndexCapacity * sizeof(uint32_t);
    mesh.indexBuffer.m_View.BufferLocation = mesh.indexBuffer.m_Resource->GetGPUVirtualAddress();
    mesh.indexBuffer.m_View.Format = DXGI_FORMAT_R32_UINT;
    mesh.indexBuffer.m_View.SizeInBytes = mesh.indexBuffer.m_Size;
    mesh.indexBuffer.m_IndexCount = 0;

    // Instance buffer (same size as before)
    mesh.instanceBuffer = CreateStructuredBuffer(mesh.maxInstanceCount * sizeof(InstanceData), sizeof(InstanceData));
}

void Render::UpdateUIVertexBuffer(UIMesh& mesh, uiVertex* vertices, uint32_t count)
{
    void* data;
    mesh.vertexBuffer.m_Resource->Map(0, nullptr, &data);
    memcpy(data, vertices, count * sizeof(uiVertex));
    mesh.vertexBuffer.m_Resource->Unmap(0, nullptr);
    mesh.vertexBuffer.m_View.SizeInBytes = count * sizeof(uiVertex);
}

void Render::UpdateUIIndexBuffer(UIMesh& mesh, uint32_t* indices, uint32_t count)
{
    void* data;
    mesh.indexBuffer.m_Resource->Map(0, nullptr, &data);
    memcpy(data, indices, count * sizeof(uint32_t));
    mesh.indexBuffer.m_Resource->Unmap(0, nullptr);
    mesh.indexBuffer.m_View.SizeInBytes = count * sizeof(uint32_t);
    mesh.indexBuffer.m_IndexCount = count;
}

void Render::UpdateUIGeometry(UIMesh& mesh, uiVertex* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount)
{
    EnsureUIMeshSize(mesh, vertexCount, indexCount);
    UpdateUIVertexBuffer(mesh, vertices, vertexCount);
    UpdateUIIndexBuffer(mesh, indices, indexCount);
}

Mesh Render::CreateMesh(Vertex* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount)
{
    Mesh mesh;
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    // Vertex buffer
    D3D12_RESOURCE_DESC vbDesc = {};
    vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vbDesc.Width = vertexCount * sizeof(Vertex);
    vbDesc.Height = 1;
    vbDesc.DepthOrArraySize = 1;
    vbDesc.MipLevels = 1;
    vbDesc.Format = DXGI_FORMAT_UNKNOWN;
    vbDesc.SampleDesc.Count = 1;
    vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &vbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mesh.vertexBuffer.m_Resource));
    void* data;
    mesh.vertexBuffer.m_Resource->Map(0, nullptr, &data);
    memcpy(data, vertices, vertexCount * sizeof(Vertex));
    mesh.vertexBuffer.m_Resource->Unmap(0, nullptr);
    mesh.vertexBuffer.m_View.BufferLocation = mesh.vertexBuffer.m_Resource->GetGPUVirtualAddress();
    mesh.vertexBuffer.m_View.StrideInBytes = sizeof(Vertex);
    mesh.vertexBuffer.m_View.SizeInBytes = vertexCount * sizeof(Vertex);
    mesh.vertexBuffer.m_Size = vertexCount * sizeof(Vertex);

    // Index buffer
    D3D12_RESOURCE_DESC ibDesc = vbDesc;
    ibDesc.Width = indexCount * sizeof(uint32_t);
    m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &ibDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mesh.indexBuffer.m_Resource));
    mesh.indexBuffer.m_Resource->Map(0, nullptr, &data);
    memcpy(data, indices, indexCount * sizeof(uint32_t));
    mesh.indexBuffer.m_Resource->Unmap(0, nullptr);
    mesh.indexBuffer.m_View.BufferLocation = mesh.indexBuffer.m_Resource->GetGPUVirtualAddress();
    mesh.indexBuffer.m_View.Format = DXGI_FORMAT_R32_UINT;
    mesh.indexBuffer.m_View.SizeInBytes = indexCount * sizeof(uint32_t);
    mesh.indexBuffer.m_Size = indexCount * sizeof(uint32_t);
    mesh.indexBuffer.m_IndexCount = indexCount;

    mesh.instanceBuffer = CreateStructuredBuffer(mesh.maxInstanceCount * sizeof(InstanceData), sizeof(InstanceData));
    return mesh;
}

UIMesh Render::CreateUIMesh(uiVertex* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount)
{
    UIMesh mesh;
    EnsureUIMeshSize(mesh, vertexCount, indexCount);
    UpdateUIVertexBuffer(mesh, vertices, vertexCount);
    UpdateUIIndexBuffer(mesh, indices, indexCount);
    mesh.instanceBuffer = CreateStructuredBuffer(mesh.maxInstanceCount * sizeof(InstanceData), sizeof(InstanceData));
    return mesh;
}

void Render::UpdateInstanceBuffer(Mesh& mesh, InstanceData* data, uint32_t count)
{
    if (count > mesh.maxInstanceCount) count = mesh.maxInstanceCount;
    mesh.currentInstanceCount = count;
    void* ptr;
    mesh.instanceBuffer.m_Resource->Map(0, nullptr, &ptr);
    memcpy(ptr, data, count * sizeof(InstanceData));
    mesh.instanceBuffer.m_Resource->Unmap(0, nullptr);
}

void Render::UpdateUIInstanceBuffer(UIMesh& mesh, InstanceData* data, uint32_t count)
{
    if (count > mesh.maxInstanceCount) count = mesh.maxInstanceCount;
    mesh.currentInstanceCount = count;
    void* ptr;
    mesh.instanceBuffer.m_Resource->Map(0, nullptr, &ptr);
    memcpy(ptr, data, count * sizeof(InstanceData));
    mesh.instanceBuffer.m_Resource->Unmap(0, nullptr);
}

void Render::Reset()
{
    m_CommandAlloc->Reset();
    commandList->Reset(m_CommandAlloc, nullptr);
}

void Render::Clear()
{
    uint32_t backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RTVHeap.GetCPUHandle(backBufferIndex);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_DSVHeap.GetCPUHandle(0);
    float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
}

void Render::BeginFrame()
{
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)m_Width, (float)m_Height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)m_Width, (LONG)m_Height };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

}

void Render::BeginGame()
{
    commandList->SetGraphicsRootSignature(m_GameRootSig);
    SetGameRootConstants();
    commandList->SetPipelineState(m_GamePSO);
}

void Render::BeginUI()
{
    commandList->SetGraphicsRootSignature(m_UIRootSig);

    ID3D12DescriptorHeap* heaps[] = { m_SRVHeap.m_Heap };
    commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootDescriptorTable(1, m_SRVHeap.GetGPUHandle(0));

    SetUIRootConstants();
    commandList->SetPipelineState(m_UIPSO);
}

void Render::Loop()
{
    commandList->Close();
    ID3D12CommandList* lists[] = { commandList };
    m_CommandQueue->ExecuteCommandLists(1, lists);
    m_SwapChain->Present(0, 0);
}

void Render::Cleanup()
{
    if (m_DepthStencilBuffer) m_DepthStencilBuffer->Release();
    if (m_GamePSO) m_GamePSO->Release();
    if (m_GameRootSig) m_GameRootSig->Release();
    if (m_UIPSO) m_UIPSO->Release();
    if (m_UIRootSig) m_UIRootSig->Release();
    for (auto& rt : m_RenderTargets) if (rt) rt->Release();
    m_RTVHeap.Destroy();
    m_DSVHeap.Destroy();
    if (m_SwapChain) m_SwapChain->Release();
    if (m_CommandQueue) m_CommandQueue->Release();
    if (m_Device) m_Device->Release();
    if (m_CommandAlloc) m_CommandAlloc->Release();
    if (commandList) commandList->Release();
}