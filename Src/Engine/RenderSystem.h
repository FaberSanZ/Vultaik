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
	Mesh triangle = {};
	Mesh cuad = {};
	Mesh pentagon = {};
	Mesh hexagon = {};
	Mesh circle = {};

    void OnInitialize(entt::registry& registry, HWND hwnd,uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;

        render.Initialize(hwnd, m_Width, m_Height);


		triangle = GeneratePolygonMesh(0.5f, 3);
		cuad = GeneratePolygonMesh(0.5f, 4);
		pentagon = GeneratePolygonMesh(0.5f, 5);
		hexagon = GeneratePolygonMesh(0.5f, 6);
		circle = GeneratePolygonMesh(0.5f, 12);


        
        
    }

    Mesh GeneratePolygonMesh(float radius, uint32_t segmentCount)
    {
        std::vector<Vertex> vertices;
        vertices.reserve(segmentCount + 1);

        //vertices.push_back({ 0.0f, 0.0f, 0.0f, 1.0f});

        for (uint32_t i = 0; i <= segmentCount; ++i)
        {
            float angle = (2.0f * 3.14159265f * i) / segmentCount;
            float x = radius * cos(angle);
            float y = radius * sin(angle);

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

        return render.CreateMesh(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size());
        
    }



	float speed = 1.0f;

    void OnUpdate(entt::registry& registry, GameTime time)
    {
		Update(registry, time);
        Loop(registry);
    }

    void Loop(entt::registry& registry)
    {
		render.Reset(); // Reset the command allocator and command list for the current frame
		render.Clear(); // Clear the render target and depth/stencil buffer, and set them for rendering
		render.BeginFrame(); // Set the viewport, scissor rect, and pipeline state for the current frame

		triangle.Draw(render.commandList); // Draw the mesh using the command list 
		cuad.Draw(render.commandList); // Draw the mesh using the command list
		pentagon.Draw(render.commandList); // Draw the mesh using the command list
		hexagon.Draw(render.commandList); // Draw the mesh using the command list
		circle.Draw(render.commandList); // Draw the mesh using the command list

        render.Loop();
    }


    void Update(entt::registry& registry, GameTime time)
    {
        std::vector<InstanceData> triangleInstancing;
        std::vector<InstanceData> cuadInstancing;
        std::vector<InstanceData> pentagonInstancing;
		std::vector<InstanceData> hexagonInstancing;
        std::vector<InstanceData> circleInstancing;
        std::vector<InstanceData> polygonInstancing;

        DirectX::XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

        auto view_mesh = registry.view<MeshComponent>();

        for (auto [entity, mesh] : view_mesh.each())
        {

            if (mesh.meshType == MeshType::Static)
                color = { 0.4f, 0.7f, 0.3f, 1.0f };

            else if (mesh.meshType == MeshType::Dynamic)
                color = { 0.25f, 0.45f, 0.75f, 1.0f };

            else if (mesh.meshType == MeshType::Kinematic)
                color = { 0.9f, 0.6f, 0.2f, 1.0f };


            if (registry.all_of<TransformComponent>(entity))
            {
                auto& trasform = registry.get<TransformComponent>(entity);

                if (mesh.shapeType == ShapeType::Triangle)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    triangleInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Cuad)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    cuadInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Pentagon)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    pentagonInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Hexagon)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    hexagonInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }

                if (mesh.shapeType == ShapeType::Circle)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    circleInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }
            }
            else if (registry.all_of<ParticleComponent>(entity))
            {
                auto& trasform = registry.get<ParticleComponent>(entity);

                if (mesh.shapeType == ShapeType::Triangle)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    triangleInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Cuad)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    cuadInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Pentagon)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    pentagonInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Hexagon)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    hexagonInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }

                if (mesh.shapeType == ShapeType::Circle)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    circleInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }
            }


            







        }


        if(triangleInstancing.size() > 0)
			render.UpdateInstanceBuffer(triangle, triangleInstancing.data(), triangleInstancing.size());

        if (cuadInstancing.size() > 0)
            render.UpdateInstanceBuffer(cuad, cuadInstancing.data(), cuadInstancing.size());

		if (pentagonInstancing.size() > 0)
			render.UpdateInstanceBuffer(pentagon, pentagonInstancing.data(), pentagonInstancing.size());

        if(hexagonInstancing.size() > 0)
			render.UpdateInstanceBuffer(hexagon, hexagonInstancing.data(), hexagonInstancing.size());

        if (circleInstancing.size() > 0)
            render.UpdateInstanceBuffer(circle, circleInstancing.data(), circleInstancing.size());

		//if (polygonInstancing.size() > 0)
		//	render.UpdateInstanceBuffer(triangle, polygonInstancing.data(), polygonInstancing.size());



    }

    void OnShutdown()
    {
        render.Cleanup();
    }

};


