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
    Mesh cube{};
    Mesh sphere{};
    Mesh plane{};

    void OnInitialize(entt::registry& registry, HWND hwnd, uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;
        m_Registry = &registry;

        render.Initialize(hwnd, m_Width, m_Height);

        cube = GenerateCubeMesh(1.0f);
        sphere = GenerateSphereMesh(1.0f, 20, 24);
        plane = GeneratePlaneMesh(1.0f);

        cube.debugName = "Cube";
        sphere.debugName = "Sphere";
        plane.debugName = "Plane";

        //Scene3(registry);
        if (render.GetTextureCount() > 1)
            spawnTextureId = 1;
    }

    void OnUpdate(entt::registry& registry, PhysicsSystem& physicsSystem, const GameTime& time)
    {
        UpdateCamera();
        render.UpdateFrameStats(time.DeltaTime());
        Loop(registry, physicsSystem, time);
    }

    void OnShutdown()
    {
        render.Cleanup();
    }

private:
    entt::registry* m_Registry = nullptr;
    float cameraDistance = 25.0f;
    float cameraYaw = 0.55f;
    float cameraPitch = 0.1f;
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
                DirectX::XMMATRIX world = BuildWorldMatrix(transform.position, transform.rotation, transform.scale);
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

                const uint32_t shapeIndex = static_cast<uint32_t>(mesh.shapeType);
                if (shapeIndex < renderStats.shapeInstanceCounts.size())
                    ++renderStats.shapeInstanceCounts[shapeIndex];
                ++renderStats.objectCount;
            }
        }

        render.UpdateInstanceBuffer(cube, cubeInstances.empty() ? nullptr : cubeInstances.data(), static_cast<uint32_t>(cubeInstances.size()));
        render.UpdateInstanceBuffer(sphere, sphereInstances.empty() ? nullptr : sphereInstances.data(), static_cast<uint32_t>(sphereInstances.size()));
        render.UpdateInstanceBuffer(plane, planeInstances.empty() ? nullptr : planeInstances.data(), static_cast<uint32_t>(planeInstances.size()));
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

        registry.emplace<MeshComponent>(entity, MeshComponent{ shape });

        registry.emplace<MaterialComponent>(entity, MaterialComponent{ baseColor, metallic, roughness, ao, textureId });

        PhysicsBodyComponent body{};
        body.type = PhysicsBodyType::Dynamic;
        body.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };

        registry.emplace<PhysicsBodyComponent>(entity, body);

        SphereColliderComponent sphereCollider{};
        sphereCollider.radius = 1.0f;
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
        for (auto [entity, mesh, transform] : selectableView.each())
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

        ImGui::Begin("Render Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        const RenderStats& stats = render.GetRenderStats();
        ImGui::Text("FPS: %.1f", stats.fps);
        ImGui::Text("Frame: %.3f ms", stats.frameTimeMs);
        ImGui::Separator();
        ImGui::Text("Objects: %u", stats.objectCount);
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
            drawShapeRow("Cube", counts[static_cast<uint32_t>(ShapeType::Cube)]);
            drawShapeRow("Sphere", counts[static_cast<uint32_t>(ShapeType::Sphere)]);
            drawShapeRow("Plane", counts[static_cast<uint32_t>(ShapeType::Plane)]);
            ImGui::EndTable();
        }

        ImGui::End();
    }

    const char* ShapeTypeName(ShapeType shape) const
    {
        switch (shape)
        {
        case ShapeType::Cube: return "Cube";
        case ShapeType::Sphere: return "Sphere";
        case ShapeType::Plane: return "Plane";
        default: return "Null";
        }
    }
    void Loop(entt::registry& registry, PhysicsSystem& physicsSystem, GameTime time)
    {
        render.BeginImGuiFrame();

        BuildImGui(registry);
        physicsSystem.OnImGui(registry);
        UpdateMeshes(registry, time);

        render.Reset();
        render.BeginFrame();
        render.Clear();

        if (render.BeginGame())
        {
            cube.Draw(render.commandList);
            sphere.Draw(render.commandList);
            plane.Draw(render.commandList);
        }

        render.RenderImGui();
        render.Loop();
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
