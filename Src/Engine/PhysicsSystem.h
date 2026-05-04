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
#include "GameMath.h"
#include "entt.hpp"
#include <array>


// TODO: move this to a collision system or something
struct Contact
{
    entt::entity entityA = entt::null;
    entt::entity entityB = entt::null;

    DirectX::XMFLOAT3 pointOnAWorld = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 pointOnBWorld = { 0.0f, 0.0f, 0.0f };

    DirectX::XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };

    float separationDistance = 0.0f;
};


struct CollisionPair
{
    entt::entity entityA = entt::null;
    entt::entity entityB = entt::null;
};

class PhysicsSystem
{
public:

    void OnInitialize(entt::registry& registry)
    {

        {
            auto entity = registry.create();

            TransformComponent transform{};
            transform.position = { 0.0f, 0.0f, 0.0f };
            transform.scale = { 1.0f, 1.0f, 1.0f };
            transform.rotation = { 0.0f, 0.0f, 0.0f };

            MeshComponent mesh{};
            mesh.shapeType = ShapeType::Sphere;

            MaterialComponent material{};
            material.baseColor = { 255.0f, 80.0f, 80.0f };
            material.metallic = 0.1f;
            material.roughness = 0.5f;
            material.ao = 1.0f;
            material.textureId = 1;

            registry.emplace<TransformComponent>(entity, transform);
			registry.emplace<MeshComponent>(entity, mesh);
			registry.emplace<MaterialComponent>(entity, material);
        }
    }

    void OnUpdate(entt::registry& registry, const GameTime& time)
    {
        const float dt = time.FixedDeltaTime();

    }

    void OnImGui(entt::registry& registry)
    {



    }


    


};
