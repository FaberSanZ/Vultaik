#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <iostream>
#include <d3dcompiler.h>
#include <dxcapi.h>
#include "ShaderCompiler.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class DescriptorHeap
{
public:
    ID3D12DescriptorHeap* m_Heap = nullptr;
    uint32_t m_DescriptorSize = 0;

    DescriptorHeap() = default;

    void Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = type;
        heapDesc.NumDescriptors = numDescriptors;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = 0;
        device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap));
        m_DescriptorSize = device->GetDescriptorHandleIncrementSize(type);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = GetCPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_DescriptorSize;
        return handle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() const
    {
        return m_Heap->GetCPUDescriptorHandleForHeapStart();
    }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() const
    {
        return m_Heap->GetGPUDescriptorHandleForHeapStart();
    }

    uint32_t GetDescriptorSize() const
    {
        return m_DescriptorSize;
    }

    void Destroy()
    {
        if (m_Heap)
        {
            m_Heap->Release();
            m_Heap = nullptr;
        }
    }
};


class VertexBuffer
{
public:
    VertexBuffer() = default;

    ID3D12Resource* m_vertexBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    uint32_t stride = { };
    uint32_t size = { };

    void Destroy()
    {
        if (m_vertexBuffer)
        {
            m_vertexBuffer->Release();
            m_vertexBuffer = nullptr;
        }
    }
};

class IndexBuffer
{
public:
    IndexBuffer() = default;
    ID3D12Resource* m_indexBuffer = nullptr;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    uint32_t stride = { };
    uint32_t size = { };
    uint32_t m_indexCount{ };

    void Destroy()
    {
        if (m_indexBuffer)
        {
            m_indexBuffer->Release();
            m_indexBuffer = nullptr;
        }
    }

};


class StructuredBuffer
{
public:
    StructuredBuffer() = default;

    ID3D12Resource* handle = nullptr;
    uint32_t stride = { };
    uint32_t size = { };

    void Destroy()
    {
        if (handle)
        {
            handle->Release();
            handle = nullptr;
        }
    }
};


// Define vertices for a triangle
struct Vertex
{
    float position[4];
};


struct InstanceData
{
    DirectX::XMMATRIX worldMatrix;
    DirectX::XMFLOAT4 color;
};


struct Mesh
{
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
	StructuredBuffer instanceBuffer;
	uint32_t maxInstanceCount = 128 * 128; // Example maximum instance count, adjust as needed
    uint32_t currentInstanceCount = 0;


    Mesh() = default;


    void Draw(ID3D12GraphicsCommandList* commandList)
    {
        commandList->IASetVertexBuffers(0, 1, &vertexBuffer.m_vertexBufferView);
        commandList->IASetIndexBuffer(&indexBuffer.m_indexBufferView);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->SetGraphicsRootShaderResourceView(1, instanceBuffer.handle->GetGPUVirtualAddress());
        commandList->DrawIndexedInstanced(indexBuffer.m_indexCount, currentInstanceCount, 0, 0, 0);


    }

    void Destroy()
    {
        vertexBuffer.Destroy();
        indexBuffer.Destroy();
    }
};

class Render
{
public:


public:
    Render() = default;

    uint32_t m_Width{ };
    uint32_t m_Height{ };
    uint32_t m_FrameCount{ 2 };

    // Render device and resources
    ID3D12Device* device = nullptr;
    ID3D12CommandQueue* commandQueue = nullptr;
    IDXGISwapChain3* swapChain = nullptr;
    ID3D12Resource* renderTargets[2];
    ID3D12CommandAllocator* commandAlloc = nullptr;
    ID3D12GraphicsCommandList* commandList = nullptr;

    ID3D12Resource* depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial

    // Pipeline state and root signature
    ID3D12PipelineState* pipelineState = nullptr;
    ID3D12RootSignature* rootSignature = nullptr;
    Core::ShaderCompiler shaderCompiler{};

    DescriptorHeap rtvDescriptorHeap{};  // This is a heap for our render target view descriptor
    DescriptorHeap dpvDescriptorHeap{};  // This is a heap for our depth/stencil buffer descriptor



    bool Initialize(HWND hwnd, uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;


        IDXGIFactory4* factory = nullptr;
        CreateDXGIFactory1(IID_PPV_ARGS(&factory));

        D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));



        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));


        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = m_FrameCount;
        swapChainDesc.Width = m_Width;
        swapChainDesc.Height = m_Height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        IDXGISwapChain1* tempSwapChain = nullptr;
        factory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, &tempSwapChain);


        tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
        factory->Release();




        // Create RTV descriptor heap
        rtvDescriptorHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_FrameCount);

        for (uint32_t i = 0; i < m_FrameCount; ++i)
        {
            ID3D12Resource* backBuffer = nullptr;
            swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));


            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap.GetCPUHandle(i);
            device->CreateRenderTargetView(backBuffer, nullptr, rtvHandle);

            renderTargets[i] = backBuffer;
        }


        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAlloc));
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAlloc, nullptr, IID_PPV_ARGS(&commandList));

        commandList->Close();


        CreateDepthBuffer();
        CreatePipeline();

        SetOrthographicMatrix((float)width, (float)height);

        return true;
    }
    void CreateDepthBuffer()
    {
        // Create depth stencil buffer
        D3D12_RESOURCE_DESC depthStencilDesc = {};
        depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthStencilDesc.Width = m_Width;
        depthStencilDesc.Height = m_Height;
        depthStencilDesc.DepthOrArraySize = 1;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        // Clear value for the depth stencil buffer
        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;


        // Create heap properties for the depth stencil buffer
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;


        // Create the depth stencil buffer resource
        device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depthStencilBuffer));

        // Create descriptor heap for depth stencil view (DSV)
        dpvDescriptorHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);


        // Create the depth stencil view (DSV)
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        dsvDesc.Texture2D.MipSlice = 0;

        device->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, dpvDescriptorHeap.GetCPUHandle(0));
    }

    void CreatePipeline()
    {
        auto vertexShaderBlob = shaderCompiler.Compile(L"../../../Assets/Shaders/DepthTests/VertexShader.hlsl", L"VS", L"vs_6_0");
        auto pixelShaderBlob = shaderCompiler.Compile(L"../../../Assets/Shaders/DepthTests/PixelShader.hlsl", L"PS", L"ps_6_0");


        // Crear root signature con 1 parámetro (constant buffer)
        D3D12_ROOT_PARAMETER rootParams[2];

        // Configurar como root constant (32-bit constants)
        // Esto permite pasar la matriz directamente sin un buffer separado
        rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParams[0].Constants.Num32BitValues = 16;  // 4x4 matrix = 16 floats
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
        device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

        // --- PIPELINE STATE ---
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = rootSignature;

        psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
        psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };



        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        psoDesc.InputLayout.NumElements = _countof(inputElementDescs);
        psoDesc.InputLayout.pInputElementDescs = inputElementDescs;

        // Rasterizer state manual
        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        rasterizerDesc.FrontCounterClockwise = false;
        rasterizerDesc.DepthClipEnable = true;
        psoDesc.RasterizerState = rasterizerDesc;

        // Blend state manual
        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.AlphaToCoverageEnable = false;
        blendDesc.IndependentBlendEnable = false;
        blendDesc.RenderTarget[0].BlendEnable = false;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        psoDesc.BlendState = blendDesc;

        // Depth stencil



        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        depthStencilDesc.StencilEnable = false;

        psoDesc.DepthStencilState = depthStencilDesc;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;



        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;

        device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
    }


    void SetOrthographicMatrix(float width, float height)
    {
        // Para depuración, usa valores fijos que muestren bien los objetos
        // Tus objetos están en el rango [-0.5, 0.5], así que queremos verlos bien
        float orthoWidth = 2.0f;   // Mostrará de -1 a 1 en X
        float orthoHeight = 2.0f;  // Mostrará de -1 a 1 en Y

        // Si quieres mantener el aspect ratio de la pantalla
        float aspect = width / height;

        DirectX::XMMATRIX orthoMatrix = DirectX::XMMatrixOrthographicOffCenterLH(
            -orthoWidth * aspect,   // left
            orthoWidth * aspect,    // right
            -orthoHeight,           // bottom
            orthoHeight,            // top
            0.1f,                   // near plane
            100.0f                  // far plane
        );

        m_projectionMatrix = orthoMatrix;



    }

    DirectX::XMMATRIX m_projectionMatrix;



    void SetRootConstants()
    {
        // Establecer los 16 floats de la matriz como root constants
        commandList->SetGraphicsRoot32BitConstants(0, 16, &m_projectionMatrix, 0);
    }


    Mesh CreateMesh(Vertex vertices[], uint32_t vertexCount, uint32_t indices[], uint32_t indexCount)
    {
        Mesh mesh{};


        mesh.vertexBuffer.stride = sizeof(Vertex);
        mesh.vertexBuffer.size = vertexCount * sizeof(Vertex);
        mesh.indexBuffer.stride = sizeof(uint32_t);
        mesh.indexBuffer.size = indexCount * sizeof(uint32_t);
        mesh.indexBuffer.m_indexCount = indexCount;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = vertexCount * sizeof(Vertex); 
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&mesh.vertexBuffer.m_vertexBuffer));
        if (FAILED(hr))
        {
            std::cout << "Failed to create vertex buffer. HRESULT: " << std::hex << hr << std::endl;
            return mesh;
        }


        void* pData;
        hr = mesh.vertexBuffer.m_vertexBuffer->Map(0, nullptr, &pData);
        if (SUCCEEDED(hr))
        {
            memcpy(pData, vertices, vertexCount * sizeof(Vertex));
            mesh.vertexBuffer.m_vertexBuffer->Unmap(0, nullptr);
        }

        mesh.vertexBuffer.m_vertexBufferView.BufferLocation = mesh.vertexBuffer.m_vertexBuffer->GetGPUVirtualAddress();
        mesh.vertexBuffer.m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        mesh.vertexBuffer.m_vertexBufferView.SizeInBytes = vertexCount * sizeof(Vertex);

        bufferDesc.Width = indexCount * sizeof(uint32_t);
        hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&mesh.indexBuffer.m_indexBuffer));
        if (FAILED(hr))
        {
            std::cout << "Failed to create index buffer. HRESULT: " << std::hex << hr << std::endl;
            return mesh;
        }

        hr = mesh.indexBuffer.m_indexBuffer->Map(0, nullptr, &pData);
        if (SUCCEEDED(hr))
        {
            memcpy(pData, indices, indexCount * sizeof(uint32_t));
            mesh.indexBuffer.m_indexBuffer->Unmap(0, nullptr);
        }

        // Configurar index buffer view
        mesh.indexBuffer.m_indexBufferView.BufferLocation = mesh.indexBuffer.m_indexBuffer->GetGPUVirtualAddress();
        mesh.indexBuffer.m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        mesh.indexBuffer.m_indexBufferView.SizeInBytes = indexCount * sizeof(uint32_t);
        mesh.indexBuffer.m_indexCount = indexCount;


		mesh.instanceBuffer = CreateStructuredBuffer(mesh.maxInstanceCount * sizeof(InstanceData), sizeof(InstanceData));

        return mesh;
    }

    void UpdateInstanceBuffer(Mesh& mesh, InstanceData instanceData[], uint32_t instanceCount)
    {
        if (instanceCount > mesh.maxInstanceCount)
        {
            std::cout << "Warning: Instance count exceeds max!" << std::endl;
            instanceCount = mesh.maxInstanceCount;
        }

        mesh.currentInstanceCount = instanceCount;

        void* pData;
        mesh.instanceBuffer.handle->Map(0, nullptr, &pData);
        memcpy(pData, instanceData, instanceCount * sizeof(InstanceData));
        mesh.instanceBuffer.handle->Unmap(0, nullptr);
        
    }

    StructuredBuffer CreateStructuredBuffer(uint32_t size, uint32_t stride)
    {
        StructuredBuffer buffer {};

        // Create vertex buffer
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = size;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


        device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer.handle));

		return buffer;
	}

    void Clear()
    {

        // get a handle to the render target view (RTV) for the current back buffer
        uint32_t backBufferIndex = swapChain->GetCurrentBackBufferIndex();

        // get a handle to the depth/stencil buffer
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dpvDescriptorHeap.GetCPUHandle(0);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap.GetCPUHandle(backBufferIndex);

        // Clear the render target
        float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        // Clear the depth stencil view
        commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


        // set the render target for the output merger stage (the output of the pipeline)
        // Set the render target view (RTV) for the current back buffer
        // Set the depth/stencil view (DSV) for the current frame
        commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
	}
    void Reset()
    {
        // Reset the command allocator and command list for the current frame
        commandAlloc->Reset();
        commandList->Reset(commandAlloc, nullptr);
	}

    // 
    void BeginFrame()
    {
        commandList->SetGraphicsRootSignature(rootSignature);

		SetRootConstants(); // Set the root constants (projection matrix) for the vertex shader



        // Set the viewport and scissor rect
        D3D12_VIEWPORT view = { 0, 0, m_Width, m_Height, 0.0f, 1.0f };
        D3D12_RECT scissorRect = { 0, 0, m_Width, m_Height };

        commandList->RSSetViewports(1, &view);
        commandList->RSSetScissorRects(1, &scissorRect);

        // draw the triangle
        commandList->SetPipelineState(pipelineState);
	}

    void Loop()
    {
        commandList->Close();

        // Execute the command list
        ID3D12CommandList* ppCommandLists[] = { commandList };
        commandQueue->ExecuteCommandLists(1, ppCommandLists);

        // Present the frame
        swapChain->Present(1, 0);
    }




    void Cleanup()
    {
        //if (indexBuffer.m_indexBuffer)
        //    indexBuffer.Destroy();

        //if (vertexBuffer.m_vertexBuffer)
        //    vertexBuffer.Destroy();

        if (depthStencilBuffer)
            depthStencilBuffer->Release();


        if (pipelineState)
            pipelineState->Release();

        for (uint32_t i = 0; i < 2; ++i)
            if (renderTargets[i])
                renderTargets[i]->Release();

        rtvDescriptorHeap.Destroy();
        dpvDescriptorHeap.Destroy();

        if (swapChain)
            swapChain->Release();

        if (commandQueue)
            commandQueue->Release();

        if (device)
            device->Release();

        if (commandAlloc)
            commandAlloc->Release();

        if (commandList)
            commandList->Release();
    }


};
