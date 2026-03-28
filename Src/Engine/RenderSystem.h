#include <windows.h>
#include <fstream>
#include <iostream>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "GameWindows.h"
#include "GameTime.h"
#include "entt.hpp"
#include "Components.h"
#include "RenderingDevice.h"


class RenderSystem
{
private:

public:
    RenderSystem()
    {
    }

    uint32_t m_Width{ }; // Width of the render target
    uint32_t m_Height{ }; // Height of the render target
    Render render{};
	Mesh cuad = {};
	Mesh triangle = {};
	Mesh circle = {};

    void OnInitialize(entt::registry& registry, HWND hwnd,uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;

        render.Initialize(hwnd, m_Width, m_Height);

        Vertex vertices[] =
        {
            // Red Quad
            { -0.5f,  0.5f, 0.2f, 1.0f,},
            {  0.5f, -0.5f, 0.2f, 1.0f,   },
            { -0.5f, -0.5f, 0.2f, 1.0f,  },
            {  0.5f,  0.5f, 0.2f, 1.0f,  },

        };

        uint32_t indices[] =
        {
            0, 1, 2,
            0, 3, 1,
        };


        cuad = render.CreateMesh(vertices, 4, indices, 6);


		// Create a simple triangle mesh
        Vertex verticest[] =
        {
            {  0.0f,  0.2f, 0.1f, 1.0f,},
            {  0.2f, -0.2f, 0.1f, 1.0f,},
            { -0.2f, -0.2f, 0.1f, 1.0f, }
        };
        uint32_t indicest[] = { 0, 1, 2 };
		triangle = render.CreateMesh(verticest, 3, indicest, 3);

		GenerateCircleMesh(0.3f, 12);
        
    }

    void GenerateCircleMesh(float radius, uint32_t segmentCount)
    {
        std::vector<Vertex> vertices;
        vertices.reserve(segmentCount + 1);

        // Centro
        vertices.push_back({ 0.0f, 0.0f, 0.0f, 1.0f});

        for (uint32_t i = 0; i <= segmentCount; ++i)
        {
            float angle = (2.0f * 3.14159265f * i) / segmentCount;
            float x = radius * cos(angle);
            float y = radius * sin(angle);

            // Colores para depuración (arcoíris)
            float r = 0;
            float g = 0;

            vertices.push_back({ x, y, 0.0f, 1.0f});
        }

        std::vector<uint32_t> indices;
        indices.reserve(segmentCount * 3);

        for (uint32_t i = 0; i < segmentCount; ++i)
        {
            indices.push_back(0);
            indices.push_back(i + 2);
            indices.push_back(i + 1);
        }

        circle = render.CreateMesh(vertices.data(), (uint32_t)vertices.size(),
            indices.data(), (uint32_t)indices.size());
    }
	float speed = 1.0f;

    void OnUpdate(entt::registry& registry, GameTime time)
    {
        float totalTime = time.GetTotalTime();

        InstanceData data[8];

        // Instancia estática en el centro
        data[0].worldMatrix = DirectX::XMMatrixIdentity();
        data[0].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

        // Instancia en movimiento
        float radius = 1.0f;
        float angle = totalTime * 1.5f;
        float x = radius * cos(angle);
        float y = radius * sin(angle);

        DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();


        for(int i = 1; i < 7; ++i)
        {
            x = radius * cos(angle + float(i));
            y = radius * sin(angle + float(i));
            translation = DirectX::XMMatrixTranslation(x, y, 0.0f);

            data[i].worldMatrix = DirectX::XMMatrixTranspose(translation);
            data[i].color = DirectX::XMFLOAT4(x, 0.5f, y, 1.0f);
		}


        render.UpdateInstanceBuffer(circle, data, 8);

        Update(registry, time);
        Loop(registry);
    }

    void Loop(entt::registry& registry)
    {
		render.Reset(); // Reset the command allocator and command list for the current frame
		render.Clear(); // Clear the render target and depth/stencil buffer, and set them for rendering
		render.BeginFrame(); // Set the viewport, scissor rect, and pipeline state for the current frame

		//cuad.Draw(render.commandList); // Draw the mesh using the command list
		//triangle.Draw(render.commandList); // Draw the mesh using the command list 
		circle.Draw(render.commandList); // Draw the mesh using the command list

        render.Loop();
    }


    void Update(entt::registry& registry, GameTime time)
    {
    }

    void OnShutdown()
    {
        render.Cleanup();

    }

};


