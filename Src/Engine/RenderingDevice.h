#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <array>
#include <cstdint>
#include <chrono>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <wincodec.h>
#include <imgui.h>
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"
#include "ShaderCompiler.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using Microsoft::WRL::ComPtr;

static ID3D12DescriptorHeap* g_ImGuiSrvHeap = nullptr;
static uint32_t g_ImGuiSrvDescriptorSize = 0;
static uint32_t g_ImGuiSrvNextIndex = 0;

static void ImGuiSrvDescriptorAlloc(ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
{
    if (!g_ImGuiSrvHeap || g_ImGuiSrvDescriptorSize == 0)
        return;

    const uint32_t index = g_ImGuiSrvNextIndex++;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu = g_ImGuiSrvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE gpu = g_ImGuiSrvHeap->GetGPUDescriptorHandleForHeapStart();
    cpu.ptr += static_cast<SIZE_T>(index) * g_ImGuiSrvDescriptorSize;
    gpu.ptr += static_cast<UINT64>(index) * g_ImGuiSrvDescriptorSize;
    *out_cpu_desc_handle = cpu;
    *out_gpu_desc_handle = gpu;
}

static void ImGuiSrvDescriptorFree(ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE)
{
}


static std::filesystem::path ResolveProjectRoot()
{
    wchar_t exePath[MAX_PATH] = {};
    DWORD len = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
        return std::filesystem::current_path();

    std::filesystem::path current = std::filesystem::path(exePath).parent_path();
    for (int i = 0; i < 8; ++i)
    {
        if (std::filesystem::exists(current / L"Assets"))
            return current;
        if (!current.has_parent_path())
            break;
        current = current.parent_path();
    }

    return std::filesystem::current_path();
}


struct Adapter
{
	uint32_t id; // Unique identifier for the adapter (could be index or custom ID)
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
    void* m_MappedData = nullptr;
    uint32_t m_Size = 0;
    void Destroy()
    {
        if (m_Resource)
        {
            if (m_MappedData)
            {
                m_Resource->Unmap(0, nullptr);
                m_MappedData = nullptr;
            }
            m_Resource->Release();
            m_Resource = nullptr;
        }
    }
};

// ------------------------------------------------------------------
// Vertex structures
// ------------------------------------------------------------------
struct Vertex
{
    float position[3];
    float uv[2];
    float normal[3];
};

struct InstanceData
{
    DirectX::XMFLOAT4X4 worldMatrix = {};
    DirectX::XMFLOAT4 baseColor;
    DirectX::XMFLOAT4 material;
};

struct GameConstants
{
    DirectX::XMMATRIX viewProj;
    DirectX::XMFLOAT4 cameraPosition;
    DirectX::XMFLOAT4 lightDirection;
};

struct TextureAsset
{
    ID3D12Resource* resource = nullptr;
    std::wstring path;
    std::wstring name;
    uint32_t mipLevels = 1;

    void Destroy()
    {
        if (resource)
        {
            resource->Release();
            resource = nullptr;
        }
    }
};

struct RenderStats
{
    double fps = 0.0;
    double frameTimeMs = 0.0;
    uint32_t objectCount = 0;
    std::array<uint32_t, 8> shapeInstanceCounts{};
};

// ------------------------------------------------------------------
// Mesh (game objects)
// ------------------------------------------------------------------
struct Mesh
{
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    StructuredBuffer instanceBuffer;
    std::string debugName;
    uint32_t maxInstanceCount = 128 * 128;
    uint32_t currentInstanceCount = 0;

    void Draw(ID3D12GraphicsCommandList* cmdList)
    {
        if (!cmdList || !vertexBuffer.m_Resource || !indexBuffer.m_Resource || !instanceBuffer.m_Resource || currentInstanceCount == 0)
            return;

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
    bool BeginGame();     // switches to game PSO and root signature
    void BeginImGuiFrame();
    void RenderImGui();
    void Loop();         // closes command list, executes, presents
    void SetGameCamera(const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT4& cameraPosition, const DirectX::XMFLOAT4& lightDirection);
    void UpdateFrameStats(double deltaSeconds);
    uint32_t GetTextureCount() const { return static_cast<uint32_t>(m_Textures.size()); }
    std::string GetTextureName(uint32_t index) const
    {
        if (index >= m_Textures.size())
            return std::string();

        const std::wstring& wide = m_Textures[index].name;
        if (wide.empty())
            return std::string();

        int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0)
            return std::string();

        std::string result(static_cast<size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, result.data(), size, nullptr, nullptr);
        result.resize(static_cast<size_t>(size - 1));
        return result;
    }
    const RenderStats& GetRenderStats() const { return m_RenderStats; }
    RenderStats& GetRenderStats() { return m_RenderStats; }


    // Mesh creation
    Mesh CreateMesh(Vertex* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount);

    // Dynamic updates (for game instances)
    void UpdateInstanceBuffer(Mesh& mesh, InstanceData* data, uint32_t count);

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
    ID3D12GraphicsCommandList* m_CommandList4 = nullptr;

    // Descriptor heaps
    DescriptorHeap m_RTVHeap;
    DescriptorHeap m_DSVHeap;
    DescriptorHeap m_SRVHeap;
    DescriptorHeap m_GameTextureHeap;

    // Pipelines and root signatures
    ID3D12RootSignature* m_GameRootSig = nullptr;
    ID3D12PipelineState* m_GamePSO = nullptr;

    // Matrices
    GameConstants m_GameConstants{};

    // Dimensions
    uint32_t m_Width = 0, m_Height = 0;
    uint32_t m_FrameCount = 2;

    // Shader compiler
    Core::ShaderCompiler m_ShaderCompiler;
    bool m_ImGuiInitialized = false;
    ImGuiContext* m_ImGuiContext = nullptr;
    std::vector<TextureAsset> m_Textures;
    RenderStats m_RenderStats;
    ID3D12Fence* m_Fence = nullptr;
    HANDLE m_FenceEvent = nullptr;
    uint64_t m_FenceValue = 0;
    bool m_CoInitialized = false;

    // Helpers
    void CreateDepthBuffer();
    void CreateGamePipeline();
    void LoadTextures();
    bool LoadTextureFile(const std::filesystem::path& path, uint32_t descriptorIndex, std::vector<ID3D12Resource*>& uploadResources);
    bool LoadWicPixels(const std::filesystem::path& path, std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height);
    ID3D12Resource* CreateTextureResource(uint32_t width, uint32_t height, const std::vector<uint8_t>& pixels, std::vector<ID3D12Resource*>& uploadResources, uint32_t mipLevels, uint32_t& outMipLevels);
    void CreateTextureSRV(ID3D12Resource* texture, uint32_t descriptorIndex, uint32_t mipLevels);
    void CreateStructuredBufferSRV(ID3D12Resource* buffer, uint32_t descriptorIndex, uint32_t structureStride, uint32_t numElements);
    void FlushCommandQueue();
    void SetGameRootConstants();
    void InitializeImGui(HWND hwnd);
    void ShutdownImGui();
    StructuredBuffer CreateStructuredBuffer(uint32_t size, uint32_t stride);
    bool IsSupportedTextureFile(const std::filesystem::path& path) const;
};

// ------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------

bool Render::Initialize(HWND hwnd, uint32_t width, uint32_t height)
{
    m_Width = width;
    m_Height = height;

    HRESULT coHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    m_CoInitialized = SUCCEEDED(coHr);


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

    m_RTVHeap.Initialize(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_FrameCount + 1);
    for (uint32_t i = 0; i < m_FrameCount; ++i)
    {
        m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[i]));
        m_Device->CreateRenderTargetView(m_RenderTargets[i], nullptr, m_RTVHeap.GetCPUHandle(i));
    }

    m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAlloc));
    m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAlloc, nullptr, IID_PPV_ARGS(&m_CommandList4));
    commandList = m_CommandList4;
    commandList->Close();

    m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
    m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    CreateDepthBuffer();
    LoadTextures();
    m_SRVHeap.Initialize(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024, true);
    CreateGamePipeline();
    InitializeImGui(hwnd);

    float aspect = (float)width / height;
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 1.5f, -6.0f, 1.0f);
    DirectX::XMVECTOR target = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eye, target, up);
    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspect, 0.1f, 100.0f);
    SetGameCamera(view * proj, { 0.0f, 1.5f, -6.0f, 1.0f }, { -0.35f, -1.0f, -0.25f, 0.0f });


    return true;
}

void Render::CreateDepthBuffer()
{
    if (m_DepthStencilBuffer)
    {
        m_DepthStencilBuffer->Release();
        m_DepthStencilBuffer = nullptr;
    }

    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = m_Width;
    depthDesc.Height = m_Height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
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
    auto vertexShaderBlob = m_ShaderCompiler.Compile(L"../../../Assets/Shaders/PBR/VertexShader.hlsl", L"VS", L"vs_6_0");
    auto pixelShaderBlob = m_ShaderCompiler.Compile(L"../../../Assets/Shaders/PBR/PixelShader.hlsl", L"PS", L"ps_6_0");

    if (!vertexShaderBlob || !pixelShaderBlob)
    {
        OutputDebugStringA("Failed to compile PBR pipeline shaders.\n");
        m_GamePSO = nullptr;
        return;
    }

    D3D12_ROOT_PARAMETER rootParams[3];

    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParams[0].Constants.Num32BitValues = 24;
    rootParams[0].Constants.ShaderRegister = 0;
    rootParams[0].Constants.RegisterSpace = 0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParams[1].Descriptor.ShaderRegister = 0;
    rootParams[1].Descriptor.RegisterSpace = 0;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_DESCRIPTOR_RANGE textureRange = {};
    textureRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    textureRange.NumDescriptors = 1024;
    textureRange.BaseShaderRegister = 1;
    textureRange.RegisterSpace = 0;
    textureRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[2].DescriptorTable.pDescriptorRanges = &textureRange;
    rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 3;
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.NumStaticSamplers = 1;
    rootSigDesc.pStaticSamplers = &samplerDesc;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* sigBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT rsHr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob);
    if (FAILED(rsHr) || !sigBlob)
    {
        if (errorBlob)
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        OutputDebugStringA("Failed to serialize game root signature.\n");
        if (errorBlob) errorBlob->Release();
        return;
    }

    HRESULT sigHr = m_Device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_GameRootSig));
    sigBlob->Release();
    if (errorBlob) errorBlob->Release();
    if (FAILED(sigHr))
    {
        OutputDebugStringA("Failed to create game root signature.\n");
        return;
    }

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_GameRootSig;
    psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    HRESULT psoHr = m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_GamePSO));
    if (FAILED(psoHr))
    {
        OutputDebugStringA("Failed to create game pipeline state.\n");
        m_GamePSO = nullptr;
    }
}

void Render::SetGameRootConstants()
{
    commandList->SetGraphicsRoot32BitConstants(0, 24, &m_GameConstants, 0);
}

void Render::SetGameCamera(const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT4& cameraPosition, const DirectX::XMFLOAT4& lightDirection)
{
    m_GameConstants.viewProj = DirectX::XMMatrixTranspose(viewProj);
    m_GameConstants.cameraPosition = cameraPosition;
    m_GameConstants.lightDirection = lightDirection;
}

void Render::UpdateFrameStats(double deltaSeconds)
{
    m_RenderStats.fps = deltaSeconds > 0.0 ? 1.0 / deltaSeconds : 0.0;
    m_RenderStats.frameTimeMs = deltaSeconds * 1000.0;
}

void Render::InitializeImGui(HWND hwnd)
{
    IMGUI_CHECKVERSION();
    m_ImGuiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_ImGuiContext);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    const bool win32Init = ImGui_ImplWin32_Init(hwnd);
    g_ImGuiSrvHeap = m_SRVHeap.m_Heap;
    g_ImGuiSrvDescriptorSize = m_SRVHeap.m_DescriptorSize;
    g_ImGuiSrvNextIndex = 0;

    ImGui_ImplDX12_InitInfo initInfo{};
    initInfo.Device = m_Device;
    initInfo.CommandQueue = m_CommandQueue;
    initInfo.NumFramesInFlight = static_cast<int>(m_FrameCount);
    initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    initInfo.SrvDescriptorHeap = m_SRVHeap.m_Heap;
    initInfo.SrvDescriptorAllocFn = ImGuiSrvDescriptorAlloc;
    initInfo.SrvDescriptorFreeFn = ImGuiSrvDescriptorFree;
    const bool dx12Init = ImGui_ImplDX12_Init(&initInfo);

    if (!win32Init || !dx12Init)
    {
        OutputDebugStringA("ImGui init failed.\n");
        m_ImGuiInitialized = false;
        return;
    }

    m_ImGuiInitialized = true;
}

void Render::ShutdownImGui()
{
    if (!m_ImGuiInitialized)
        return;

    ImGui::SetCurrentContext(m_ImGuiContext);
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    m_ImGuiContext = nullptr;
    m_ImGuiInitialized = false;
}

StructuredBuffer Render::CreateStructuredBuffer(uint32_t size, uint32_t stride)
{
    StructuredBuffer buffer;
    buffer.m_Size = size;
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
    if (SUCCEEDED(m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer.m_Resource))))
    {
        D3D12_RANGE readRange = { 0, 0 };
        if (FAILED(buffer.m_Resource->Map(0, &readRange, &buffer.m_MappedData)))
        {
            buffer.Destroy();
        }
        else
        {
            memset(buffer.m_MappedData, 0, size);
        }
    }
    return buffer;
}

bool Render::IsSupportedTextureFile(const std::filesystem::path& path) const
{
    const std::wstring ext = path.extension().wstring();
    return _wcsicmp(ext.c_str(), L".png") == 0
        || _wcsicmp(ext.c_str(), L".jpg") == 0
        || _wcsicmp(ext.c_str(), L".jpeg") == 0
        || _wcsicmp(ext.c_str(), L".bmp") == 0
        || _wcsicmp(ext.c_str(), L".tif") == 0
        || _wcsicmp(ext.c_str(), L".tiff") == 0;
}

bool Render::LoadWicPixels(const std::filesystem::path& path, std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height)
{
    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr))
        return false;

    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr))
        return false;

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr))
        return false;

    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr))
        return false;

    hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr))
        return false;

    IWICBitmapSource* source = converter.Get();
    UINT w = 0;
    UINT h = 0;
    hr = source->GetSize(&w, &h);
    if (FAILED(hr) || w == 0 || h == 0)
        return false;

    width = w;
    height = h;
    pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4u);
    const UINT stride = width * 4u;
    hr = source->CopyPixels(nullptr, stride, static_cast<UINT>(pixels.size()), pixels.data());
    return SUCCEEDED(hr);
}

ID3D12Resource* Render::CreateTextureResource(uint32_t width, uint32_t height, const std::vector<uint8_t>& pixels, std::vector<ID3D12Resource*>& uploadResources, uint32_t mipLevels, uint32_t& outMipLevels)
{
    auto calcMipLevels = [](uint32_t w, uint32_t h)
    {
        uint32_t levels = 1;
        while (w > 1 || h > 1)
        {
            w = std::max(1u, w >> 1);
            h = std::max(1u, h >> 1);
            ++levels;
        }
        return levels;
    };

    auto downsampleRgba = [](const std::vector<uint8_t>& srcPixels, uint32_t srcWidth, uint32_t srcHeight)
    {
        const uint32_t dstWidth = std::max(1u, srcWidth >> 1);
        const uint32_t dstHeight = std::max(1u, srcHeight >> 1);
        std::vector<uint8_t> dstPixels(static_cast<size_t>(dstWidth) * static_cast<size_t>(dstHeight) * 4u);

        for (uint32_t y = 0; y < dstHeight; ++y)
        {
            for (uint32_t x = 0; x < dstWidth; ++x)
            {
                uint32_t accum[4] = {};
                uint32_t samples = 0;
                for (uint32_t oy = 0; oy < 2; ++oy)
                {
                    for (uint32_t ox = 0; ox < 2; ++ox)
                    {
                        const uint32_t sx = std::min(srcWidth - 1, x * 2 + ox);
                        const uint32_t sy = std::min(srcHeight - 1, y * 2 + oy);
                        const size_t srcIndex = (static_cast<size_t>(sy) * srcWidth + sx) * 4u;
                        accum[0] += srcPixels[srcIndex + 0];
                        accum[1] += srcPixels[srcIndex + 1];
                        accum[2] += srcPixels[srcIndex + 2];
                        accum[3] += srcPixels[srcIndex + 3];
                        ++samples;
                    }
                }

                const size_t dstIndex = (static_cast<size_t>(y) * dstWidth + x) * 4u;
                dstPixels[dstIndex + 0] = static_cast<uint8_t>(accum[0] / samples);
                dstPixels[dstIndex + 1] = static_cast<uint8_t>(accum[1] / samples);
                dstPixels[dstIndex + 2] = static_cast<uint8_t>(accum[2] / samples);
                dstPixels[dstIndex + 3] = static_cast<uint8_t>(accum[3] / samples);
            }
        }

        return dstPixels;
    };

    outMipLevels = mipLevels > 0 ? mipLevels : calcMipLevels(width, height);
    if (outMipLevels == 0)
        outMipLevels = 1;

    std::vector<std::vector<uint8_t>> mipPixels;
    std::vector<UINT> mipWidths(outMipLevels);
    std::vector<UINT> mipHeights(outMipLevels);
    mipPixels.reserve(outMipLevels);

    mipPixels.push_back(pixels);
    mipWidths[0] = width;
    mipHeights[0] = height;

    for (uint32_t mip = 1; mip < outMipLevels; ++mip)
    {
        const uint32_t prevWidth = mipWidths[mip - 1];
        const uint32_t prevHeight = mipHeights[mip - 1];
        mipPixels.push_back(downsampleRgba(mipPixels.back(), prevWidth, prevHeight));
        mipWidths[mip] = std::max(1u, prevWidth >> 1);
        mipHeights[mip] = std::max(1u, prevHeight >> 1);
    }

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = static_cast<UINT16>(outMipLevels);
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    ID3D12Resource* texture = nullptr;
    if (FAILED(m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture))))
    {
        return nullptr;
    }

    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints(outMipLevels);
    std::vector<UINT> numRows(outMipLevels);
    std::vector<UINT64> rowSizes(outMipLevels);
    UINT64 uploadSize = 0;
    m_Device->GetCopyableFootprints(&texDesc, 0, outMipLevels, 0, footprints.data(), numRows.data(), rowSizes.data(), &uploadSize);

    D3D12_RESOURCE_DESC uploadDesc = {};
    uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadDesc.Width = uploadSize;
    uploadDesc.Height = 1;
    uploadDesc.DepthOrArraySize = 1;
    uploadDesc.MipLevels = 1;
    uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
    uploadDesc.SampleDesc.Count = 1;
    uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    ID3D12Resource* upload = nullptr;
    if (FAILED(m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload))))
    {
        texture->Release();
        return nullptr;
    }

    void* mapped = nullptr;
    if (FAILED(upload->Map(0, nullptr, &mapped)) || !mapped)
    {
        upload->Release();
        texture->Release();
        return nullptr;
    }

    uint8_t* dst = static_cast<uint8_t*>(mapped);
    for (uint32_t mip = 0; mip < outMipLevels; ++mip)
    {
        const auto& mipData = mipPixels[mip];
        const UINT mipWidth = mipWidths[mip];
        const UINT mipHeight = mipHeights[mip];
        const UINT rowPitch = footprints[mip].Footprint.RowPitch;
        const size_t srcPitch = static_cast<size_t>(mipWidth) * 4u;
        for (UINT y = 0; y < mipHeight; ++y)
        {
            memcpy(dst + footprints[mip].Offset + static_cast<size_t>(y) * rowPitch,
                   mipData.data() + static_cast<size_t>(y) * srcPitch,
                   srcPitch);
        }
    }
    upload->Unmap(0, nullptr);

    for (uint32_t mip = 0; mip < outMipLevels; ++mip)
    {
        D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
        dstLoc.pResource = texture;
        dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLoc.SubresourceIndex = mip;

        D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
        srcLoc.pResource = upload;
        srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcLoc.PlacedFootprint = footprints[mip];

        commandList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
    }

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = texture;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);

    uploadResources.push_back(upload);
    return texture;
}

void Render::CreateTextureSRV(ID3D12Resource* texture, uint32_t descriptorIndex, uint32_t mipLevels)
{
    if (!texture || !m_GameTextureHeap.m_Heap)
        return;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = mipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    m_Device->CreateShaderResourceView(texture, &srvDesc, m_GameTextureHeap.GetCPUHandle(descriptorIndex));
}

void Render::CreateStructuredBufferSRV(ID3D12Resource* buffer, uint32_t descriptorIndex, uint32_t structureStride, uint32_t numElements)
{
    if (!buffer || !m_GameTextureHeap.m_Heap || structureStride == 0 || numElements == 0)
        return;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = numElements;
    srvDesc.Buffer.StructureByteStride = structureStride;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    m_Device->CreateShaderResourceView(buffer, &srvDesc, m_GameTextureHeap.GetCPUHandle(descriptorIndex));
}

bool Render::LoadTextureFile(const std::filesystem::path& path, uint32_t descriptorIndex, std::vector<ID3D12Resource*>& uploadResources)
{
    std::vector<uint8_t> pixels;
    uint32_t width = 0;
    uint32_t height = 0;
    if (!LoadWicPixels(path, pixels, width, height))
    {
        std::wstring message = L"Failed to load texture: ";
        message += path.wstring();
        message += L"\n";
        OutputDebugStringW(message.c_str());
        return false;
    }

    uint32_t mipLevels = 1;
    ID3D12Resource* texture = CreateTextureResource(width, height, pixels, uploadResources, 0, mipLevels);
    if (!texture)
        return false;

    CreateTextureSRV(texture, descriptorIndex, mipLevels);

    TextureAsset asset;
    asset.resource = texture;
    asset.path = path.wstring();
    asset.name = path.filename().wstring();
    asset.mipLevels = mipLevels;
    m_Textures.push_back(asset);
    return true;
}

void Render::FlushCommandQueue()
{
    if (!m_Fence || !m_FenceEvent)
        return;

    ++m_FenceValue;
    m_CommandQueue->Signal(m_Fence, m_FenceValue);
    if (m_Fence->GetCompletedValue() < m_FenceValue)
    {
        m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
        WaitForSingleObject(m_FenceEvent, INFINITE);
    }
}

void Render::LoadTextures()
{
    m_Textures.clear();

    std::filesystem::path root = ResolveProjectRoot() / L"Assets/Textures";
    std::vector<std::filesystem::path> textureFiles;
    if (std::filesystem::exists(root))
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
        {
            if (!entry.is_regular_file())
                continue;
            if (!IsSupportedTextureFile(entry.path()))
                continue;
            textureFiles.push_back(entry.path());
        }
        std::sort(textureFiles.begin(), textureFiles.end());
    }

    const uint32_t textureCount = static_cast<uint32_t>(textureFiles.size() + 1);
    m_GameTextureHeap.Initialize(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, std::max(1024u, textureCount + 1u), true);

    m_CommandAlloc->Reset();
    commandList->Reset(m_CommandAlloc, nullptr);

    std::vector<ID3D12Resource*> uploadResources;
    uploadResources.reserve(textureFiles.size() + 1);

    uint32_t descriptorIndex = 0;

    std::vector<uint8_t> white = { 255, 255, 255, 255 };
    uint32_t whiteMipLevels = 1;
    ID3D12Resource* whiteTexture = CreateTextureResource(1, 1, white, uploadResources, 0, whiteMipLevels);
    if (whiteTexture)
    {
        CreateTextureSRV(whiteTexture, descriptorIndex, whiteMipLevels);
        TextureAsset whiteAsset;
        whiteAsset.resource = whiteTexture;
        whiteAsset.name = L"white_fallback";
        whiteAsset.mipLevels = whiteMipLevels;
        m_Textures.push_back(whiteAsset);
        ++descriptorIndex;
    }

    for (const auto& texturePath : textureFiles)
    {
        if (LoadTextureFile(texturePath, descriptorIndex, uploadResources))
            ++descriptorIndex;
    }

    commandList->Close();
    ID3D12CommandList* lists[] = { commandList };
    m_CommandQueue->ExecuteCommandLists(1, lists);
    FlushCommandQueue();

    for (ID3D12Resource* upload : uploadResources)
    {
        if (upload)
            upload->Release();
    }

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

void Render::UpdateInstanceBuffer(Mesh& mesh, InstanceData* data, uint32_t count)
{
    if (!mesh.instanceBuffer.m_Resource || !mesh.instanceBuffer.m_MappedData || !data || count == 0)
    {
        mesh.currentInstanceCount = 0;
        return;
    }

    if (count > mesh.maxInstanceCount) count = mesh.maxInstanceCount;
    if (count == 0)
    {
        mesh.currentInstanceCount = 0;
        return;
    }

    mesh.currentInstanceCount = count;
    memcpy(mesh.instanceBuffer.m_MappedData, data, count * sizeof(InstanceData));
}

void Render::Reset()
{
    if (m_Fence && m_FenceEvent && m_FenceValue > 0 && m_Fence->GetCompletedValue() < m_FenceValue)
    {
        m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
        WaitForSingleObject(m_FenceEvent, INFINITE);
    }

    m_CommandAlloc->Reset();
    commandList->Reset(m_CommandAlloc, nullptr);
}

void Render::Clear()
{
    const uint32_t backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RTVHeap.GetCPUHandle(backBufferIndex);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_DSVHeap.GetCPUHandle(0);
    const float clearColor[] = { 0.65f, 0.80f, 0.95f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Render::BeginFrame()
{
    const uint32_t backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_RenderTargets[backBufferIndex];
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RTVHeap.GetCPUHandle(backBufferIndex);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_DSVHeap.GetCPUHandle(0);
    commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

    D3D12_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(m_Width);
    viewport.Height = static_cast<float>(m_Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    D3D12_RECT scissor = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);
}

bool Render::BeginGame()
{
    if (!m_GamePSO || !m_GameRootSig)
        return false;

    ID3D12DescriptorHeap* heaps[] = { m_GameTextureHeap.m_Heap };
    commandList->SetDescriptorHeaps(1, heaps);
    commandList->SetGraphicsRootSignature(m_GameRootSig);
    SetGameRootConstants();
    commandList->SetGraphicsRootDescriptorTable(2, m_GameTextureHeap.GetGPUHandle(0));
    commandList->SetPipelineState(m_GamePSO);
    return true;
}

void Render::BeginImGuiFrame()
{
    if (!m_ImGuiInitialized)
        return;

    ImGui::SetCurrentContext(m_ImGuiContext);
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Render::RenderImGui()
{
    if (!m_ImGuiInitialized)
        return;

    ImGui::SetCurrentContext(m_ImGuiContext);
    ImGui::Render();

    const uint32_t backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        m_RTVHeap.GetCPUHandle(backBufferIndex);
    commandList->OMSetRenderTargets(
        1,
        &rtvHandle,
        false,
        nullptr
    );

    ID3D12DescriptorHeap* heaps[] = { m_SRVHeap.m_Heap };
    commandList->SetDescriptorHeaps(1, heaps);

    ImGui_ImplDX12_RenderDrawData(
        ImGui::GetDrawData(),
        commandList
    );
}


void Render::Loop()
{
    const uint32_t backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_RenderTargets[backBufferIndex];
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);

    HRESULT closeHr = commandList->Close();
    if (FAILED(closeHr))
        return;

    ID3D12CommandList* lists[] = { commandList };
    m_CommandQueue->ExecuteCommandLists(1, lists);
    m_SwapChain->Present(1, 0);

    if (m_Fence && m_FenceEvent)
    {
        ++m_FenceValue;
        m_CommandQueue->Signal(m_Fence, m_FenceValue);
    }
}

void Render::Cleanup()
{
    ShutdownImGui();
    for (auto& texture : m_Textures)
        texture.Destroy();
    m_Textures.clear();
    m_GameTextureHeap.Destroy();
    if (m_DepthStencilBuffer) m_DepthStencilBuffer->Release();
    if (m_GamePSO) m_GamePSO->Release();
    if (m_GameRootSig) m_GameRootSig->Release();
    for (auto& rt : m_RenderTargets) if (rt) rt->Release();
    m_RTVHeap.Destroy();
    m_DSVHeap.Destroy();
    m_SRVHeap.Destroy();
    if (m_CommandList4) m_CommandList4->Release();
    if (m_SwapChain) m_SwapChain->Release();
    if (m_CommandQueue) m_CommandQueue->Release();
    if (m_Device) m_Device->Release();
    if (m_CommandAlloc) m_CommandAlloc->Release();
    if (m_Fence) m_Fence->Release();
    if (m_FenceEvent) CloseHandle(m_FenceEvent);
    if (commandList) commandList->Release();
    if (m_CoInitialized) CoUninitialize();
}

