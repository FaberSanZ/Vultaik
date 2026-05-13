#pragma once

#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

#include <DirectXMath.h>
#include <imgui.h>

#include "Components.h"
#include "GameTime.h"
#include "GameWindows.h"
#include "RenderingDevice.h"
#include "PhysicsSystem.h"
#include "TerrainSystem.h"
#include "entt.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")


class RenderSystem
{
public:
    uint32_t m_Width{};
    uint32_t m_Height{};

    Render render{};
    Mesh cube{};
    Mesh sphere{};
    Mesh plane{};
	Mesh terrainMesh{};

    TerrainChunk testChunk;
    Mesh cellHighlightMesh{};
    Mesh cellHoverMesh{};
    Mesh blockedCellMesh{};

    bool terrainMouseHit = false;
    DirectX::XMFLOAT3 terrainMouseWorld = { 0.0f, 0.0f, 0.0f };

    bool hasHoveredCell = false;
    uint32_t hoveredCellX = 0;
    uint32_t hoveredCellZ = 0;




    void OnInitialize(entt::registry& registry, HWND hwnd, uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;
        m_Registry = &registry;

        render.Initialize(hwnd, m_Width, m_Height);

        cube = GenerateCubeMesh(1.0f);
        sphere = GenerateSphereMesh(1.0f, 30, 36);
        plane = GeneratePlaneMesh(1.0f);

        cube.debugName = "Cube";
        sphere.debugName = "Sphere";
        plane.debugName = "Plane";

        if (render.GetTextureCount() > 1)
            spawnTextureId = 1;


		terrainMesh = BuildMeshFromTerrain();
        cellHighlightMesh = GenerateSphereMesh(0.3f, 12, 36);
        cellHighlightMesh.debugName = "Cell Highlight";


        cellHighlightMesh = GenerateSphereMesh(0.3f, 12, 36);
        cellHighlightMesh.debugName = "Cell Highlight";

        cellHoverMesh = GenerateSphereMesh(0.18f, 12, 24);
        cellHoverMesh.debugName = "Cell Hover";

        blockedCellMesh = GenerateCubeMesh(1.0f);
        blockedCellMesh.debugName = "Blocked Cells";


    }

    void OnUpdate(entt::registry& registry, PhysicsSystem& physicsSystem, const GameTime& time)
    {
        UpdateCamera();
        render.UpdateFrameStats(time.DeltaTime());
        Loop(registry, physicsSystem, time);
    }


    entt::registry* m_Registry = nullptr;
    float cameraDistance = 25.0f;
    float cameraYaw = 0.0f;
    float cameraPitch = 0.0f;
    DirectX::XMFLOAT3 spawnPosition = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 spawnScale = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 spawnRotation = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 spawnColor = { 0.80f, 0.80f, 0.80f };
    float spawnMetallic = 0.1f;
    float spawnRoughness = 0.5f;
    int spawnTextureId = 0;
    entt::entity selectedEntity = entt::null;


    Mesh GeneratePlaneMesh(float size)
    {
        const float half = size * 0.5f;
        std::vector<Vertex> vertices =
        {
            { { -half, 0.0f, -half }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
            { { -half, 0.0f,  half }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  half, 0.0f,  half }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  half, 0.0f, -half }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
        };

        std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };
        return render.CreateMesh(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size());
    }

    Mesh GenerateCubeMesh(float size)
    {
        const float h = size * 0.5f;
        std::vector<Vertex> vertices =
        {
            { { -h, -h, -h }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
            { {  h, -h, -h }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
            { {  h,  h, -h }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
            { { -h,  h, -h }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },

            { { -h, -h,  h }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
            { {  h, -h,  h }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
            { {  h,  h,  h }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
            { { -h,  h,  h }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

            { { -h, -h, -h }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
            { { -h,  h, -h }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
            { { -h,  h,  h }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
            { { -h, -h,  h }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },

            { {  h, -h, -h }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
            { {  h,  h, -h }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
            { {  h,  h,  h }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { {  h, -h,  h }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },

            { { -h,  h, -h }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  h,  h, -h }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  h,  h,  h }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { { -h,  h,  h }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },

            { { -h, -h, -h }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
            { {  h, -h, -h }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
            { {  h, -h,  h }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
            { { -h, -h,  h }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
        };

        std::vector<uint32_t> indices =
        {
            0,1,2, 0,2,3,
            4,5,6, 4,6,7,
            8,9,10, 8,10,11,
            12,13,14, 12,14,15,
            16,17,18, 16,18,19,
            20,21,22, 20,22,23
        };

        return render.CreateMesh(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size());
    }

    Mesh GenerateSphereMesh(float radius, uint32_t stackCount, uint32_t sliceCount)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t stack = 0; stack <= stackCount; ++stack)
        {
            float v = (float)stack / (float)stackCount;
            float phi = v * DirectX::XM_PI;

            for (uint32_t slice = 0; slice <= sliceCount; ++slice)
            {
                float u = (float)slice / (float)sliceCount;
                float theta = u * DirectX::XM_2PI;

                float x = std::sin(phi) * std::cos(theta);
                float y = std::cos(phi);
                float z = std::sin(phi) * std::sin(theta);

                float uTex = (float)slice / (float)sliceCount;
                float vTex = (float)stack / (float)stackCount;
                vertices.push_back({ { x * radius, y * radius, z * radius }, { uTex, vTex }, { x, y, z } });
            }
        }

        const uint32_t stride = sliceCount + 1;
        for (uint32_t stack = 0; stack < stackCount; ++stack)
        {
            for (uint32_t slice = 0; slice < sliceCount; ++slice)
            {
                uint32_t a = stack * stride + slice;
                uint32_t b = a + stride;
                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(a + 1);
                indices.push_back(a + 1);
                indices.push_back(b);
                indices.push_back(b + 1);
            }
        }

        return render.CreateMesh(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size());
    }

    Mesh BuildMeshFromTerrain()
    {
        testChunk.Initialize(0, 0, 8, 1.0f, { -8.0f, 0.0f, -8.0f });


        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;


        return render.CreateMesh((Vertex*)testChunk.GetVertices().data(), static_cast<uint32_t>(testChunk.GetVertices().size()), (uint32_t*)testChunk.GetIndices().data(), static_cast<uint32_t>(testChunk.GetIndices().size()));
    
	}


    void UpdateRenderInstance()
    {
        DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();

        DirectX::XMFLOAT4X4 worldMatrix{};
        DirectX::XMStoreFloat4x4(&worldMatrix, world);

        InstanceData instance{};
        instance.worldMatrix = worldMatrix;

        // Verde terreno
        instance.baseColor = { 0.25f, 0.55f, 0.22f, 1.0f };

        // material.x = metallic
        // material.y = roughness
        // material.z = ao
        // material.w = textureId
        instance.material = { 0.0f, 0.85f, 1.0f, 1.0f };

        render.UpdateInstanceBuffer(terrainMesh, &instance, 1);
    }


    void UpdateCellHighlightInstance()
    {
        if (!testChunk.HasSelectedCell())
        {
            render.UpdateInstanceBuffer(cellHighlightMesh, nullptr, 0);
            return;
        }

        const uint32_t cellX = testChunk.GetSelectedCellX();
        const uint32_t cellZ = testChunk.GetSelectedCellZ();

        DirectX::XMFLOAT3 center = testChunk.GetCellCenter(cellX, cellZ);

        center.y += 0.03f;

        const float scale = testChunk.GetCellSize() * 0.92f;

        DirectX::XMMATRIX world = DirectX::XMMatrixScaling(scale, 1.0f, scale) * DirectX::XMMatrixTranslation(center.x, center.y, center.z);

        DirectX::XMFLOAT4X4 worldMatrix{};
        DirectX::XMStoreFloat4x4(&worldMatrix, world);

        InstanceData instance{};
        instance.worldMatrix = worldMatrix;

        instance.baseColor = { 0.95f, 0.85f, 0.15f, 1.0f };

        instance.material = { 0.0f, 0.35f, 1.0f, 2.0f };

        render.UpdateInstanceBuffer(cellHighlightMesh, &instance, 1);
    }


    bool ScreenPointToTerrain(int mouseX, int mouseY, DirectX::XMFLOAT3& outWorldPosition) const
    {
        if (m_Width == 0 || m_Height == 0)
            return false;

        const float ndcX = (2.0f * static_cast<float>(mouseX) / static_cast<float>(m_Width)) - 1.0f;
        const float ndcY = 1.0f - (2.0f * static_cast<float>(mouseY) / static_cast<float>(m_Height));

        DirectX::XMVECTOR nearPoint = DirectX::XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
        DirectX::XMVECTOR farPoint = DirectX::XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);

        DirectX::XMMATRIX viewProj = BuildViewProjection();

        DirectX::XMVECTOR determinant{};
        DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(&determinant, viewProj);

        DirectX::XMVECTOR nearWorld = DirectX::XMVector3TransformCoord(nearPoint, invViewProj);
        DirectX::XMVECTOR farWorld = DirectX::XMVector3TransformCoord(farPoint, invViewProj);

        DirectX::XMVECTOR rayOrigin = nearWorld;
        DirectX::XMVECTOR rayDirection = DirectX::XMVector3Normalize(
            DirectX::XMVectorSubtract(farWorld, nearWorld)
        );

        const float originY = DirectX::XMVectorGetY(rayOrigin);
        const float dirY = DirectX::XMVectorGetY(rayDirection);

        if (std::abs(dirY) < 0.00001f)
            return false;

        const float terrainPlaneY = testChunk.GetWorldPosition().y;
        const float t = (terrainPlaneY - originY) / dirY;

        if (t < 0.0f)
            return false;

        DirectX::XMVECTOR hitPoint = DirectX::XMVectorAdd(
            rayOrigin,
            DirectX::XMVectorScale(rayDirection, t)
        );

        DirectX::XMStoreFloat3(&outWorldPosition, hitPoint);
        return true;
    }


    void UpdateCellHoverInstance()
    {
        if (!hasHoveredCell)
        {
            render.UpdateInstanceBuffer(cellHoverMesh, nullptr, 0);
            return;
        }

        if (testChunk.HasSelectedCell())
        {
            if (hoveredCellX == testChunk.GetSelectedCellX() &&
                hoveredCellZ == testChunk.GetSelectedCellZ())
            {
                render.UpdateInstanceBuffer(cellHoverMesh, nullptr, 0);
                return;
            }
        }

        DirectX::XMFLOAT3 center = testChunk.GetCellCenter(hoveredCellX, hoveredCellZ);

        center.y += 0.08f;

        DirectX::XMMATRIX world =
            DirectX::XMMatrixTranslation(center.x, center.y, center.z);

        DirectX::XMFLOAT4X4 worldMatrix{};
        DirectX::XMStoreFloat4x4(&worldMatrix, world);

        InstanceData instance{};
        instance.worldMatrix = worldMatrix;

        // Azul suave para hover.
        instance.baseColor = { 0.20f, 0.55f, 1.0f, 1.0f };

        // metallic, roughness, ao, textureId
        instance.material = { 0.0f, 0.30f, 1.0f, 0.0f };

        render.UpdateInstanceBuffer(cellHoverMesh, &instance, 1);
    }

    void UpdateTerrainMousePicking()
    {
        terrainMouseHit = false;
        hasHoveredCell = false;

        ImGuiIO& io = ImGui::GetIO();

        if (io.WantCaptureMouse)
            return;

        DirectX::XMFLOAT3 hitPosition{};

        if (!ScreenPointToTerrain(GameInput::GetMouseX(), GameInput::GetMouseY(), hitPosition))
            return;

        terrainMouseWorld = hitPosition;
        terrainMouseHit = true;

        uint32_t cellX = 0;
        uint32_t cellZ = 0;

        if (!testChunk.WorldToCell(hitPosition.x, hitPosition.z, cellX, cellZ))
            return;

        hoveredCellX = cellX;
        hoveredCellZ = cellZ;
        hasHoveredCell = true;

        if (GameInput::IsMouseButtonPressed(GameInput::MouseButton::Left))
        {
            testChunk.SelectCell(cellX, cellZ);
        }

        if (GameInput::IsMouseButtonPressed(GameInput::MouseButton::Right))
        {
            testChunk.ToggleCellWalkable(cellX, cellZ);
        }
    }


    void UpdateBlockedCellInstances()
    {
        std::vector<InstanceData> instances;
        instances.reserve(testChunk.GetCellCount());

        for (uint32_t z = 0; z < testChunk.GetCellsPerSide(); ++z)
        {
            for (uint32_t x = 0; x < testChunk.GetCellsPerSide(); ++x)
            {
                if (testChunk.IsCellWalkable(x, z))
                    continue;

                DirectX::XMFLOAT3 center = testChunk.GetCellCenter(x, z);

                center.y += 0.15f;

                const float scale = testChunk.GetCellSize() * 0.55f;

                DirectX::XMMATRIX world =
                    DirectX::XMMatrixScaling(scale, 0.25f, scale) *
                    DirectX::XMMatrixTranslation(center.x, center.y, center.z);

                DirectX::XMFLOAT4X4 worldMatrix{};
                DirectX::XMStoreFloat4x4(&worldMatrix, world);

                InstanceData instance{};
                instance.worldMatrix = worldMatrix;

                // Rojo/naranja para celda bloqueada.
                instance.baseColor = { 0.85f, 0.18f, 0.08f, 1.0f };

                // metallic, roughness, ao, textureId
                instance.material = { 0.0f, 0.65f, 1.0f, 0.0f };

                instances.push_back(instance);
            }
        }

        render.UpdateInstanceBuffer(
            blockedCellMesh,
            instances.empty() ? nullptr : instances.data(),
            static_cast<uint32_t>(instances.size())
        );
    }




    void ResetScene(entt::registry& registry)
    {
        registry.clear();
        if (render.GetTextureCount() > 1)
            spawnTextureId = 1;
    }

    DirectX::XMMATRIX BuildViewProjection() const
    {
        const float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
        const float clampedPitch = cameraPitch < -1.25f ? -1.25f : (cameraPitch > 1.25f ? 1.25f : cameraPitch);
        const float clampedDistance = cameraDistance > 2.5f ? cameraDistance : 2.5f;

        DirectX::XMVECTOR eye = BuildCameraEye(clampedDistance, clampedPitch);

        DirectX::XMVECTOR target = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eye, target, up);
        DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspect, 0.1f, 100.0f);
        return view * proj;
    }

    DirectX::XMVECTOR BuildCameraEye(float distance, float pitch) const
    {
        return DirectX::XMVectorSet(
            std::cos(cameraYaw) * std::cos(pitch) * distance,
            std::sin(pitch) * distance + 0.75f,
            std::sin(cameraYaw) * std::cos(pitch) * distance,
            1.0f);
    }

    DirectX::XMMATRIX BuildWorldMatrix(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& scale) const
    {
        DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
        DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
        DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
        return scaleMatrix * rotationMatrix * translation;
    }

    void UpdateCamera()
    {
        const float clampedPitch = cameraPitch < -1.25f ? -1.25f : (cameraPitch > 1.25f ? 1.25f : cameraPitch);
        const float clampedDistance = cameraDistance > 2.5f ? cameraDistance : 2.5f;
        DirectX::XMVECTOR eye = BuildCameraEye(clampedDistance, clampedPitch);
        DirectX::XMFLOAT4 cameraPosition;
        DirectX::XMStoreFloat4(&cameraPosition, eye);
        render.SetGameCamera(BuildViewProjection(), cameraPosition, { -0.35f, -1.0f, -0.25f, 0.0f });
    }

    void UpdateMeshes(entt::registry& registry, GameTime time)
    {
        auto& renderStats = render.GetRenderStats();
        renderStats.shapeInstanceCounts.fill(0);
        renderStats.objectCount = 0;

        std::vector<InstanceData> cubeInstances;
        std::vector<InstanceData> sphereInstances;
        std::vector<InstanceData> planeInstances;

        auto view_mesh = registry.view<MeshComponent>();

        for (auto [entity, mesh] : view_mesh.each())
        {
            DirectX::XMFLOAT4 baseColor = { 0.75f, 0.75f, 0.75f, 1.0f };
            DirectX::XMFLOAT4 material = { 0.0f, 0.65f, 1.0f, 0.0f };

            if (registry.all_of<MaterialComponent>(entity))
            {
                auto& mat = registry.get<MaterialComponent>(entity);
                const uint32_t textureCount = render.GetTextureCount();
                if (textureCount > 0)
                {
                    mat.textureId = std::clamp(mat.textureId, 0, static_cast<int>(textureCount) - 1);
                }
                else
                {
                    mat.textureId = 0;
                }
                baseColor = { mat.baseColor.x, mat.baseColor.y, mat.baseColor.z, 1.0f };
                material = { mat.metallic, mat.roughness, mat.ao, static_cast<float>(mat.textureId) };
            }

            if (registry.all_of<TransformComponent>(entity))
            {
                auto& transform = registry.get<TransformComponent>(entity);
                DirectX::XMMATRIX world{};
                world = BuildWorldMatrix(transform.position, transform.rotation, transform.scale);

                //if (registry.all_of<PhysicsBodyComponent>(entity))
                //{
                //    auto& body = registry.get<PhysicsBodyComponent>(entity);
                //    world = BuildWorldMatrixQuat(transform.position, body.orientation, transform.scale);
                //}

                DirectX::XMFLOAT4X4 worldMatrix{};
                DirectX::XMStoreFloat4x4(&worldMatrix, world);

                switch (mesh.shapeType)
                {
                    case ShapeType::Plane:
                        planeInstances.push_back({ worldMatrix, baseColor, material });
						break;

                    case ShapeType::Cube:
                        cubeInstances.push_back({ worldMatrix, baseColor, material });
                        break;


                    case ShapeType::Sphere:
                        sphereInstances.push_back({ worldMatrix, baseColor, material });
                        break;

                default:
                    break;
                }

            }
        }

        render.UpdateInstanceBuffer(cube, cubeInstances.empty() ? nullptr : cubeInstances.data(), static_cast<uint32_t>(cubeInstances.size()));
        render.UpdateInstanceBuffer(sphere, sphereInstances.empty() ? nullptr : sphereInstances.data(), static_cast<uint32_t>(sphereInstances.size()));
        render.UpdateInstanceBuffer(plane, planeInstances.empty() ? nullptr : planeInstances.data(), static_cast<uint32_t>(planeInstances.size()));

        UpdateRenderInstance();
        UpdateCellHighlightInstance();
        UpdateCellHoverInstance();
        UpdateBlockedCellInstances();
    }



    DirectX::XMMATRIX BuildWorldMatrixQuat(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& scale) const
    {

        DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&orientation));
        DirectX::XMMATRIX scaling = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

        return scaling * rotation * translation;
    }





    void BuildImGui(entt::registry& registry)
    {
        ImGui::TextUnformatted("3D scene");
        ImGui::SliderFloat("Camera distance", &cameraDistance, 2.5f, 14.0f, "%.1f");
        ImGui::SliderFloat("Camera yaw", &cameraYaw, -DirectX::XM_PI, DirectX::XM_PI, "%.2f");
        ImGui::SliderFloat("Camera pitch", &cameraPitch, -1.25f, 1.25f, "%.2f");


        ImGui::Separator();
        ImGui::TextUnformatted("Terrain Cells");

        ImGui::Text("Cell count: %u", testChunk.GetCellCount());

        static int debugCellX = 0;
        static int debugCellZ = 0;

        ImGui::InputInt("Cell X", &debugCellX);
        ImGui::InputInt("Cell Z", &debugCellZ);

        debugCellX = std::clamp(debugCellX, 0, static_cast<int>(testChunk.GetCellsPerSide()) - 1);
        debugCellZ = std::clamp(debugCellZ, 0, static_cast<int>(testChunk.GetCellsPerSide()) - 1);

        if (ImGui::Button("Select Cell"))
        {
            testChunk.SelectCell(
                static_cast<uint32_t>(debugCellX),
                static_cast<uint32_t>(debugCellZ)
            );
        }

        if (ImGui::Button("Clear Cell Selection"))
        {
            testChunk.ClearSelection();
        }

        if (testChunk.HasSelectedCell())
        {
            const uint32_t selectedX = testChunk.GetSelectedCellX();
            const uint32_t selectedZ = testChunk.GetSelectedCellZ();

            const DirectX::XMFLOAT3 center = testChunk.GetCellCenter(selectedX, selectedZ);

            ImGui::Text("Selected cell: %u, %u", selectedX, selectedZ);
            ImGui::Text("Center: %.2f, %.2f, %.2f", center.x, center.y, center.z);
        }
        else
        {
            ImGui::TextUnformatted("Selected cell: none");
        }



        ImGui::Separator();
        ImGui::TextUnformatted("Terrain Mouse Picking");

        ImGui::Text("Mouse: %d, %d", GameInput::GetMouseX(), GameInput::GetMouseY());

        if (terrainMouseHit)
        {
            ImGui::Text(
                "World hit: %.2f, %.2f, %.2f",
                terrainMouseWorld.x,
                terrainMouseWorld.y,
                terrainMouseWorld.z
            );
        }
        else
        {
            ImGui::TextUnformatted("World hit: none");
        }

        if (hasHoveredCell)
        {
            ImGui::Text("Hovered cell: %u, %u", hoveredCellX, hoveredCellZ);
        }
        else
        {
            ImGui::TextUnformatted("Hovered cell: none");
        }

        if (testChunk.HasSelectedCell())
        {
            ImGui::Text(
                "Selected cell: %u, %u",
                testChunk.GetSelectedCellX(),
                testChunk.GetSelectedCellZ()
            );
        }
        else
        {
            ImGui::TextUnformatted("Selected cell: none");
        }

    }

    void Loop(entt::registry& registry, PhysicsSystem& physicsSystem, GameTime time)
    {
        render.BeginImGuiFrame();

        BuildImGui(registry);
        physicsSystem.OnImGui(registry);
        UpdateTerrainMousePicking();
        UpdateMeshes(registry, time);

        render.Reset();
        render.BeginFrame();
        render.Clear();

        if (render.BeginGame())
        {
            cube.Draw(render.commandList);
            sphere.Draw(render.commandList);
            plane.Draw(render.commandList);
			terrainMesh.Draw(render.commandList);
			cellHighlightMesh.Draw(render.commandList);
            cellHoverMesh.Draw(render.commandList);
            blockedCellMesh.Draw(render.commandList);
        }

        render.RenderImGui();
        render.Loop();
    }




    void OnShutdown()
    {
        cellHoverMesh.Destroy();
        cellHighlightMesh.Destroy();
        terrainMesh.Destroy();
		blockedCellMesh.Destroy();

        cube.Destroy();
        sphere.Destroy();
        plane.Destroy();

        render.Cleanup();
    }
};

class MyGame
{
public:
    void Run()
    {

        GameTime::Config config;

        config.fixedDeltaTime = 1.0 / 60.0;
        config.maxDeltaTime = 0.25;
        config.maxPhysicsStepsPerFrame = 8;
        config.timeScale = 1.0;
        config.clearPhysicsAccumulatorOnReset = true;

        gameTime.Reset();
        gameTime.SetConfig(config);

        gameWindow.OnInitialize();

        physicsSystem.OnInitialize(registry);
        renderSystem.OnInitialize(registry, gameWindow.GetHandle(), gameWindow.GetClientWidth(), gameWindow.GetClientHeight());
        Update();

        renderSystem.OnShutdown();
        gameWindow.OnShutdown();
    }

private:
    void Scene3()
    {

    }

    void Update()
    {
        while (gameWindow.IsRunning())
        {
            gameTime.OnUpdate();
            while (gameTime.UpdatePhysics())
            {
                //const double fixedDt = gameTime.FixedDeltaTime();

                physicsSystem.OnUpdate(registry, gameTime);
            }
            renderSystem.OnUpdate(registry, physicsSystem, gameTime);

            gameWindow.PumpMessages();
        }
    }

    GameWindows gameWindow;
    GameTime gameTime;

    RenderSystem renderSystem;
    PhysicsSystem physicsSystem;

    entt::registry registry;
};

int main()
{
    MyGame myGame;
    myGame.Run();
    return 0;
}
