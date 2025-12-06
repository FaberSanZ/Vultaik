
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
#include "GameTime.h"
#include "entt.hpp"
#include "Components.h"


class RenderSystem
{
private:



    // Define a struct for the camera matrices (must match HLSL cbuffer layout)
    struct CameraBuffer
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    } cameraData;


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
    Graphics::Buffer constantBuffer;


    void OnInitialize(entt::registry& registry, HWND hwnd)
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



    }


    void Loop(entt::registry& registry)
    {
        float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };

        Graphics::RenderPass& pass = swapChain.GetRenderPass();

        commandList.ClearRenderPass(pass, color);
        commandList.SetRenderPass(pass);
        commandList.SetViewport(m_Width, m_Height);
        commandList.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList.SetPipelineState(pipeline);

        constantBuffer.Bind(device.GetContext(), 0);        // ConstantBuffer: slot 0, stage VS


        auto view_mesh = registry.view<MeshComponent, InstanceComponent>();
        for (auto [entity, mesh, ins] : view_mesh.each())
        {
            uint32_t instanceCount = ins.words.size();


            mesh.mesh.vertexBuffer.Bind(device.GetContext());
            mesh.mesh.indexBuffer.Bind(device.GetContext());
            mesh.mesh.InstanceBuffer.Bind(device.GetContext(), 1); // StructuredBuffer: slot 1, stage VS
            commandList.DrawIndexedInstanced(mesh.mesh.indexCount, instanceCount, 0, 0, 0);
		}




        swapChain.Present(false); // Present the swap chain with vsync enabled
    }

    void CreateCamera()
    {
        DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH({ 0, 0, -10 }, { 0, 0, 0 }, { 0, 1, 0 });

        // Set up projection matrix (perspective)
        float fov = 45.0f * (3.14f / 180.0f);
        float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
        float nearZ = 0.1f;
        float farZ = 1000.0f;
        DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

        // Transpose matrices for HLSL (row-major in C++, column-major in HLSL)
        cameraData.view = DirectX::XMMatrixTranspose(view);
        cameraData.projection = DirectX::XMMatrixTranspose(projection);



        constantBuffer.Initialize(device, Graphics::BufferType::ConstantBuffer, nullptr, sizeof(CameraBuffer));

    }




    void Update(entt::registry& registry, GameTime time)
    {
        // Update the constant buffer with the latest camera data
        commandList.UpdateBuffer(constantBuffer, &cameraData, sizeof(CameraBuffer));






        auto view_mesh = registry.view<InstanceComponent, MeshComponent>();
        for (auto [entity, instance, mesh] : view_mesh.each())
        {


            std::vector<DirectX::XMMATRIX> wordInstancing;

    //        DirectX::XMMATRIX baseRot = DirectX::XMMatrixRotationRollPitchYaw(transform.rotationX, transform.rotationY, transform.rotationZ);
    //        DirectX::XMMATRIX baseScale = DirectX::XMMatrixScaling(transform.scale, transform.scale, transform.scale);


    //        {
    //            DirectX::XMMATRIX baseTrans = DirectX::XMMatrixTranslation(transform.x, transform.y, transform.z);
    //            DirectX::XMMATRIX baseWorld = DirectX::XMMatrixTranspose(baseScale * baseRot * baseTrans);
    //            wordInstancing.push_back(baseWorld);
				//std::cout << "Base World Matrix for Entity " << static_cast<uint32_t>(entity) << ":\n";
    //        }

            if (not instance.words.empty())
            {
                for (const auto& instance : instance.words)
                {
                    //float finalX = transform.x + instancePos.x;
                    //float finalY = transform.y + instancePos.y;
                    //float finalZ = transform.z + instancePos.z;

                    //DirectX::XMMATRIX instTrans = DirectX::XMMatrixTranslation(finalX, finalY, finalZ);
                    //DirectX::XMMATRIX instWorld = DirectX::XMMatrixTranspose(baseScale * baseRot * instTrans);
					DirectX::XMMATRIX instTrans = DirectX::XMMatrixTranslation(instance.position.x, instance.position.y, instance.position.z);
					DirectX::XMMATRIX instRot = DirectX::XMMatrixRotationRollPitchYaw(instance.rotation.x, instance.rotation.y, instance.rotation.z);
					DirectX::XMMATRIX instScale = DirectX::XMMatrixScaling(instance.scale.x, instance.scale.y, instance.scale.z);
					DirectX::XMMATRIX instWorld = DirectX::XMMatrixTranspose(instScale * instRot * instTrans);

                    wordInstancing.push_back(instWorld);
                }
            }

            uint32_t numInstances = static_cast<uint32_t>(wordInstancing.size());


            if (mesh.shapeType == ShapeType::Cube)
            {
                if (not mesh.dirty)
                {
                    uint32_t indexCount = 36;

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

                    mesh.mesh.vertexBuffer.Initialize(device, Graphics::BufferType::VertexBuffer, vertices, sizeof(vertices), sizeof(Graphics::VertexPositionColor));




                    mesh.mesh.indexBuffer.Initialize(device, Graphics::BufferType::IndexBuffer, indices, sizeof(indices));
					mesh.mesh.indexCount = indexCount;

                    mesh.mesh.InstanceBuffer.Initialize(device, Graphics::BufferType::StructuredBuffer, nullptr, sizeof(DirectX::XMMATRIX) * numInstances, sizeof(DirectX::XMMATRIX));
					mesh.dirty = true;


                }


            }

            else if (mesh.shapeType == ShapeType::Null)
            {

                if (not mesh.dirty)
                {

                    std::cout << "Cube mesh created and buffers initialized." << std::endl;
                    std::cout << "Drawing mesh index ptr: " << mesh.mesh.indexBuffer.GetBuffer() << std::endl;
                    std::cout << "idx count: " << mesh.Indices.size() << std::endl;
                    std::cout << "vertex count: " << mesh.Vertices.size() << std::endl;

                    size_t vertexDataSize = mesh.Vertices.size() * sizeof(Graphics::VertexPositionColor);
                    mesh.mesh.vertexBuffer.Initialize(device, Graphics::BufferType::VertexBuffer, mesh.Vertices.data(), vertexDataSize, sizeof(Graphics::VertexPositionColor));

                    size_t indexDataSize = mesh.Indices.size() * sizeof(uint32_t);
                    mesh.mesh.indexBuffer.Initialize(device, Graphics::BufferType::IndexBuffer, mesh.Indices.data(), indexDataSize);
                    mesh.mesh.InstanceBuffer.Initialize(device, Graphics::BufferType::StructuredBuffer, nullptr, sizeof(DirectX::XMMATRIX) * numInstances, sizeof(DirectX::XMMATRIX));
                    mesh.mesh.indexCount = mesh.Indices.size();
					mesh.dirty = true;

                }
			}

			// Update instance buffer
			if (numInstances > 0) // Avoid updating with zero instances
                commandList.UpdateBuffer(mesh.mesh.InstanceBuffer, wordInstancing.data(), sizeof(DirectX::XMMATRIX) * numInstances);


        }



    }

    void OnUpdate(entt::registry& registry, GameTime time)
    {

        Update(registry, time);
        Loop(registry);
    }


    void OnShutdown()
    {

        //indexBuffer.Release();
        //vertexBuffer.Release();
        //constantBuffer.Release();
        //instanceBuffer.Release();



        pipeline.Release();
        commandList.Release();
        swapChain.Release();
        device.Release();
    }

};
