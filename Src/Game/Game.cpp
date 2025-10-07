#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3Dcompiler.lib")

const char* vertexShader = R"(
struct VSOutput {
    float4 pos : SV_POSITION;
};
VSOutput vs(uint vid : SV_VERTEXID) {
    float2 vertices[3] = {
        float2(0.0, 0.5),
        float2(0.5, -0.5), 
        float2(-0.5, -0.5)
    };
    VSOutput output;
    output.pos = float4(vertices[vid], 0, 1);
    return output;
}
)";

const char* pixelShader = R"(
float4 ps() : SV_TARGET {
    return float4(1, 0, 0, 1);
}
)";

int main() {
    // Crear ventana
    WNDCLASS wc = {};
    wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT 
    {
        if (msg == WM_DESTROY) 
            PostQuitMessage(0);

        return DefWindowProc(hwnd, msg, wParam, lParam);
    };
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"Ventana";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, L"triangle DX12", WS_OVERLAPPEDWINDOW,
        100, 100, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);

    // Inicializar DX12
    ID3D12Device* device = nullptr;
    IDXGISwapChain3* swapChain = nullptr;
    ID3D12CommandQueue* commandQueue = nullptr;
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12GraphicsCommandList* commandList = nullptr;
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    ID3D12DescriptorHeap* rtvHeap = nullptr;
    ID3D12Resource* renderTargets[2] = {};

    // Factory y Device
    IDXGIFactory4* factory = nullptr;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

    // Command Queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

    // Swap Chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = 800;
    swapChainDesc.Height = 600;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    IDXGISwapChain1* tempSwapChain = nullptr;
    factory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, &tempSwapChain);
    tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
    tempSwapChain->Release();
    factory->Release();

    // RTV Heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	// create RTVs
    UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < 2; i++) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    // Command Allocator y List
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));

    // Root Signature
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* signatureBlob = nullptr;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, nullptr);
    device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    signatureBlob->Release();

    // shaders
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    D3DCompile(vertexShader, strlen(vertexShader), nullptr, nullptr, nullptr, "vs", "vs_5_0", 0, 0, &vsBlob, nullptr);
    D3DCompile(pixelShader, strlen(pixelShader), nullptr, nullptr, nullptr, "ps", "ps_5_0", 0, 0, &psBlob, nullptr);

    // Pipeline State
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature;
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

    vsBlob->Release();
    psBlob->Release();
    commandList->Close();

    // Loop
    MSG msg = {};
    while (msg.message != WM_QUIT) 
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else 
        {
            // Render frame
            commandAllocator->Reset();
            commandList->Reset(commandAllocator, pipelineState);

            UINT frameIndex = swapChain->GetCurrentBackBufferIndex();
            D3D12_CPU_DESCRIPTOR_HANDLE currentRTV = rtvHeap->GetCPUDescriptorHandleForHeapStart();
            currentRTV.ptr += frameIndex * rtvDescriptorSize;

            commandList->SetGraphicsRootSignature(rootSignature);
            commandList->SetPipelineState(pipelineState);

            D3D12_VIEWPORT viewport = { 0, 0, 800, 600, 0, 1 };
            D3D12_RECT scissor = { 0, 0, 800, 600 };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissor);

            commandList->OMSetRenderTargets(1, &currentRTV, FALSE, nullptr);

            float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
            commandList->ClearRenderTargetView(currentRTV, clearColor, 0, nullptr);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->DrawInstanced(3, 1, 0, 0);

            commandList->Close();

            ID3D12CommandList* commandLists[] = { commandList };
            commandQueue->ExecuteCommandLists(1, commandLists);
            swapChain->Present(1, 0);
        }
    }

    return 0;
}