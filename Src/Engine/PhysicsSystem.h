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
        // -------------------------
        // Small dynamic sphere
        // -------------------------
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

            PhysicsBodyComponent body{};
            body.type = PhysicsBodyType::Dynamic;
            body.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };
            body.mass = 1.0f;
            body.invMass = 1.0f / body.mass;
            body.linearVelocity = { 0.0f, 0.0f, 0.0f };
            body.useGravity = true;

            SphereColliderComponent collider{};
            collider.radius = 1.0f;
            collider.centerOfMassLocal = { 0.0f, 0.0f, 0.0f };

            registry.emplace<TransformComponent>(entity, transform);
            registry.emplace<MeshComponent>(entity, mesh);
            registry.emplace<MaterialComponent>(entity, material);
            registry.emplace<PhysicsBodyComponent>(entity, body);
            registry.emplace<SphereColliderComponent>(entity, collider);
        }

        // -------------------------
        // Big static sphere
        // -------------------------
        {
            auto entity = registry.create();

            TransformComponent transform{};
            transform.position = { 0.0f, -10.5f, 0.0f };
            transform.scale = { 6.0f, 6.0f, 6.0f };
            transform.rotation = { 0.0f, 0.0f, 0.0f };

            MeshComponent mesh{};
            mesh.shapeType = ShapeType::Sphere;

            MaterialComponent material{};
            material.baseColor = { 255.0f, 255.0f, 255.0f };
            material.metallic = 0.1f;
            material.roughness = 0.5f;
            material.ao = 1.0f;
            material.textureId = 2;

            PhysicsBodyComponent body{};
            body.type = PhysicsBodyType::Static;
            body.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };
            body.mass = 0.0f;
            body.invMass = 0.0f;
            body.linearVelocity = { 0.0f, 0.0f, 0.0f };
            body.useGravity = false;

            SphereColliderComponent collider{};
            collider.radius = 6.0f;
            collider.centerOfMassLocal = { 0.0f, 0.0f, 0.0f };

            registry.emplace<TransformComponent>(entity, transform);
            registry.emplace<MeshComponent>(entity, mesh);
            registry.emplace<MaterialComponent>(entity, material);
            registry.emplace<PhysicsBodyComponent>(entity, body);
            registry.emplace<SphereColliderComponent>(entity, collider);
        }
    }

    void OnUpdate(entt::registry& registry, const GameTime& time)
    {
        const float dt = time.FixedDeltaTime();

        auto view = registry.view<TransformComponent>();

        ApplyGravityImpulse(registry, dt);
        DetectCollisions(registry);
        IntegratePositions(registry, dt);
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
            const DirectX::XMFLOAT3 deltaVelocity = GameMath::Mul(gravity, dt);

            body.linearVelocity = GameMath::Add(body.linearVelocity, deltaVelocity);
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



    // -------------

    void SetMass(PhysicsBodyComponent& body, float mass)
    {
        body.mass = std::max(mass, 0.0001f);

        if (body.type == PhysicsBodyType::Static)
        {
            body.invMass = 0.0f;
        }
        else
        {
            body.invMass = 1.0f / body.mass;
        }
    }


    void SetBodyType(PhysicsBodyComponent& body, PhysicsBodyType type)
    {
        body.type = type;

        if (body.type == PhysicsBodyType::Static)
        {
            body.mass = 0.0f;
            body.invMass = 0.0f;
            body.linearVelocity = { 0.0f, 0.0f, 0.0f };
        }
        else
        {
            if (body.mass <= 0.0f)
                body.mass = 1.0f;

            body.invMass = 1.0f / body.mass;
        }
    }

    void ApplyLinearImpulse(PhysicsBodyComponent& body, const DirectX::XMFLOAT3& impulse )
    {
        if (!body.enabled)
            return;

        if (body.type != PhysicsBodyType::Dynamic)
            return;

        if (body.invMass == 0.0f)
            return;

        // dv = J / m
        // invMass = 1 / m:
        // dv = J * invMass
        DirectX::XMFLOAT3 deltaVelocity = GameMath::Mul(impulse, body.invMass);

        body.linearVelocity = GameMath::Add(body.linearVelocity, deltaVelocity);
    }


    void ApplyGravityImpulse(entt::registry& registry, float dt)
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

            // F = m * g
            const DirectX::XMFLOAT3 force = GameMath::Mul(gravity, body.mass);

            // J = F * dt
            const DirectX::XMFLOAT3 impulse = GameMath::Mul(force, dt);

            ApplyLinearImpulse(body, impulse);
        }
    }



    bool IntersectSphereSphere(const TransformComponent& transformA, const SphereColliderComponent& sphereA, const TransformComponent& transformB, const SphereColliderComponent& sphereB)
    {
        const DirectX::XMFLOAT3 ab = GameMath::Sub(transformB.position, transformA.position);

        const float radiusA = sphereA.radius;
        const float radiusB = sphereB.radius;

        const float radiusSum = radiusA + radiusB;
        const float distanceSq = GameMath::LengthSq(ab);

        return distanceSq <= radiusSum * radiusSum;
    }

    void StopBody(PhysicsBodyComponent& body)
    {
        if (body.type != PhysicsBodyType::Dynamic)
            return;

        body.linearVelocity = { 0.0f, 0.0f, 0.0f };
    }


    void DetectCollisions(entt::registry& registry)
    {
        auto view = registry.view<TransformComponent,PhysicsBodyComponent,SphereColliderComponent>();

        std::vector<entt::entity> entities;
        entities.reserve(view.size_hint());

        for (auto [entity, transform, body, sphere] : view.each())
        {
            entities.push_back(entity);
        }

        for (size_t i = 0; i < entities.size(); ++i)
        {
            for (size_t j = i + 1; j < entities.size(); ++j)
            {
                entt::entity entityA = entities[i];
                entt::entity entityB = entities[j];

                auto& transformA = registry.get<TransformComponent>(entityA);
                auto& bodyA = registry.get<PhysicsBodyComponent>(entityA);
                auto& sphereA = registry.get<SphereColliderComponent>(entityA);

                auto& transformB = registry.get<TransformComponent>(entityB);
                auto& bodyB = registry.get<PhysicsBodyComponent>(entityB);
                auto& sphereB = registry.get<SphereColliderComponent>(entityB);

                if (!bodyA.enabled || !bodyB.enabled)
                    continue;

                if (bodyA.invMass == 0.0f && bodyB.invMass == 0.0f)
                    continue;

                if (IntersectSphereSphere(transformA, sphereA, transformB, sphereB))
                {
                    StopBody(bodyA);
                    StopBody(bodyB);
                }
            }
        }
    }



	// TODO: move this to a config file or something
    DirectX::XMFLOAT3 gravity = { 0.0f, -1.0f, 0.0f };


};
