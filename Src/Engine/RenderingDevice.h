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

struct RaytracingConstants
{
    DirectX::XMMATRIX invViewProj;
    DirectX::XMFLOAT4 cameraPosition;
    DirectX::XMFLOAT4 lightDirection;
    DirectX::XMFLOAT4 viewportSize;
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

struct BLASStats
{
    std::string name;
    uint32_t triangleCount = 0;
    uint32_t geometryCount = 0;
    uint64_t resultSizeBytes = 0;
    uint64_t compactedSizeBytes = 0;
    uint64_t scratchSizeBytes = 0;
    uint64_t updateScratchSizeBytes = 0;
    float buildTimeMs = 0.0f;
    float updateTimeMs = 0.0f;
    bool allowCompaction = false;
    bool allowUpdate = false;
    bool preferFastTrace = false;
    bool preferFastBuild = false;
};

struct RenderStats
{
    double fps = 0.0;
    double frameTimeMs = 0.0;
    float blasBuildTotalMs = 0.0f;
    float tlasBuildMs = 0.0f;
    float rayDispatchMs = 0.0f;
    float presentMs = 0.0f;
    float copyMs = 0.0f;
    float uploadMs = 0.0f;
    uint32_t objectCount = 0;
    uint32_t blasCount = 0;
    std::array<uint32_t, 8> shapeInstanceCounts{};
};

struct DxrInstanceData
{
    InstanceData instance;
    uint32_t shapeType = 0;
    DirectX::XMFLOAT3 padding = {};
};

struct RaytracingObject
{
    ShapeType shapeType = ShapeType::Null;
    InstanceData instance{};
};

// ------------------------------------------------------------------
// Mesh (game objects)
// ------------------------------------------------------------------
struct Mesh
{
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    StructuredBuffer instanceBuffer;
    ID3D12Resource* bottomLevelAS = nullptr;
    std::string debugName;
    BLASStats blasStats{};
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
        if (bottomLevelAS)
        {
            bottomLevelAS->Release();
            bottomLevelAS = nullptr;
        }
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
    bool RenderRaytracedScene(const std::vector<RaytracingObject>& objects, Mesh& triangle, Mesh& cuad, Mesh& pentagon, Mesh& hexagon, Mesh& circle, Mesh& cube, Mesh& sphere, Mesh& plane);
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
    const std::vector<BLASStats>& GetBLASStats() const { return m_BLASStats; }
    const RenderStats& GetRenderStats() const { return m_RenderStats; }


    // Mesh creation
    Mesh CreateMesh(Vertex* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount);

    // Dynamic updates (for game instances)
    void UpdateInstanceBuffer(Mesh& mesh, InstanceData* data, uint32_t count);

    bool RenderDXRFrame(
        const std::vector<RaytracingObject>& objects,
        Mesh& triangle,
        Mesh& cuad,
        Mesh& pentagon,
        Mesh& hexagon,
        Mesh& circle,
        Mesh& cube,
        Mesh& sphere,
        Mesh& plane,
        bool drawImGui = true
    );

    void PresentDXRFrame();

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
    ID3D12Resource* m_MSAAColorBuffer = nullptr;
    ID3D12Device5* m_Device5 = nullptr;
    ID3D12GraphicsCommandList4* m_CommandList4 = nullptr;

    // Descriptor heaps
    DescriptorHeap m_RTVHeap;
    DescriptorHeap m_DSVHeap;
    DescriptorHeap m_SRVHeap;
    DescriptorHeap m_GameTextureHeap;

    // Pipelines and root signatures
    ID3D12RootSignature* m_GameRootSig = nullptr;
    ID3D12PipelineState* m_GamePSO = nullptr;
    ID3D12RootSignature* m_DXRRootSig = nullptr;
    ID3D12StateObject* m_DXRStateObject = nullptr;
    ID3D12StateObjectProperties* m_DXRStateObjectProperties = nullptr;
    ID3D12Resource* m_DXRShaderTable = nullptr;

    // Matrices
    GameConstants m_GameConstants{};
    RaytracingConstants m_DXRConstants{};

    // Dimensions
    uint32_t m_Width = 0, m_Height = 0;
    uint32_t m_FrameCount = 2;
    uint32_t m_MSAASampleCount = 4;
    uint32_t m_MSAAQuality = 0;

    // Shader compiler
    Core::ShaderCompiler m_ShaderCompiler;
    bool m_ImGuiInitialized = false;
    ImGuiContext* m_ImGuiContext = nullptr;
    std::vector<TextureAsset> m_Textures;
    std::vector<BLASStats> m_BLASStats;
    RenderStats m_RenderStats;
    StructuredBuffer m_DXRInstanceBuffer;
    StructuredBuffer m_DXRInstanceDescBuffer;
    StructuredBuffer m_DXRConstantBuffer;
    ID3D12Resource* m_DXROutput = nullptr;
    ID3D12Resource* m_DXRTLAS = nullptr;
    ID3D12Resource* m_DXRTLASScratch = nullptr;
    uint32_t m_DXROutputDescriptorIndex = 0;
    uint32_t m_DXRInstanceDescriptorIndex = 1023;
    uint32_t m_DXRSceneObjectCount = 0;
    ID3D12Fence* m_Fence = nullptr;
    HANDLE m_FenceEvent = nullptr;
    uint64_t m_FenceValue = 0;
    bool m_CoInitialized = false;
    bool m_DXRReady = false;

    // Helpers
    void CreateDepthBuffer();
    void CreateMSAAColorBuffer();
    void CreateGamePipeline();
    void CreateDXRRootSignature();
    void CreateDXRPipeline();
    void CreateDXROutput();
    void LoadTextures();
    bool LoadTextureFile(const std::filesystem::path& path, uint32_t descriptorIndex, std::vector<ID3D12Resource*>& uploadResources);
    bool LoadWicPixels(const std::filesystem::path& path, std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height);
    ID3D12Resource* CreateTextureResource(uint32_t width, uint32_t height, const std::vector<uint8_t>& pixels, std::vector<ID3D12Resource*>& uploadResources, uint32_t mipLevels, uint32_t& outMipLevels);
    void CreateTextureSRV(ID3D12Resource* texture, uint32_t descriptorIndex, uint32_t mipLevels);
    void CreateStructuredBufferSRV(ID3D12Resource* buffer, uint32_t descriptorIndex, uint32_t structureStride, uint32_t numElements);
    void BuildBottomLevelAS(Mesh& mesh);
    void UpdateDXRConstants();
    void FlushCommandQueue();
    void ResolveMSAA();
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
    if (FAILED(m_Device->QueryInterface(IID_PPV_ARGS(&m_Device5))) || !m_Device5)
    {
        OutputDebugStringA("DXR unsupported on this device.\n");
        return false;
    }
    m_MSAASampleCount = 1;
    m_MSAAQuality = 0;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));

    m_MSAAQuality = 0;
    if (m_MSAASampleCount > 1)
    {
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaData = {};
        msaaData.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        msaaData.SampleCount = m_MSAASampleCount;
        msaaData.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        msaaData.NumQualityLevels = 0;
        if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaData, sizeof(msaaData))) || msaaData.NumQualityLevels == 0)
        {
            m_MSAASampleCount = 1;
            m_MSAAQuality = 0;
        }
        else
        {
            m_MSAAQuality = msaaData.NumQualityLevels;
        }
    }

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

    CreateMSAAColorBuffer();
    m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAlloc));
    m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAlloc, nullptr, IID_PPV_ARGS(&m_CommandList4));
    commandList = m_CommandList4;
    commandList->Close();

    m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
    m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    CreateDepthBuffer();
    LoadTextures();
    m_SRVHeap.Initialize(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024, true);
    CreateDXROutput();
    CreateDXRRootSignature();
    CreateDXRPipeline();
    UpdateDXRConstants();
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
    depthDesc.SampleDesc.Count = m_MSAASampleCount;
    depthDesc.SampleDesc.Quality = m_MSAASampleCount > 1 ? (m_MSAAQuality - 1) : 0;
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
    dsvDesc.ViewDimension = m_MSAASampleCount > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
    m_Device->CreateDepthStencilView(m_DepthStencilBuffer, &dsvDesc, m_DSVHeap.GetCPUHandle(0));
}

void Render::CreateMSAAColorBuffer()
{

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
    psoDesc.SampleDesc.Count = m_MSAASampleCount;
    psoDesc.SampleDesc.Quality = m_MSAASampleCount > 1 ? (m_MSAAQuality - 1) : 0;

    HRESULT psoHr = m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_GamePSO));
    if (FAILED(psoHr))
    {
        OutputDebugStringA("Failed to create game pipeline state.\n");
        m_GamePSO = nullptr;
        if (m_MSAASampleCount > 1)
        {
            m_MSAASampleCount = 1;
            m_MSAAQuality = 0;
            if (m_MSAAColorBuffer)
            {
                m_MSAAColorBuffer->Release();
                m_MSAAColorBuffer = nullptr;
            }
            CreateDepthBuffer();
            CreateGamePipeline();
        }
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

    m_DXRConstants.invViewProj = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, viewProj));
    m_DXRConstants.cameraPosition = cameraPosition;
    m_DXRConstants.lightDirection = lightDirection;
    m_DXRConstants.viewportSize = { static_cast<float>(m_Width), static_cast<float>(m_Height), 0.0f, 0.0f };
    UpdateDXRConstants();
}

void Render::UpdateFrameStats(double deltaSeconds)
{
    m_RenderStats.fps = deltaSeconds > 0.0 ? 1.0 / deltaSeconds : 0.0;
    m_RenderStats.frameTimeMs = deltaSeconds * 1000.0;
}

void Render::UpdateDXRConstants()
{
    const uint32_t alignedSize = (sizeof(RaytracingConstants) + 255u) & ~255u;
    if (!m_DXRConstantBuffer.m_Resource || m_DXRConstantBuffer.m_Size < alignedSize)
    {
        m_DXRConstantBuffer.Destroy();
        m_DXRConstantBuffer = CreateStructuredBuffer(alignedSize, sizeof(RaytracingConstants));
    }

    if (m_DXRConstantBuffer.m_MappedData)
        memcpy(m_DXRConstantBuffer.m_MappedData, &m_DXRConstants, sizeof(RaytracingConstants));
}

void Render::CreateDXROutput()
{
    if (m_DXROutput)
    {
        m_DXROutput->Release();
        m_DXROutput = nullptr;
    }

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = m_Width;
    desc.Height = m_Height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_DXROutput));

    m_DXROutputDescriptorIndex = static_cast<uint32_t>(m_Textures.size());
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Texture2D.PlaneSlice = 0;
    m_Device->CreateUnorderedAccessView(m_DXROutput, nullptr, &uavDesc, m_GameTextureHeap.GetCPUHandle(m_DXROutputDescriptorIndex));
}

void Render::CreateDXRRootSignature()
{
    if (m_DXRRootSig)
    {
        m_DXRRootSig->Release();
        m_DXRRootSig = nullptr;
    }

    D3D12_ROOT_PARAMETER params[5] = {};

    params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    params[0].Descriptor.ShaderRegister = 0;
    params[0].Descriptor.RegisterSpace = 0;
    params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_DESCRIPTOR_RANGE instanceRange = {};
    instanceRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    instanceRange.NumDescriptors = 1;
    instanceRange.BaseShaderRegister = 0;
    instanceRange.RegisterSpace = 0;
    instanceRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    params[1].DescriptorTable.NumDescriptorRanges = 1;
    params[1].DescriptorTable.pDescriptorRanges = &instanceRange;
    params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    params[2].Descriptor.ShaderRegister = 2;
    params[2].Descriptor.RegisterSpace = 0;
    params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_DESCRIPTOR_RANGE outputRange = {};
    outputRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    outputRange.NumDescriptors = 1;
    outputRange.BaseShaderRegister = 0;
    outputRange.RegisterSpace = 0;
    outputRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    params[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    params[3].DescriptorTable.NumDescriptorRanges = 1;
    params[3].DescriptorTable.pDescriptorRanges = &outputRange;
    params[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_DESCRIPTOR_RANGE textureRange = {};
    textureRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    textureRange.NumDescriptors = 1024;
    textureRange.BaseShaderRegister = 100;
    textureRange.RegisterSpace = 0;
    textureRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    params[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    params[4].DescriptorTable.NumDescriptorRanges = 1;
    params[4].DescriptorTable.pDescriptorRanges = &textureRange;
    params[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
    rsDesc.NumParameters = _countof(params);
    rsDesc.pParameters = params;
    rsDesc.NumStaticSamplers = 1;
    rsDesc.pStaticSamplers = &samplerDesc;
    rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    ID3DBlob* sigBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    if (FAILED(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob)) || !sigBlob)
    {
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        if (errorBlob) errorBlob->Release();
        OutputDebugStringA("Failed to serialize DXR root signature.\n");
        return;
    }

    if (FAILED(m_Device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_DXRRootSig))))
    {
        OutputDebugStringA("Failed to create DXR root signature.\n");
    }
    else
    {
    }

    sigBlob->Release();
    if (errorBlob) errorBlob->Release();
}

void Render::CreateDXRPipeline()
{
    if (m_DXRStateObject)
    {
        m_DXRStateObject->Release();
        m_DXRStateObject = nullptr;
    }
    if (m_DXRStateObjectProperties)
    {
        m_DXRStateObjectProperties->Release();
        m_DXRStateObjectProperties = nullptr;
    }

    const std::filesystem::path assetRoot = ResolveProjectRoot() / L"Assets";
    auto rayGenBlob = m_ShaderCompiler.Compile((assetRoot / L"Shaders/DXR/RayGen.hlsl").wstring(), L"RayGen", L"lib_6_3");
    auto missBlob = m_ShaderCompiler.Compile((assetRoot / L"Shaders/DXR/Miss.hlsl").wstring(), L"Miss", L"lib_6_3");
    auto hitBlob = m_ShaderCompiler.Compile((assetRoot / L"Shaders/DXR/ClosestHit.hlsl").wstring(), L"ClosestHit", L"lib_6_3");
    if (!rayGenBlob || !missBlob || !hitBlob)
    {
        OutputDebugStringA("Failed to compile DXR shaders.\n");
        return;
    }

    D3D12_EXPORT_DESC rayGenExport = { L"RayGen", nullptr, D3D12_EXPORT_FLAG_NONE };
    D3D12_EXPORT_DESC missExport = { L"Miss", nullptr, D3D12_EXPORT_FLAG_NONE };
    D3D12_EXPORT_DESC hitExport = { L"ClosestHit", nullptr, D3D12_EXPORT_FLAG_NONE };

    D3D12_DXIL_LIBRARY_DESC rayGenLib = {};
    rayGenLib.DXILLibrary = { rayGenBlob->GetBufferPointer(), rayGenBlob->GetBufferSize() };
    rayGenLib.NumExports = 1;
    rayGenLib.pExports = &rayGenExport;

    D3D12_DXIL_LIBRARY_DESC missLib = {};
    missLib.DXILLibrary = { missBlob->GetBufferPointer(), missBlob->GetBufferSize() };
    missLib.NumExports = 1;
    missLib.pExports = &missExport;

    D3D12_DXIL_LIBRARY_DESC hitLib = {};
    hitLib.DXILLibrary = { hitBlob->GetBufferPointer(), hitBlob->GetBufferSize() };
    hitLib.NumExports = 1;
    hitLib.pExports = &hitExport;

    D3D12_STATE_SUBOBJECT subobjects[9] = {};

    subobjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    subobjects[0].pDesc = &rayGenLib;
    subobjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    subobjects[1].pDesc = &missLib;
    subobjects[2].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    subobjects[2].pDesc = &hitLib;

    D3D12_HIT_GROUP_DESC hitGroup = {};
    hitGroup.ClosestHitShaderImport = L"ClosestHit";
    hitGroup.HitGroupExport = L"HitGroup";
    hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
    subobjects[3].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    subobjects[3].pDesc = &hitGroup;

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    shaderConfig.MaxPayloadSizeInBytes = 16;
    shaderConfig.MaxAttributeSizeInBytes = 8;
    subobjects[4].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    subobjects[4].pDesc = &shaderConfig;

    LPCWSTR shaderConfigExports[] = { L"RayGen", L"Miss", L"ClosestHit" };
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderConfigAssociation = {};
    shaderConfigAssociation.pSubobjectToAssociate = &subobjects[4];
    shaderConfigAssociation.NumExports = _countof(shaderConfigExports);
    shaderConfigAssociation.pExports = shaderConfigExports;
    subobjects[5].Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    subobjects[5].pDesc = &shaderConfigAssociation;

    D3D12_GLOBAL_ROOT_SIGNATURE globalRoot = {};
    globalRoot.pGlobalRootSignature = m_DXRRootSig;
    subobjects[6].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    subobjects[6].pDesc = &globalRoot;

    LPCWSTR globalRootExports[] = { L"RayGen", L"Miss", L"ClosestHit" };
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION globalRootAssociation = {};
    globalRootAssociation.pSubobjectToAssociate = &subobjects[6];
    globalRootAssociation.NumExports = _countof(globalRootExports);
    globalRootAssociation.pExports = globalRootExports;
    subobjects[7].Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    subobjects[7].pDesc = &globalRootAssociation;

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = 1;
    subobjects[8].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    subobjects[8].pDesc = &pipelineConfig;

    D3D12_STATE_OBJECT_DESC stateDesc = {};
    stateDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    stateDesc.NumSubobjects = _countof(subobjects);
    stateDesc.pSubobjects = subobjects;

    if (FAILED(m_Device5->CreateStateObject(&stateDesc, IID_PPV_ARGS(&m_DXRStateObject))))
    {
        OutputDebugStringA("Failed to create DXR state object.\n");

        return;
    }

    if (FAILED(m_DXRStateObject->QueryInterface(IID_PPV_ARGS(&m_DXRStateObjectProperties))))
    {
        OutputDebugStringA("Failed to query DXR state object properties.\n");
        return;
    }

    if (m_DXRShaderTable)
    {
        m_DXRShaderTable->Release();
        m_DXRShaderTable = nullptr;
    }

    const uint64_t shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    const uint64_t recordSize = (shaderIdSize + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1) & ~(uint64_t)(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1);
    const uint64_t sbtSize = recordSize * 3ull;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC sbtDesc = {};
    sbtDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    sbtDesc.Width = sbtSize;
    sbtDesc.Height = 1;
    sbtDesc.DepthOrArraySize = 1;
    sbtDesc.MipLevels = 1;
    sbtDesc.Format = DXGI_FORMAT_UNKNOWN;
    sbtDesc.SampleDesc.Count = 1;
    sbtDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    if (SUCCEEDED(m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &sbtDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_DXRShaderTable))))
    {
        void* mapped = nullptr;
        if (SUCCEEDED(m_DXRShaderTable->Map(0, nullptr, &mapped)) && mapped)
        {
            uint8_t* dst = static_cast<uint8_t*>(mapped);
            memcpy(dst + recordSize * 0, m_DXRStateObjectProperties->GetShaderIdentifier(L"RayGen"), shaderIdSize);
            memcpy(dst + recordSize * 1, m_DXRStateObjectProperties->GetShaderIdentifier(L"Miss"), shaderIdSize);
            memcpy(dst + recordSize * 2, m_DXRStateObjectProperties->GetShaderIdentifier(L"HitGroup"), shaderIdSize);
            m_DXRShaderTable->Unmap(0, nullptr);
        }
    }

    m_DXRReady = true;
}

void Render::BuildBottomLevelAS(Mesh& mesh)
{
    if (!m_Device5 || !m_CommandList4 || !mesh.vertexBuffer.m_Resource || !mesh.indexBuffer.m_Resource)
        return;

    const auto startTime = std::chrono::high_resolution_clock::now();

    if (mesh.bottomLevelAS)
    {
        mesh.bottomLevelAS->Release();
        mesh.bottomLevelAS = nullptr;
    }

    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    geometryDesc.Triangles.VertexBuffer.StartAddress = mesh.vertexBuffer.m_Resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
    geometryDesc.Triangles.VertexCount = mesh.vertexBuffer.m_View.SizeInBytes / sizeof(Vertex);
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.IndexBuffer = mesh.indexBuffer.m_Resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.IndexCount = mesh.indexBuffer.m_IndexCount;
    geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    geometryDesc.Triangles.Transform3x4 = 0;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs = 1;
    inputs.pGeometryDescs = &geometryDesc;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
    m_Device5->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC resultDesc = {};
    resultDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resultDesc.Width = info.ResultDataMaxSizeInBytes;
    resultDesc.Height = 1;
    resultDesc.DepthOrArraySize = 1;
    resultDesc.MipLevels = 1;
    resultDesc.Format = DXGI_FORMAT_UNKNOWN;
    resultDesc.SampleDesc.Count = 1;
    resultDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resultDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    if (FAILED(m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resultDesc,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&mesh.bottomLevelAS))))
        return;

    D3D12_RESOURCE_DESC scratchDesc = resultDesc;
    scratchDesc.Width = info.ScratchDataSizeInBytes;
    ID3D12Resource* scratch = nullptr;
    if (FAILED(m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &scratchDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&scratch))))
    {
        mesh.bottomLevelAS->Release();
        mesh.bottomLevelAS = nullptr;
        return;
    }

    m_CommandAlloc->Reset();
    m_CommandList4->Reset(m_CommandAlloc, nullptr);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs = inputs;
    buildDesc.DestAccelerationStructureData = mesh.bottomLevelAS->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
    m_CommandList4->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource = mesh.bottomLevelAS;
    m_CommandList4->ResourceBarrier(1, &barrier);

    m_CommandList4->Close();
    ID3D12CommandList* lists[] = { m_CommandList4 };
    m_CommandQueue->ExecuteCommandLists(1, lists);
    FlushCommandQueue();

    const auto endTime = std::chrono::high_resolution_clock::now();
    const float buildTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    BLASStats stats{};
    stats.name = mesh.debugName.empty() ? std::string("BLAS") : mesh.debugName;
    stats.triangleCount = mesh.indexBuffer.m_IndexCount / 3;
    stats.geometryCount = 1;
    stats.resultSizeBytes = info.ResultDataMaxSizeInBytes;
    stats.compactedSizeBytes = 0;
    stats.scratchSizeBytes = info.ScratchDataSizeInBytes;
    stats.updateScratchSizeBytes = info.UpdateScratchDataSizeInBytes;
    stats.buildTimeMs = buildTimeMs;
    stats.updateTimeMs = 0.0f;
    stats.allowCompaction = false;
    stats.allowUpdate = (inputs.Flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE) != 0;
    stats.preferFastTrace = (inputs.Flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE) != 0;
    stats.preferFastBuild = (inputs.Flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD) != 0;

    auto existing = std::find_if(m_BLASStats.begin(), m_BLASStats.end(),
        [&](const BLASStats& candidate)
        {
            return candidate.name == stats.name;
        });
    if (existing != m_BLASStats.end())
        *existing = stats;
    else
        m_BLASStats.push_back(stats);

    m_RenderStats.blasCount = static_cast<uint32_t>(m_BLASStats.size());
    m_RenderStats.blasBuildTotalMs = 0.0f;
    for (const auto& item : m_BLASStats)
        m_RenderStats.blasBuildTotalMs += item.buildTimeMs;

    scratch->Release();
}

bool Render::RenderRaytracedScene(
    const std::vector<RaytracingObject>& objects,
    Mesh& triangle,
    Mesh& cuad,
    Mesh& pentagon,
    Mesh& hexagon,
    Mesh& circle,
    Mesh& cube,
    Mesh& sphere,
    Mesh& plane)
{
    if (!m_DXRReady ||
        !m_DXRStateObject ||
        !m_DXRRootSig ||
        !m_DXRStateObjectProperties ||
        !m_DXRShaderTable ||
        !m_DXROutput)
    {
        return false;
    }

    if (objects.empty())
    {
        return false;
    }

    UpdateDXRConstants();

    const uint32_t objectCount = static_cast<uint32_t>(objects.size());
    m_RenderStats.objectCount = objectCount;
    m_RenderStats.shapeInstanceCounts.fill(0);

    const uint32_t instanceStride = sizeof(DxrInstanceData);
    const uint32_t instanceSize = objectCount * instanceStride;

    if (!m_DXRInstanceBuffer.m_Resource || m_DXRInstanceBuffer.m_Size < instanceSize)
    {
        m_DXRInstanceBuffer.Destroy();
        m_DXRInstanceBuffer = CreateStructuredBuffer(instanceSize, instanceStride);
    }

    if (!m_DXRInstanceBuffer.m_Resource || !m_DXRInstanceBuffer.m_MappedData)
    {
        return false;
    }

    CreateStructuredBufferSRV(
        m_DXRInstanceBuffer.m_Resource,
        m_DXRInstanceDescriptorIndex,
        instanceStride,
        objectCount
    );

    const uint32_t descSize = objectCount * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);

    if (!m_DXRInstanceDescBuffer.m_Resource || m_DXRInstanceDescBuffer.m_Size < descSize)
    {
        m_DXRInstanceDescBuffer.Destroy();
        m_DXRInstanceDescBuffer = CreateStructuredBuffer(descSize, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
    }

    if (!m_DXRInstanceDescBuffer.m_Resource || !m_DXRInstanceDescBuffer.m_MappedData)
    {
        return false;
    }

    const auto uploadStart = std::chrono::high_resolution_clock::now();

    auto meshForShape = [&](ShapeType shape) -> Mesh*
        {
            switch (shape)
            {
            case ShapeType::Triangle: return &triangle;
            case ShapeType::Cuad:     return &cuad;
            case ShapeType::Pentagon: return &pentagon;
            case ShapeType::Hexagon:  return &hexagon;
            case ShapeType::Circle:   return &circle;
            case ShapeType::Cube:     return &cube;
            case ShapeType::Sphere:   return &sphere;
            case ShapeType::Plane:    return &plane;
            default:                  return nullptr;
            }
        };

    std::vector<DxrInstanceData> dxrInstances(objectCount);
    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> tlasDesc(objectCount);

    for (uint32_t i = 0; i < objectCount; ++i)
    {
        dxrInstances[i].instance = objects[i].instance;
        dxrInstances[i].shapeType = static_cast<uint32_t>(objects[i].shapeType);
        const uint32_t shapeIndex = static_cast<uint32_t>(objects[i].shapeType);
        if (shapeIndex < m_RenderStats.shapeInstanceCounts.size())
            ++m_RenderStats.shapeInstanceCounts[shapeIndex];

        Mesh* mesh = meshForShape(objects[i].shapeType);
        if (!mesh)
        {
            continue;
        }

        if (!mesh->bottomLevelAS)
            BuildBottomLevelAS(*mesh);

        if (!mesh->bottomLevelAS)
        {
            continue;
        }

        std::memset(&tlasDesc[i], 0, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));

        // OJO:
        // DXR espera matriz 3x4 row-major.
        // Esto mantiene tu forma actual, pero si ves objetos raros,
        // revisamos despu�s esta matriz.
        DirectX::XMMATRIX worldMatrixT = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&objects[i].instance.worldMatrix));
        std::memcpy(
            tlasDesc[i].Transform,
            &worldMatrixT,
            sizeof(float) * 12
        );

        tlasDesc[i].InstanceID = i;
        tlasDesc[i].InstanceMask = 0xFF;
        tlasDesc[i].InstanceContributionToHitGroupIndex = 0;
        tlasDesc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        tlasDesc[i].AccelerationStructure = mesh->bottomLevelAS->GetGPUVirtualAddress();
    }

    std::memcpy(
        m_DXRInstanceBuffer.m_MappedData,
        dxrInstances.data(),
        instanceSize
    );

    std::memcpy(
        m_DXRInstanceDescBuffer.m_MappedData,
        tlasDesc.data(),
        descSize
    );
    const auto uploadEnd = std::chrono::high_resolution_clock::now();
    m_RenderStats.uploadMs = std::chrono::duration<float, std::milli>(uploadEnd - uploadStart).count();

    triangle.currentInstanceCount = m_RenderStats.shapeInstanceCounts[static_cast<uint32_t>(ShapeType::Triangle)];
    cuad.currentInstanceCount = m_RenderStats.shapeInstanceCounts[static_cast<uint32_t>(ShapeType::Cuad)];
    pentagon.currentInstanceCount = m_RenderStats.shapeInstanceCounts[static_cast<uint32_t>(ShapeType::Pentagon)];
    hexagon.currentInstanceCount = m_RenderStats.shapeInstanceCounts[static_cast<uint32_t>(ShapeType::Hexagon)];
    circle.currentInstanceCount = m_RenderStats.shapeInstanceCounts[static_cast<uint32_t>(ShapeType::Circle)];
    cube.currentInstanceCount = m_RenderStats.shapeInstanceCounts[static_cast<uint32_t>(ShapeType::Cube)];
    sphere.currentInstanceCount = m_RenderStats.shapeInstanceCounts[static_cast<uint32_t>(ShapeType::Sphere)];
    plane.currentInstanceCount = m_RenderStats.shapeInstanceCounts[static_cast<uint32_t>(ShapeType::Plane)];

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs = objectCount;
    inputs.InstanceDescs = m_DXRInstanceDescBuffer.m_Resource->GetGPUVirtualAddress();
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
    m_Device5->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

    auto createBuffer = [&](ID3D12Resource** resource,
        uint64_t size,
        D3D12_RESOURCE_FLAGS flags,
        D3D12_RESOURCE_STATES initState) -> bool
        {
            if (*resource)
            {
                (*resource)->Release();
                *resource = nullptr;
            }

            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

            D3D12_RESOURCE_DESC desc = {};
            desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            desc.Width = size;
            desc.Height = 1;
            desc.DepthOrArraySize = 1;
            desc.MipLevels = 1;
            desc.Format = DXGI_FORMAT_UNKNOWN;
            desc.SampleDesc.Count = 1;
            desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            desc.Flags = flags;

            return SUCCEEDED(m_Device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                initState,
                nullptr,
                IID_PPV_ARGS(resource)
            ));
        };

    if (!m_DXRTLAS ||
        !m_DXRTLASScratch ||
        m_DXRTLAS->GetDesc().Width < info.ResultDataMaxSizeInBytes ||
        m_DXRTLASScratch->GetDesc().Width < info.ScratchDataSizeInBytes)
    {
        if (!createBuffer(
            &m_DXRTLAS,
            info.ResultDataMaxSizeInBytes,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE))
        {
            return false;
        }

        if (!createBuffer(
            &m_DXRTLASScratch,
            info.ScratchDataSizeInBytes,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
        {
            return false;
        }
    }

    if (FAILED(m_CommandAlloc->Reset()) ||
        FAILED(m_CommandList4->Reset(m_CommandAlloc, nullptr)))
    {
        return false;
    }


    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs = inputs;
    buildDesc.DestAccelerationStructureData = m_DXRTLAS->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData = m_DXRTLASScratch->GetGPUVirtualAddress();

    const auto tlasBuildStart = std::chrono::high_resolution_clock::now();
    m_CommandList4->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    D3D12_RESOURCE_BARRIER tlasBarrier = {};
    tlasBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    tlasBarrier.UAV.pResource = m_DXRTLAS;
    m_CommandList4->ResourceBarrier(1, &tlasBarrier);
    const auto tlasBuildEnd = std::chrono::high_resolution_clock::now();
    m_RenderStats.tlasBuildMs = std::chrono::duration<float, std::milli>(tlasBuildEnd - tlasBuildStart).count();


    ID3D12DescriptorHeap* heaps[] = { m_GameTextureHeap.m_Heap };
    m_CommandList4->SetDescriptorHeaps(1, heaps);

    m_CommandList4->SetComputeRootSignature(m_DXRRootSig);

    m_CommandList4->SetComputeRootConstantBufferView(
        0,
        m_DXRConstantBuffer.m_Resource->GetGPUVirtualAddress()
    );

    m_CommandList4->SetComputeRootDescriptorTable(
        1,
        m_GameTextureHeap.GetGPUHandle(m_DXRInstanceDescriptorIndex)
    );

    m_CommandList4->SetComputeRootShaderResourceView(
        2,
        m_DXRTLAS->GetGPUVirtualAddress()
    );

    m_CommandList4->SetComputeRootDescriptorTable(
        3,
        m_GameTextureHeap.GetGPUHandle(m_DXROutputDescriptorIndex)
    );

    m_CommandList4->SetComputeRootDescriptorTable(
        4,
        m_GameTextureHeap.GetGPUHandle(0)
    );

    m_CommandList4->SetPipelineState1(m_DXRStateObject);

    const uint64_t shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    const uint64_t recordSize =
        (shaderIdSize + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1) &
        ~(uint64_t)(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1);

    D3D12_GPU_VIRTUAL_ADDRESS sbtAddress = m_DXRShaderTable->GetGPUVirtualAddress();

    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    dispatchDesc.RayGenerationShaderRecord.StartAddress = sbtAddress + recordSize * 0;
    dispatchDesc.RayGenerationShaderRecord.SizeInBytes = recordSize;

    dispatchDesc.MissShaderTable.StartAddress = sbtAddress + recordSize * 1;
    dispatchDesc.MissShaderTable.SizeInBytes = recordSize;
    dispatchDesc.MissShaderTable.StrideInBytes = recordSize;

    dispatchDesc.HitGroupTable.StartAddress = sbtAddress + recordSize * 2;
    dispatchDesc.HitGroupTable.SizeInBytes = recordSize;
    dispatchDesc.HitGroupTable.StrideInBytes = recordSize;

    dispatchDesc.Width = m_Width;
    dispatchDesc.Height = m_Height;
    dispatchDesc.Depth = 1;

    m_CommandList4->DispatchRays(&dispatchDesc);


    D3D12_RESOURCE_BARRIER outputUavBarrier = {};
    outputUavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    outputUavBarrier.UAV.pResource = m_DXROutput;
    m_CommandList4->ResourceBarrier(1, &outputUavBarrier);


    const uint32_t backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    D3D12_RESOURCE_BARRIER copyBarriers[2] = {};

    copyBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    copyBarriers[0].Transition.pResource = m_RenderTargets[backBufferIndex];
    copyBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    copyBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    copyBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    copyBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    copyBarriers[1].Transition.pResource = m_DXROutput;
    copyBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    copyBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    copyBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    const auto rayDispatchStart = std::chrono::high_resolution_clock::now();
    m_CommandList4->ResourceBarrier(2, copyBarriers);
    const auto copyStart = std::chrono::high_resolution_clock::now();
    m_CommandList4->CopyResource(
        m_RenderTargets[backBufferIndex],
        m_DXROutput
    );
    const auto copyEnd = std::chrono::high_resolution_clock::now();
    m_RenderStats.copyMs = std::chrono::duration<float, std::milli>(copyEnd - copyStart).count();


    D3D12_RESOURCE_BARRIER postCopyBarriers[2] = {};

    postCopyBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postCopyBarriers[0].Transition.pResource = m_DXROutput;
    postCopyBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    postCopyBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    postCopyBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    postCopyBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postCopyBarriers[1].Transition.pResource = m_RenderTargets[backBufferIndex];
    postCopyBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    postCopyBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    postCopyBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_CommandList4->ResourceBarrier(2, postCopyBarriers);
    const auto rayDispatchEnd = std::chrono::high_resolution_clock::now();
    m_RenderStats.rayDispatchMs = std::chrono::duration<float, std::milli>(rayDispatchEnd - rayDispatchStart).count();

    return true;
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

void Render::ResolveMSAA()
{
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
    BuildBottomLevelAS(mesh);
    return mesh;
}

void Render::UpdateInstanceBuffer(Mesh& mesh, InstanceData* data, uint32_t count)
{
    if (!mesh.instanceBuffer.m_Resource || !mesh.instanceBuffer.m_MappedData || !data || count == 0)
        return;

    if (count > mesh.maxInstanceCount) count = mesh.maxInstanceCount;
    if (count == 0)
        return;

    mesh.currentInstanceCount = count;
    memcpy(mesh.instanceBuffer.m_MappedData, data, count * sizeof(InstanceData));
}

void Render::Reset()
{
    m_CommandAlloc->Reset();
    commandList->Reset(m_CommandAlloc, nullptr);
}

void Render::Clear()
{

}

void Render::BeginFrame()
{


}

bool Render::BeginGame()
{

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

    // El backbuffer ya debe estar en RENDER_TARGET
    // porque RenderRaytracedScene lo deja as� despu�s del CopyResource.
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


void Render::PresentDXRFrame()
{
    const auto presentStart = std::chrono::high_resolution_clock::now();
    const uint32_t backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    D3D12_RESOURCE_BARRIER presentBarrier = {};
    presentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    presentBarrier.Transition.pResource = m_RenderTargets[backBufferIndex];
    presentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    presentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    presentBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    commandList->ResourceBarrier(1, &presentBarrier);

    HRESULT closeHr = commandList->Close();
    if (FAILED(closeHr))
    {

        return;
    }

    ID3D12CommandList* lists[] = { commandList };
    m_CommandQueue->ExecuteCommandLists(1, lists);

    m_SwapChain->Present(1, 0);

    const auto presentEnd = std::chrono::high_resolution_clock::now();
    m_RenderStats.presentMs = std::chrono::duration<float, std::milli>(presentEnd - presentStart).count();
}


bool Render::RenderDXRFrame(
    const std::vector<RaytracingObject>& objects,
    Mesh& triangle,
    Mesh& cuad,
    Mesh& pentagon,
    Mesh& hexagon,
    Mesh& circle,
    Mesh& cube,
    Mesh& sphere,
    Mesh& plane,
    bool drawImGui)
{
    // ImGui frame es opcional y es CPU-side.
    // No cambia a pipeline raster todav�a.
    if (drawImGui)
    {
        BeginImGuiFrame();

        // Aqu� puedes crear tus ventanas ImGui si quieres,
        // pero normalmente las haces desde fuera.
        //
        // Ejemplo:
        // ImGui::Begin("DXR Debug");
        // ImGui::Text("RTPSO activo");
        // ImGui::End();
    }

    const bool rendered = RenderRaytracedScene(
        objects,
        triangle,
        cuad,
        pentagon,
        hexagon,
        circle,
        cube,
        sphere,
        plane
    );

    if (!rendered)
    {
        return false;
    }

    if (drawImGui)
    {
        RenderImGui();
    }

    PresentDXRFrame();

    return true;
}




void Render::Loop()
{
    PresentDXRFrame();
}

void Render::Cleanup()
{
    ShutdownImGui();
    for (auto& texture : m_Textures)
        texture.Destroy();
    m_Textures.clear();
    m_GameTextureHeap.Destroy();
    if (m_DXRShaderTable) m_DXRShaderTable->Release();
    if (m_DXRRootSig) m_DXRRootSig->Release();
    if (m_DXRStateObjectProperties) m_DXRStateObjectProperties->Release();
    if (m_DXRStateObject) m_DXRStateObject->Release();
    if (m_DXROutput) m_DXROutput->Release();
    if (m_DXRTLAS) m_DXRTLAS->Release();
    if (m_DXRTLASScratch) m_DXRTLASScratch->Release();
    m_DXRConstantBuffer.Destroy();
    m_DXRInstanceBuffer.Destroy();
    m_DXRInstanceDescBuffer.Destroy();
    if (m_MSAAColorBuffer) m_MSAAColorBuffer->Release();
    if (m_DepthStencilBuffer) m_DepthStencilBuffer->Release();
    if (m_GamePSO) m_GamePSO->Release();
    if (m_GameRootSig) m_GameRootSig->Release();
    for (auto& rt : m_RenderTargets) if (rt) rt->Release();
    m_RTVHeap.Destroy();
    m_DSVHeap.Destroy();
    m_SRVHeap.Destroy();
    if (m_CommandList4) m_CommandList4->Release();
    if (m_Device5) m_Device5->Release();
    if (m_SwapChain) m_SwapChain->Release();
    if (m_CommandQueue) m_CommandQueue->Release();
    if (m_Device) m_Device->Release();
    if (m_CommandAlloc) m_CommandAlloc->Release();
    if (m_Fence) m_Fence->Release();
    if (m_FenceEvent) CloseHandle(m_FenceEvent);
    if (commandList) commandList->Release();
    if (m_CoInitialized) CoUninitialize();
}
