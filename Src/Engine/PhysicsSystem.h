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

class PhysicsSystem
{
public:
    void OnInitialize(entt::registry& registry)
    {
        TransformComponent transform{};
        transform.position = { 0.0f, 0.0f, 0.0f };
        transform.scale = { 1.0f, 1.0f, 1.0f };
        transform.rotation = { 0.0f, 0.0f, 0.0f };

		MeshComponent mesh{};
		mesh.shapeType = ShapeType::Sphere;

		MaterialComponent material{};
		material.baseColor = { 255.0f, 255.0f, 255.0f };
		material.metallic = 0.1f;
		material.roughness = 0.5f;
		material.ao = 1.0f;
		material.textureId = 1;

        PhysicsBodyComponent body{};
        body.type = PhysicsBodyType::Static;
        body.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };

        SphereColliderComponent sphereCollider{};
        sphereCollider.radius = 0.5f;
        sphereCollider.centerOfMassLocal = { 0.0f, 0.0f, 0.0f };


        auto entity = registry.create();
        registry.emplace<TransformComponent>(entity, transform);
        registry.emplace<MeshComponent>(entity, mesh);
        registry.emplace<MaterialComponent>(entity, material);
        registry.emplace<PhysicsBodyComponent>(entity, body);
        registry.emplace<SphereColliderComponent>(entity, sphereCollider);



        auto entity2 = registry.create();
		body.type = PhysicsBodyType::Static;
		transform.position = { 0.0f, -3.6f, 0.0f };
		transform.scale = { 6.0f, 6.0f, 6.0f };
        registry.emplace<TransformComponent>(entity2, transform);
        registry.emplace<MeshComponent>(entity2, mesh);
        registry.emplace<MaterialComponent>(entity2, material);
        registry.emplace<PhysicsBodyComponent>(entity2, body);
        registry.emplace<SphereColliderComponent>(entity2, sphereCollider);
    }

    void OnUpdate(entt::registry& registry, const GameTime& time)
    {
        auto view = registry.view<TransformComponent>();

        for (auto [entity, transform] : view.each())
        {
            auto& transform = registry.get<TransformComponent>(entity);
            auto& body = registry.get<PhysicsBodyComponent>(entity);
            auto& collider = registry.get<SphereColliderComponent>(entity);

            if (!body.enabled)
                continue;

			DirectX::XMFLOAT3 centerOfMassWorld = GameMath::GetCenterOfMassWorld(transform, body, collider);

            ApplyGravity(registry, time.FixedDeltaTime());
            IntegratePositions(registry, time.FixedDeltaTime());
        }

    }

    void OnImGui(entt::registry& registry)
    {
		//TODO: ImGui for physics system
	}



    void ApplyGravity(entt::registry& registry, float dt)
    {
        auto view = registry.view<PhysicsBodyComponent>();

        for (auto entity : view)
        {
            auto& body = view.get<PhysicsBodyComponent>(entity);

            if (!body.enabled)
                continue;

            if (body.type != PhysicsBodyType::Dynamic)
                continue;

            if (!body.useGravity)
                continue;

            // dv = a * dt
            const DirectX::XMFLOAT3 deltaVelocity =
                GameMath::Mul(gravity, dt);

            body.linearVelocity =
                GameMath::Add(body.linearVelocity, deltaVelocity);
        }
    }

    void IntegratePositions(entt::registry& registry, float dt)
    {
        auto view = registry.view<TransformComponent, PhysicsBodyComponent>();

        for (auto [entity, transform, body] : view.each())
        {
            if (!body.enabled)
                continue;

            if (body.type != PhysicsBodyType::Dynamic)
                continue;

            // dx = v * dt
            const DirectX::XMFLOAT3 deltaPosition = GameMath::Mul(body.linearVelocity, dt);

            transform.position = GameMath::Add(transform.position, deltaPosition);

        }
    }


	// TODO: move this to a config file or something
    DirectX::XMFLOAT3 gravity = { 0.0f, -1.0f, 0.0f };


};
