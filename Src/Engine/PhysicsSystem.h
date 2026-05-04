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


class PhysicsSystem
{
public:

    void OnInitialize(entt::registry& registry)
    {

        {
            auto entity = registry.create();

            TransformComponent transform{};
            transform.position = { 0.0f, 4.0f, 0.0f };
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

            RigidbodyComponent body{};
            body.type = PhysicsBodyType::Dynamic;
            body.position = transform.position;
            body.linearVelocity = { 0.0f, 0.0f, 1.0f };
			body.linearAcceleration = { 0.0f, 0.0f, 0.0f };

            registry.emplace<TransformComponent>(entity, transform);
			registry.emplace<MeshComponent>(entity, mesh);
			registry.emplace<MaterialComponent>(entity, material);
			registry.emplace<RigidbodyComponent>(entity, body);
        }


        {
            auto entity = registry.create();

            TransformComponent transform{};
            transform.position = { 3.0f, 0.0f, 0.0f };
            transform.scale = { 1.0f, 1.0f, 1.0f };
            transform.rotation = { 0.0f, 0.0f, 0.0f };

            MeshComponent mesh{};
            mesh.shapeType = ShapeType::Sphere;

            MaterialComponent material{};
            material.baseColor = { 80.0f, 255.0f, 120.0f };
            material.metallic = 0.1f;
            material.roughness = 0.5f;
            material.ao = 1.0f;
            material.textureId = 1;

            RigidbodyComponent body{};
            body.type = PhysicsBodyType::Static;
            body.position = transform.position;
            body.linearVelocity = { 0.0f, 0.0f, 0.0f };
            body.linearAcceleration = { 0.0f, 0.0f, 0.0f };

            registry.emplace<TransformComponent>(entity, transform);
            registry.emplace<MeshComponent>(entity, mesh);
            registry.emplace<MaterialComponent>(entity, material);
            registry.emplace<RigidbodyComponent>(entity, body);
        }
    }

    void OnUpdate(entt::registry& registry, const GameTime& time)
    {
        const float dt = time.FixedDeltaTime();

		auto view = registry.view<TransformComponent, RigidbodyComponent>();

        for(auto [entity, transform, body] : view.each())
        {
            if (body.type != PhysicsBodyType::Dynamic)
                continue;


            body.linearVelocity.x += body.linearAcceleration.x * dt;
            body.linearVelocity.y += body.linearAcceleration.y * dt;
            body.linearVelocity.z += body.linearAcceleration.z * dt;

			body.linearVelocity.x += gravity.x * dt;
            body.linearVelocity.y += gravity.y * dt;
			body.linearVelocity.z += gravity.z * dt;

			body.position.x += body.linearVelocity.x * dt;
			body.position.y += body.linearVelocity.y * dt;
			body.position.z += body.linearVelocity.z * dt;


            transform.position = body.position;
		}

    }

    void OnImGui(entt::registry& registry)
    {



    }








    void SetGravity(const DirectX::XMFLOAT3& newGravity)
    {
        gravity = newGravity;
	}

private:
    DirectX::XMFLOAT3 gravity = { 0.0f, -0.8f, 0.0f };


};
