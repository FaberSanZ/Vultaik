// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <fstream>
#include <iostream>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "Adapter.h"
#include "Device.h"
#include "SwapChain.h"
#include "CommandList.h"
#include "Buffer.h"
#include "Texture.h"
#include "Pipeline.h"
#include "GameWindows.h"


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")


class RenderSystem
{
private:



    // Define a struct for the camera matrices (must match HLSL cbuffer layout)
    struct CameraBuffer
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    } cameraData;

    float m_CubeRotation = 0.0f; // Rotation angle for the cube (tools for game)

public:
    RenderSystem()
    {
    }

    uint32_t m_Width{ 1200 }; // Width of the render target
    uint32_t m_Height{ 820 }; // Height of the render target


    Graphics::Adapter adapter;
    Graphics::Device device;
    Graphics::SwapChain swapChain;
    Graphics::CommandList commandList;
    Graphics::Pipeline pipeline;
    Graphics::Buffer vertexBuffer;
    Graphics::Buffer indexBuffer;
    Graphics::Buffer constantBuffer;
    Graphics::Buffer instanceBuffer;

    void Initialize(HWND hwnd)
    {

        adapter.Initialize(0); // Initialize the first GPU adapter (0)
        device.Initialize(adapter); // Initialize the Direct3D device using the adapter
        swapChain.Initialize(device, hwnd, m_Width, m_Height); // Initialize the swap chain with the device and window handle
        commandList.Initialize(device.GetContext()); // Initialize the command list with the device context


        Graphics::VertexInputElement layout{};
        layout.Add(Graphics::VertexType::Position);
        layout.Add(Graphics::VertexType::Color);

        Graphics::PipelineDesc pipelineDesc{};
        pipelineDesc.fillMode = D3D11_FILL_SOLID; // Set fill mode to solid
        pipelineDesc.cullMode = D3D11_CULL_NONE; // Disable backface culling
        pipelineDesc.depthEnabled = true; // Enable depth testing
        pipelineDesc.vertexInputElement = layout; // Set the vertex input layout    
        pipeline.Initialize(device, pipelineDesc);

        CreateCamera();
        CreateMesh();

        std::wcout << L"GPU: " << adapter.GetGpuName() << std::endl;
        std::wcout << L"Dedicated Video Memory: " << adapter.GetDedicatedVideoMemory() / (1024 * 1024) << L" MB" << std::endl;
        std::wcout << L"Dedicated System Memory: " << adapter.GetDedicatedSystemMemory() / (1024 * 1024) << L" MB" << std::endl;
        std::wcout << L"Shared System Memory: " << adapter.GetSharedSystemMemory() / (1024 * 1024) << L" MB" << std::endl;
    }


    void Loop()
    {
        float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };

        Graphics::RenderPass& pass = swapChain.GetRenderPass();

        commandList.ClearRenderPass(pass, color);
        commandList.SetRenderPass(pass);
        commandList.SetViewport(m_Width, m_Height);
        commandList.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList.SetPipelineState(pipeline);


        constantBuffer.Bind(device.GetContext(), 0);        // ConstantBuffer: slot 0, stage VS
		instanceBuffer.Bind(device.GetContext(), 1);        // StructureBuffer: slot 1, stage VS
		commandList.DrawIndexedInstanced(36, numInstances, 0, 0, 0); 

 

        swapChain.Present(false); // Present the swap chain with vsync enabled
    }


    bool CreateMesh()
    {
        Graphics::VertexPositionColor vertices[] =
        {
            // Front face
            {{-0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},

            // Right side face
            {{ 0.5f, -0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},

            // Left side face
            {{-0.5f,  0.5f,  0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},

            // Back face
            {{ 0.5f,  0.5f,  0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},

            // Top face
            {{-0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},

            // Bottom face
            {{ 0.5f, -0.5f,  0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        };


        vertexBuffer.Initialize(device, Graphics::BufferType::VertexBuffer, vertices, sizeof(vertices), sizeof(Graphics::VertexPositionColor));
        vertexBuffer.Bind(device.GetContext());



        uint32_t indices[] =
        {
            // front face
            0, 1, 2, // first triangle
            0, 3, 1, // second triangle

            // left face
            4, 5, 6, // first triangle
            4, 7, 5, // second triangle

            // right face
            8, 9, 10, // first triangle
            8, 11, 9, // second triangle

            // back face
            12, 13, 14, // first triangle
            12, 15, 13, // second triangle

            // top face
            16, 17, 18, // first triangle
            16, 19, 17, // second triangle

            // bottom face
            20, 21, 22, // first triangle
            20, 23, 21, // second triangle
        };



        indexBuffer.Initialize(device, Graphics::BufferType::IndexBuffer, indices, sizeof(indices));
        indexBuffer.Bind(device.GetContext());


        return true;
    }




    void CreateCamera()
    {
        DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH({ 0, 0, -36 }, { 0, 0, 0 }, { 0, 1, 0 });

        // Set up projection matrix (perspective)
        float fov = 45.0f * (3.14f / 180.0f);
        float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
        float nearZ = 0.1f;
        float farZ = 1000.0f;
        DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

        // Transpose matrices for HLSL (row-major in C++, column-major in HLSL)
        cameraData.view = DirectX::XMMatrixTranspose(view);
        cameraData.projection = DirectX::XMMatrixTranspose(projection);



        constantBuffer.Initialize(device, Graphics::BufferType::ConstantBuffer, &cameraData, sizeof(CameraBuffer));



        instanceBuffer.Initialize(device, Graphics::BufferType::StructuredBuffer, nullptr, sizeof(DirectX::XMMATRIX) * 256 * 256 * 8, sizeof(DirectX::XMMATRIX));

    }


	uint32_t numInstances = 256 * 256;
	float dimension = 1.6f;
    void UpdateCamera()
    {
        m_CubeRotation += 0.01f;

        constantBuffer.Update(device.GetContext(), &cameraData, sizeof(CameraBuffer));


        std::vector<DirectX::XMMATRIX> dataArray(numInstances);
        uint32_t dim = static_cast<uint32_t>(std::cbrt(numInstances)); // using cube root to determine the dimension of the grid
        DirectX::XMFLOAT3 offset = { dimension, dimension, dimension };

        float halfDimOffsetX = (dim * offset.x) / 2.0f;
        float halfDimOffsetY = (dim * offset.y) / 2.0f;
        float halfDimOffsetZ = (dim * offset.z) / 2.0f;

        for (uint32_t x = 0; x < dim; ++x)
        {
            for (uint32_t y = 0; y < dim; ++y)
            {
                for (uint32_t z = 0; z < dim; ++z)
                {
                    uint32_t index = x * dim * dim + y * dim + z;


                    DirectX::XMFLOAT3 position =
                    {
                        -halfDimOffsetX + offset.x / 2.0f + x * offset.x,
                        -halfDimOffsetY + offset.y / 2.0f + y * offset.y,
                        -halfDimOffsetZ + offset.z / 2.0f + z * offset.z
                    };


                    DirectX::XMFLOAT3 cubeRotation =
                    {
                        m_CubeRotation,
                        m_CubeRotation,
                        m_CubeRotation
                    };
                    if (index % 2 == 0)
                    {
                        cubeRotation.x = -m_CubeRotation;
                        cubeRotation.y = -m_CubeRotation;
                        cubeRotation.z = -m_CubeRotation;
                    }
                    DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
                    DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYaw(cubeRotation.x, cubeRotation.y, cubeRotation.z);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.25f, 0.25f, 0.25f);
                    DirectX::XMMATRIX world = DirectX::XMMatrixTranspose(rot * trans * scale);
                    dataArray[index] = world;
                }
            }
        }

        instanceBuffer.Update(device.GetContext(), dataArray.data(), sizeof(DirectX::XMMATRIX) * static_cast<uint32_t>(dataArray.size()));

    }





    void Cleanup()
    {
        //if (constantBuffer.data)
        //    constantBuffer.data = nullptr;

        //if (constantBuffer.buffer)
        //    constantBuffer.buffer->Release();

        //if (pipeline.rasterState)
        //    pipeline.rasterState->Release();

        //if (pipeline.depthStencilState)
        //    pipeline.depthStencilState->Release();

        //if (renderDevice.depthStencilView)
        //    renderDevice.depthStencilView->Release();

        //if (renderDevice.depthStencilBuffer)
        //    renderDevice.depthStencilBuffer->Release();

        //if (indexBuffer.buffer)
        //    indexBuffer.buffer->Release();

        //if (vertexBuffer.buffer)
        //    vertexBuffer.buffer->Release();

        //if (pipeline.inputLayout)
        //    pipeline.inputLayout->Release();

        //if (pipeline.vertexShader)
        //    pipeline.vertexShader->Release();

        //if (pipeline.pixelShader)
        //    pipeline.pixelShader->Release();

        //if (renderDevice.swapChain)
        //    renderDevice.swapChain->Release();

        //if (renderDevice.renderTargetView)
        //    renderDevice.renderTargetView->Release();

        //if (renderDevice.deviceContext)
        //    renderDevice.deviceContext->Release();

        //if (renderDevice.device)
        //    renderDevice.device->Release();
    }

};


int main()
{
    RenderSystem render = {};

    GameWindows gameWindow{ };
	gameWindow.OnInitialize();
    render.Initialize(gameWindow.GetHandle());

    while (gameWindow.IsRunning())
    {
        gameWindow.PumpMessages();

		render.UpdateCamera();
		render.Loop();

    }

    render.Cleanup();
    return 0;
}