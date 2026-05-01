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
#include "entt.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")


class RenderSystem
{
public:
    uint32_t m_Width{};
    uint32_t m_Height{};

    Render render{};
    Mesh triangle{};
    Mesh cuad{};
    Mesh pentagon{};
    Mesh hexagon{};
    Mesh circle{};
    Mesh cube{};
    Mesh sphere{};
    Mesh plane{};
    std::vector<RaytracingObject> m_SceneObjects;

    void OnInitialize(entt::registry& registry, HWND hwnd, uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;
        m_Registry = &registry;

        render.Initialize(hwnd, m_Width, m_Height);

        triangle = GeneratePolygonMesh(0.65f, 3);
        cuad = GeneratePolygonMesh(0.65f, 4);
        pentagon = GeneratePolygonMesh(0.65f, 5);
        hexagon = GeneratePolygonMesh(0.65f, 6);
        circle = GeneratePolygonMesh(0.65f, 20);
        cube = GenerateCubeMesh(1.0f);
        sphere = GenerateSphereMesh(0.6f, 20, 24);
        plane = GeneratePlaneMesh(1.0f);

        triangle.debugName = "Triangle";
        cuad.debugName = "Quad";
        pentagon.debugName = "Pentagon";
        hexagon.debugName = "Hexagon";
        circle.debugName = "Circle";
        cube.debugName = "Cube";
        sphere.debugName = "Sphere";
        plane.debugName = "Plane";

        Scene3(registry);
        if (render.GetTextureCount() > 1)
            spawnTextureId = 1;
    }

    void OnUpdate(entt::registry& registry, PhysicsSystem& physicsSystem, GameTime time)
    {
        UpdateCamera();
        render.UpdateFrameStats(time.GetDeltaTime());
        Loop(registry, physicsSystem, time);
    }

    void OnShutdown()
    {
        render.Cleanup();
    }

private:
    entt::registry* m_Registry = nullptr;
    float cameraDistance = 6.0f;
    float cameraYaw = 0.55f;
    float cameraPitch = 0.25f;
    DirectX::XMFLOAT3 spawnPosition = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 spawnScale = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 spawnRotation = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 spawnColor = { 0.80f, 0.80f, 0.80f };
    float spawnMetallic = 0.1f;
    float spawnRoughness = 0.5f;
    int spawnTextureId = 0;
    entt::entity selectedEntity = entt::null;

    Mesh GeneratePolygonMesh(float radius, uint32_t segmentCount)
    {
        std::vector<Vertex> vertices;
        vertices.reserve(segmentCount + 1);

        vertices.push_back({ { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } });

        for (uint32_t i = 0; i <= segmentCount; ++i)
        {
            float angle = (2.0f * DirectX::XM_PI * i) / segmentCount;
            float x = radius * std::cos(angle);
            float y = radius * std::sin(angle);
            float u = (x / (radius * 2.0f)) + 0.5f;
            float v = (y / (radius * 2.0f)) + 0.5f;
            vertices.push_back({ { x, y, 0.0f }, { u, v }, { 0.0f, 0.0f, 1.0f } });
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

    void Scene3(entt::registry& registry)
    {
        SpawnShapeInternal(
            registry,
            ShapeType::Sphere,
            { 0.0f, -0.2f, 0.0f },
            { 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 0.0f },
            { 0.20f, 0.45f, 0.95f },
            0.9f,
            0.18f,
            1.0f,
            1);
    }

    void ResetScene(entt::registry& registry)
    {
        registry.clear();
        Scene3(registry);
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
        m_SceneObjects.clear();

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
                DirectX::XMMATRIX world = BuildWorldMatrix(transform.position, transform.rotation, transform.scale);
                DirectX::XMFLOAT4X4 worldMatrix{};
                DirectX::XMStoreFloat4x4(&worldMatrix, world);
                RaytracingObject object{};
                object.shapeType = mesh.shapeType;
                object.instance = InstanceData{ worldMatrix, baseColor, material };
                m_SceneObjects.push_back(object);
            }
        }
    }

    void PushInstance(
        ShapeType shape,
        const InstanceData& instance,
        std::vector<InstanceData>& triangleInstancing,
        std::vector<InstanceData>& cuadInstancing,
        std::vector<InstanceData>& pentagonInstancing,
        std::vector<InstanceData>& hexagonInstancing,
        std::vector<InstanceData>& circleInstancing,
        std::vector<InstanceData>& cubeInstancing,
        std::vector<InstanceData>& sphereInstancing,
        std::vector<InstanceData>& planeInstancing)
    {
        switch (shape)
        {
        case ShapeType::Triangle:
            triangleInstancing.push_back(instance);
            break;
        case ShapeType::Cuad:
            cuadInstancing.push_back(instance);
            break;
        case ShapeType::Pentagon:
            pentagonInstancing.push_back(instance);
            break;
        case ShapeType::Hexagon:
            hexagonInstancing.push_back(instance);
            break;
        case ShapeType::Circle:
            circleInstancing.push_back(instance);
            break;
        case ShapeType::Cube:
            cubeInstancing.push_back(instance);
            break;
        case ShapeType::Sphere:
            sphereInstancing.push_back(instance);
            break;
        case ShapeType::Plane:
            planeInstancing.push_back(instance);
            break;
        default:
            break;
        }
    }

    void SpawnShape(entt::registry& registry, ShapeType shape)
    {
        SpawnShapeInternal(
            registry,
            shape,
            spawnPosition,
            spawnScale,
            spawnRotation,
            spawnColor,
            spawnMetallic,
            spawnRoughness,
            1.0f,
            spawnTextureId);
    }

    void SpawnShapeInternal(
        entt::registry& registry,
        ShapeType shape,
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& scale,
        const DirectX::XMFLOAT3& rotation,
        const DirectX::XMFLOAT3& baseColor,
        float metallic,
        float roughness,
        float ao,
        int textureId)
    {
        auto entity = registry.create();

        TransformComponent transform{};
        transform.position = position;
        transform.scale = scale;
        transform.rotation = rotation;
        registry.emplace<TransformComponent>(entity, transform);

        registry.emplace<MeshComponent>(entity, MeshComponent{ shape, MeshType::Kinematic });

        registry.emplace<MaterialComponent>(entity, MaterialComponent{ baseColor, metallic, roughness, ao, textureId });

        // Física
        PhysicsBodyComponent body{};
        body.type = PhysicsBodyType::Dynamic;
        body.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };

        registry.emplace<PhysicsBodyComponent>(entity, body);

        SphereColliderComponent sphereCollider{};
        sphereCollider.radius = 0.5f;
        sphereCollider.centerOfMassLocal = { 0.0f, 0.0f, 0.0f };

        registry.emplace<SphereColliderComponent>(entity, sphereCollider);
    }

    void BuildImGui(entt::registry& registry)
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
        ImGui::Begin("Vultaik", nullptr, flags);

        auto textureLabel = [&](int index) -> std::string
        {
            std::string name = render.GetTextureName(static_cast<uint32_t>(index));
            if (name.empty())
                name = std::string("Texture ") + std::to_string(index);
            return name;
        };

        auto drawTextureCombo = [&](const char* label, int& textureId)
        {
            const int textureCount = static_cast<int>(render.GetTextureCount());
            if (textureCount <= 0)
            {
                ImGui::InputInt(label, &textureId);
                return;
            }

            textureId = std::clamp(textureId, 0, textureCount - 1);
            std::string preview = textureLabel(textureId);
            if (ImGui::BeginCombo(label, preview.c_str()))
            {
                for (int i = 0; i < textureCount; ++i)
                {
                    const bool isSelected = (textureId == i);
                    std::string itemLabel = textureLabel(i);
                    if (ImGui::Selectable(itemLabel.c_str(), isSelected))
                        textureId = i;
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        };

        ImGui::TextUnformatted("3D scene");
        ImGui::SliderFloat("Camera distance", &cameraDistance, 2.5f, 14.0f, "%.1f");
        ImGui::SliderFloat("Camera yaw", &cameraYaw, -DirectX::XM_PI, DirectX::XM_PI, "%.2f");
        ImGui::SliderFloat("Camera pitch", &cameraPitch, -1.25f, 1.25f, "%.2f");

        if (ImGui::Button("Reset scene", ImVec2(-1.0f, 0.0f)))
        {
            ResetScene(registry);
            selectedEntity = entt::null;
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Spawn");
        ImGui::DragFloat3("Position", &spawnPosition.x, 0.05f);
        ImGui::DragFloat3("Scale", &spawnScale.x, 0.05f, 0.1f, 10.0f);
        ImGui::DragFloat3("Rotation", &spawnRotation.x, 0.02f, -DirectX::XM_PI, DirectX::XM_PI);
        ImGui::SliderFloat("Metallic", &spawnMetallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &spawnRoughness, 0.02f, 1.0f);
        drawTextureCombo("Texture", spawnTextureId);
        ImGui::Text("Textures loaded: %u", render.GetTextureCount());

        if (ImGui::Button("Cube", ImVec2(-1.0f, 0.0f)))
            SpawnShape(registry, ShapeType::Cube);
        if (ImGui::Button("Sphere", ImVec2(-1.0f, 0.0f)))
            SpawnShape(registry, ShapeType::Sphere);
        if (ImGui::Button("Plane", ImVec2(-1.0f, 0.0f)))
            SpawnShape(registry, ShapeType::Plane);

        ImGui::Separator();
        ImGui::TextUnformatted("Selected entity");
        auto selectableView = registry.view<MeshComponent, TransformComponent>();
        std::vector<entt::entity> selectableEntities;
        selectableEntities.reserve(selectableView.size_hint());
        for (auto entity : selectableView)
            selectableEntities.push_back(entity);

        auto entityLabel = [&](entt::entity entity) -> std::string
        {
            const auto& mesh = registry.get<MeshComponent>(entity);
            return std::string("Entity ") + std::to_string(static_cast<int>(entity)) + " - " + ShapeTypeName(mesh.shapeType);
        };

        if (selectedEntity == entt::null || !registry.valid(selectedEntity) || !registry.all_of<MeshComponent, TransformComponent>(selectedEntity))
        {
            selectedEntity = selectableEntities.empty() ? entt::null : selectableEntities.front();
        }

        const char* currentLabel = "None";
        std::string currentLabelStorage;
        if (selectedEntity != entt::null && registry.valid(selectedEntity) && registry.all_of<MeshComponent, TransformComponent>(selectedEntity))
        {
            currentLabelStorage = entityLabel(selectedEntity);
            currentLabel = currentLabelStorage.c_str();
        }

        if (ImGui::BeginCombo("Active", currentLabel))
        {
            for (entt::entity entity : selectableEntities)
            {
                const bool isSelected = (entity == selectedEntity);
                std::string label = entityLabel(entity);
                if (ImGui::Selectable(label.c_str(), isSelected))
                    selectedEntity = entity;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (selectedEntity != entt::null && registry.valid(selectedEntity) && registry.all_of<MeshComponent, TransformComponent>(selectedEntity))
        {
            auto& transform = registry.get<TransformComponent>(selectedEntity);
            ImGui::DragFloat3("Entity Position", &transform.position.x, 0.05f);
            ImGui::DragFloat3("Entity Scale", &transform.scale.x, 0.05f, 0.1f, 10.0f);
            ImGui::DragFloat3("Entity Rotation", &transform.rotation.x, 0.02f, -DirectX::XM_PI, DirectX::XM_PI);

            if (registry.all_of<MaterialComponent>(selectedEntity))
            {
                auto& mat = registry.get<MaterialComponent>(selectedEntity);
                ImGui::ColorEdit3("Entity Color", &mat.baseColor.x);
                ImGui::SliderFloat("Entity Metallic", &mat.metallic, 0.0f, 1.0f);
                ImGui::SliderFloat("Entity Roughness", &mat.roughness, 0.02f, 1.0f);
                drawTextureCombo("Entity Texture", mat.textureId);
            }
        }

        ImGui::End();

        const RenderStats& stats = render.GetRenderStats();
        const std::vector<BLASStats>& blasStats = render.GetBLASStats();

        ImGui::Begin("Render Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("FPS: %.1f", stats.fps);
        ImGui::Text("Frame: %.3f ms", stats.frameTimeMs);
        ImGui::Separator();
        ImGui::Text("Objects: %u", stats.objectCount);
        ImGui::Text("BLAS count: %u", stats.blasCount);
        ImGui::Text("BLAS total: %.3f ms", stats.blasBuildTotalMs);
        ImGui::Text("BLAS build: %.3f ms", stats.blasBuildTotalMs);
        ImGui::Text("TLAS build: %.3f ms", stats.tlasBuildMs);
        ImGui::Text("Ray dispatch: %.3f ms", stats.rayDispatchMs);
        ImGui::Text("Upload: %.3f ms", stats.uploadMs);
        ImGui::Text("Present: %.3f ms", stats.presentMs);
        ImGui::Separator();

        ImGui::TextUnformatted("Scene instances");
        if (ImGui::BeginTable("SceneInstanceTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Shape");
            ImGui::TableSetupColumn("Instances");
            ImGui::TableHeadersRow();

            auto drawShapeRow = [&](const char* name, uint32_t count)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(name);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%u", count);
            };

            const auto& counts = stats.shapeInstanceCounts;
            drawShapeRow("Triangle", counts[static_cast<uint32_t>(ShapeType::Triangle)]);
            drawShapeRow("Quad", counts[static_cast<uint32_t>(ShapeType::Cuad)]);
            drawShapeRow("Pentagon", counts[static_cast<uint32_t>(ShapeType::Pentagon)]);
            drawShapeRow("Hexagon", counts[static_cast<uint32_t>(ShapeType::Hexagon)]);
            drawShapeRow("Circle", counts[static_cast<uint32_t>(ShapeType::Circle)]);
            drawShapeRow("Cube", counts[static_cast<uint32_t>(ShapeType::Cube)]);
            drawShapeRow("Sphere", counts[static_cast<uint32_t>(ShapeType::Sphere)]);
            drawShapeRow("Plane", counts[static_cast<uint32_t>(ShapeType::Plane)]);
            ImGui::EndTable();
        }

        ImGui::Separator();

        if (ImGui::BeginTable("BLASStatsTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Tris");
            ImGui::TableSetupColumn("Geom");
            ImGui::TableSetupColumn("Result KB");
            ImGui::TableSetupColumn("Scratch KB");
            ImGui::TableSetupColumn("Build ms");
            ImGui::TableSetupColumn("Flags");
            ImGui::TableHeadersRow();

            for (const auto& blas : blasStats)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(blas.name.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%u", blas.triangleCount);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%u", blas.geometryCount);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.1f", static_cast<double>(blas.resultSizeBytes) / 1024.0);
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.1f", static_cast<double>(blas.scratchSizeBytes) / 1024.0);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%.3f", blas.buildTimeMs);
                ImGui::TableSetColumnIndex(6);
                std::string flagsText;
                flagsText += blas.allowCompaction ? "C " : "";
                flagsText += blas.allowUpdate ? "U " : "";
                flagsText += blas.preferFastTrace ? "FT " : "";
                flagsText += blas.preferFastBuild ? "FB " : "";
                if (flagsText.empty())
                    flagsText = "-";
                ImGui::TextUnformatted(flagsText.c_str());
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }

    const char* ShapeTypeName(ShapeType shape) const
    {
        switch (shape)
        {
        case ShapeType::Triangle: return "Triangle";
        case ShapeType::Cuad: return "Quad";
        case ShapeType::Pentagon: return "Pentagon";
        case ShapeType::Hexagon: return "Hexagon";
        case ShapeType::Circle: return "Circle";
        case ShapeType::Cube: return "Cube";
        case ShapeType::Sphere: return "Sphere";
        case ShapeType::Plane: return "Plane";
        default: return "Null";
        }
    }
    void Loop(entt::registry& registry, PhysicsSystem& physicsSystem, GameTime time)
    {
        // 1. Crear frame ImGui en CPU.
        // Esto NO graba comandos DX12 todavía.
        render.BeginImGuiFrame();

        BuildImGui(registry);
        physicsSystem.OnImGui(registry);
        UpdateMeshes(registry, time);

        // 2. Render DXR.
        // IMPORTANTE:
        // No llamar render.Reset() aquí.
        // RenderRaytracedScene ya resetea y abre la command list.
        const bool rendered = render.RenderRaytracedScene(
            m_SceneObjects,
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
            // No llames render.RenderImGui() ni render.Loop()
            // si RenderRaytracedScene falló, porque puede que la command list
            // no esté abierta correctamente.
            return;
        }

        // 3. Dibujar ImGui encima del resultado DXR.
        render.RenderImGui();

        // 4. Present.
        render.Loop();
    }
};

class MyGame
{
public:
    void Run()
    {
        gameTime.OnInitialize();
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
            physicsSystem.OnUpdate(registry, gameTime);
            renderSystem.OnUpdate(registry, physicsSystem, gameTime);

            gameWindow.PumpMessages();
            gameTime.OnUpdate();
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
