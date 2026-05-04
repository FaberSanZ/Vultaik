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
            body.angularVelocity = { 0.0f, 0.0f, 0.0f };
            body.useGravity = true;
            body.restitution = 0.5f;
			body.massPropertiesDirty = true;
            body.friction = 0.6f;

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
            body.angularVelocity = { 0.0f, 0.0f, 0.0f };
            body.useGravity = false;
            body.restitution = 0.5f;
            body.friction = 0.6f;

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

        UpdateMassProperties(registry);

        ApplyGravityImpulse(registry, dt);

        IntegratePositions(registry, dt);

        SolveContacts(registry);

        IntegrateOrientations(registry, dt);
    }

    void OnImGui(entt::registry& registry)
    {
        ImGui::Begin("Physics Debug");

        ImGui::DragFloat3("Gravity", &gravity.x, 0.05f, -50.0f, 50.0f);
        ImGui::DragFloat("Rest Threshold", &restingVelocityThreshold, 0.01f, 0.0f, 5.0f);

        ImGui::SeparatorText("Chapter 13 - General Impulses");

        if (ImGui::Button("Apply Center Impulse"))
        {
            ApplyDebugCenterImpulse(registry);
        }

        if (ImGui::Button("Apply Side Impulse"))
        {
            ApplyDebugSideImpulse(registry);
        }

        ImGui::SeparatorText("First Dynamic Body");

        entt::entity entity = FindFirstDynamicSphere(registry);

        if (entity != entt::null)
        {
            auto& transform = registry.get<TransformComponent>(entity);
            auto& body = registry.get<PhysicsBodyComponent>(entity);
            auto& sphere = registry.get<SphereColliderComponent>(entity);

            ImGui::Text(
                "Position: %.3f %.3f %.3f",
                transform.position.x,
                transform.position.y,
                transform.position.z
            );

            ImGui::Text(
                "Linear Velocity: %.3f %.3f %.3f",
                body.linearVelocity.x,
                body.linearVelocity.y,
                body.linearVelocity.z
            );

            ImGui::Text(
                "Angular Velocity: %.3f %.3f %.3f",
                body.angularVelocity.x,
                body.angularVelocity.y,
                body.angularVelocity.z
            );

            ImGui::Text(
                "Inv Inertia: %.3f %.3f %.3f",
                body.invInertiaLocal.x,
                body.invInertiaLocal.y,
                body.invInertiaLocal.z
            );

            ImGui::Text("Mass: %.3f", body.mass);
            ImGui::Text("Inv Mass: %.3f", body.invMass);
            ImGui::Text("Radius: %.3f", sphere.radius);

            if (ImGui::Button("Stop Linear Velocity"))
            {
                body.linearVelocity = { 0.0f, 0.0f, 0.0f };
            }

            if (ImGui::Button("Stop Angular Velocity"))
            {
                body.angularVelocity = { 0.0f, 0.0f, 0.0f };
            }
        }
        else
        {
            ImGui::TextUnformatted("No dynamic sphere found.");
        }

        ImGui::End();
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



    bool IntersectSphereSphere(entt::entity entityA, entt::entity entityB, const TransformComponent& transformA, const SphereColliderComponent& sphereA, const TransformComponent& transformB, const SphereColliderComponent& sphereB, Contact& contact)
    {
        const DirectX::XMFLOAT3 ab = GameMath::Sub(transformB.position, transformA.position);

        const float distance = GameMath::Length(ab);

        const DirectX::XMFLOAT3 normal = distance > 0.00001f ? GameMath::Div(ab, distance) : DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f };

        const float radiusA =  sphereA.radius;
        const float radiusB = sphereB.radius;

        const float radiusSum = radiusA + radiusB;

        contact.entityA = entityA;
        contact.entityB = entityB;
        contact.normal = normal;

        contact.pointOnAWorld = GameMath::Add(transformA.position, GameMath::Mul(normal, radiusA));

        contact.pointOnBWorld = GameMath::Sub(transformB.position, GameMath::Mul(normal, radiusB));

        contact.separationDistance = distance - radiusSum;

        return contact.separationDistance <= 0.0f;
    }



    void ResolveContactProjection(entt::registry& registry, const Contact& contact)
    {
        auto& transformA = registry.get<TransformComponent>(contact.entityA);
        auto& bodyA = registry.get<PhysicsBodyComponent>(contact.entityA);

        auto& transformB = registry.get<TransformComponent>(contact.entityB);
        auto& bodyB = registry.get<PhysicsBodyComponent>(contact.entityB);

        if (contact.separationDistance >= 0.0f)
            return;

        const float invMassA = bodyA.invMass;
        const float invMassB = bodyB.invMass;
        const float invMassSum = invMassA + invMassB;

        if (invMassSum <= 0.0f)
            return;

        const float penetrationDepth = -contact.separationDistance;

        const DirectX::XMFLOAT3 correction = GameMath::Mul(contact.normal, penetrationDepth);
        const DirectX::XMFLOAT3 correctionA = GameMath::Mul(correction, invMassA / invMassSum);
        const DirectX::XMFLOAT3 correctionB = GameMath::Mul(correction, invMassB / invMassSum);

        if (bodyA.type == PhysicsBodyType::Dynamic)
        {
            transformA.position = GameMath::Sub(transformA.position, correctionA);
        }

        if (bodyB.type == PhysicsBodyType::Dynamic)
        {
            transformB.position = GameMath::Add(transformB.position, correctionB);
        }
    }


    void SolveContacts(entt::registry& registry)
    {
        constexpr int solverIterations = 4;

        for (int iteration = 0; iteration < solverIterations; ++iteration)
        {
            std::vector<Contact> contacts;
            BuildContactList(registry, contacts);

            for (const Contact& contact : contacts)
            {
                ResolveContactProjection(registry, contact);
            }

            for (const Contact& contact : contacts)
            {
                ResolveContactVelocity(registry, contact);
            }
        }
    }

    void ResolveContactVelocity(entt::registry& registry, const Contact& contact)
    {
        auto& transformA = registry.get<TransformComponent>(contact.entityA);
        auto& bodyA = registry.get<PhysicsBodyComponent>(contact.entityA);
        auto& sphereA = registry.get<SphereColliderComponent>(contact.entityA);

        auto& transformB = registry.get<TransformComponent>(contact.entityB);
        auto& bodyB = registry.get<PhysicsBodyComponent>(contact.entityB);
        auto& sphereB = registry.get<SphereColliderComponent>(contact.entityB);

        if (!bodyA.enabled || !bodyB.enabled)
            return;

        if (bodyA.invMass + bodyB.invMass <= 0.0f)
            return;

        const DirectX::XMFLOAT3 normal = contact.normal;
        const DirectX::XMFLOAT3 contactPoint = GetContactPoint(contact);
        const DirectX::XMFLOAT3 centerA = GetCenterOfMassWorld(transformA, bodyA, sphereA);
        const DirectX::XMFLOAT3 centerB = GetCenterOfMassWorld(transformB, bodyB, sphereB);
        const DirectX::XMFLOAT3 rA = GameMath::Sub(contactPoint, centerA);
        const DirectX::XMFLOAT3 rB = GameMath::Sub(contactPoint, centerB);

        // Normal impulse
        DirectX::XMFLOAT3 velocityA = GetVelocityAtPoint(bodyA, centerA, contactPoint);
        DirectX::XMFLOAT3 velocityB = GetVelocityAtPoint(bodyB, centerB, contactPoint);
        DirectX::XMFLOAT3 relativeVelocity = GameMath::Sub(velocityB, velocityA);

        float contactVelocity = GameMath::Dot(relativeVelocity, normal);

        if (contactVelocity > 0.0f)
            return;

        float restitution = CombineRestitution(bodyA, bodyB);

        if (std::abs(contactVelocity) < restingVelocityThreshold)
            restitution = 0.0f;

        const float normalDenominator = ComputeImpulseDenominator(bodyA, bodyB, rA, rB, normal);

        if (normalDenominator <= 0.00001f)
            return;

        const float normalImpulseMagnitude = -(1.0f + restitution) * contactVelocity / normalDenominator;

        const DirectX::XMFLOAT3 normalImpulse = GameMath::Mul(normal, normalImpulseMagnitude);

        ApplyImpulsePairAtPoint(bodyA, bodyB, centerA, centerB, contactPoint, normalImpulse);


        // Friction impulse
        velocityA = GetVelocityAtPoint(bodyA, centerA, contactPoint);
        velocityB = GetVelocityAtPoint(bodyB, centerB, contactPoint);

        relativeVelocity = GameMath::Sub(velocityB, velocityA);

        const float normalSpeed = GameMath::Dot(relativeVelocity, normal);
        DirectX::XMFLOAT3 tangentVelocity = GameMath::Sub(relativeVelocity, GameMath::Mul(normal, normalSpeed));
        const float tangentSpeedSq = GameMath::LengthSq(tangentVelocity);

        if (tangentSpeedSq <= 0.000001f)
            return;

        const DirectX::XMFLOAT3 tangent = GameMath::Normalize(tangentVelocity);
        const float tangentDenominator = ComputeImpulseDenominator(bodyA, bodyB, rA, rB, tangent);

        if (tangentDenominator <= 0.00001f)
            return;

        float tangentImpulseMagnitude = -GameMath::Dot(relativeVelocity, tangent) / tangentDenominator;
        const float friction = CombineFriction(bodyA, bodyB);
        const float maxFrictionImpulse = normalImpulseMagnitude * friction;

        tangentImpulseMagnitude = std::clamp(tangentImpulseMagnitude, -maxFrictionImpulse, maxFrictionImpulse);

        const DirectX::XMFLOAT3 frictionImpulse = GameMath::Mul(tangent, tangentImpulseMagnitude);

        ApplyImpulsePairAtPoint(bodyA, bodyB, centerA, centerB, contactPoint, frictionImpulse);
    }


    void BuildContactList(entt::registry& registry, std::vector<Contact>& contacts)
    {
        contacts.clear();

        auto view = registry.view<TransformComponent, PhysicsBodyComponent, SphereColliderComponent>();

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

                Contact contact{};

                if (IntersectSphereSphere(entityA, entityB, transformA, sphereA, transformB, sphereB, contact))
                {
                    contacts.push_back(contact);
                }
            }
        }
    }

    float CombineRestitution(const PhysicsBodyComponent& bodyA, const PhysicsBodyComponent& bodyB)
    {
        return std::min(bodyA.restitution, bodyB.restitution);
    }

    void SetSphereInertia(PhysicsBodyComponent& body, const SphereColliderComponent& sphere)
    {
        if (body.type == PhysicsBodyType::Static || body.invMass == 0.0f)
        {
            body.invInertiaLocal = { 0.0f, 0.0f, 0.0f };
            return;
        }

        const float radius = sphere.radius;
        const float inertia = (2.0f / 5.0f) * body.mass * radius * radius;

        if (inertia <= 0.00001f)
        {
            body.invInertiaLocal = { 0.0f, 0.0f, 0.0f };
            return;
        }

        const float invI = 1.0f / inertia;
        body.invInertiaLocal = { invI, invI, invI };
    }


    void ApplyAngularImpulse(PhysicsBodyComponent& body, const DirectX::XMFLOAT3& angularImpulse)
    {
        if (!body.enabled)
            return;

        if (body.type != PhysicsBodyType::Dynamic)
            return;

        DirectX::XMFLOAT3 deltaAngularVelocity =
        {
            angularImpulse.x * body.invInertiaLocal.x,
            angularImpulse.y * body.invInertiaLocal.y,
            angularImpulse.z * body.invInertiaLocal.z
        };

        body.angularVelocity = GameMath::Add(body.angularVelocity, deltaAngularVelocity);

        const float maxAngularSpeed = 30.0f;
        const float speedSq = GameMath::LengthSq(body.angularVelocity);

        if (speedSq > maxAngularSpeed * maxAngularSpeed)
        {
            body.angularVelocity = GameMath::Mul(GameMath::Normalize(body.angularVelocity), maxAngularSpeed);
        }
    }


    void IntegrateOrientations(entt::registry& registry, float dt)
    {
        auto view = registry.view<PhysicsBodyComponent>();

        for (auto entity : view)
        {
            auto& body = view.get<PhysicsBodyComponent>(entity);

            if (!body.enabled)
                continue;

            if (body.type != PhysicsBodyType::Dynamic)
                continue;

            const float angularSpeed = GameMath::Length(body.angularVelocity);

            if (angularSpeed <= 0.00001f)
                continue;

            using namespace DirectX;

            XMVECTOR axis = XMLoadFloat3(&body.angularVelocity);
            axis = XMVector3Normalize(axis);

            const float angle = angularSpeed * dt;

            XMVECTOR deltaRotation = XMQuaternionRotationAxis(axis, angle);

            XMVECTOR orientation = XMLoadFloat4(&body.orientation);

            orientation = XMQuaternionNormalize(XMQuaternionMultiply(deltaRotation, orientation));

            XMStoreFloat4(&body.orientation, orientation);
        }
    }

    void UpdateMassProperties(entt::registry& registry)
    {
        auto view = registry.view<TransformComponent, PhysicsBodyComponent, SphereColliderComponent>();

        for (auto [entity, transform, body, sphere] : view.each())
        {
            if (!body.massPropertiesDirty)
                continue;

            if (body.type == PhysicsBodyType::Static)
            {
                body.mass = 0.0f;
                body.invMass = 0.0f;
                body.invInertiaLocal = { 0.0f, 0.0f, 0.0f };
                body.linearVelocity = { 0.0f, 0.0f, 0.0f };
                body.angularVelocity = { 0.0f, 0.0f, 0.0f };
                body.massPropertiesDirty = false;
                continue;
            }

            body.mass = std::max(body.mass, 0.0001f);
            body.invMass = 1.0f / body.mass;

            const float radius = sphere.radius;
            const float inertia = (2.0f / 5.0f) * body.mass * radius * radius;

            if (inertia <= 0.00001f)
            {
                body.invInertiaLocal = { 0.0f, 0.0f, 0.0f };
            }
            else
            {
                const float invI = 1.0f / inertia;
                body.invInertiaLocal = { invI, invI, invI };
            }

            body.massPropertiesDirty = false;
        }
    }

    DirectX::XMFLOAT3 GetCenterOfMassWorld(const TransformComponent& transform,const PhysicsBodyComponent& body,const SphereColliderComponent& collider)
    {
        DirectX::XMVECTOR localCenter = DirectX::XMLoadFloat3(&collider.centerOfMassLocal);
        DirectX::XMVECTOR orientation = DirectX::XMLoadFloat4(&body.orientation);

        orientation = DirectX::XMQuaternionNormalize(orientation);

        DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&transform.position);
        DirectX::XMVECTOR rotatedCenter = DirectX::XMVector3Rotate(localCenter, orientation);
        DirectX::XMVECTOR worldCenter = DirectX::XMVectorAdd(rotatedCenter, position);

        DirectX::XMFLOAT3 result{};
        DirectX::XMStoreFloat3(&result, worldCenter);

        return result;
    }


    void ApplyImpulseAtPoint(PhysicsBodyComponent& body, const DirectX::XMFLOAT3& centerOfMassWorld, const DirectX::XMFLOAT3& pointWorld, const DirectX::XMFLOAT3& impulse)
    {
        if (!body.enabled)
            return;

        if (body.type != PhysicsBodyType::Dynamic)
            return;


        // v += J * invMass
        ApplyLinearImpulse(body, impulse);


        const DirectX::XMFLOAT3 r = GameMath::Sub(pointWorld, centerOfMassWorld);

        // Jangular = r x Jlinear
        const DirectX::XMFLOAT3 angularImpulse = GameMath::Cross(r, impulse);

        ApplyAngularImpulse(body, angularImpulse);
    }



    void ApplyImpulseAtPoint(entt::registry& registry, entt::entity entity, const DirectX::XMFLOAT3& pointWorld, const DirectX::XMFLOAT3& impulse)
    {
        if (!registry.valid(entity))
            return;

        if (!registry.all_of<TransformComponent, PhysicsBodyComponent, SphereColliderComponent>(entity))
            return;

        auto& transform = registry.get<TransformComponent>(entity);
        auto& body = registry.get<PhysicsBodyComponent>(entity);
        auto& collider = registry.get<SphereColliderComponent>(entity);

        const DirectX::XMFLOAT3 centerOfMassWorld =GetCenterOfMassWorld(transform, body, collider);

        ApplyImpulseAtPoint(body, centerOfMassWorld, pointWorld, impulse);
    }


    entt::entity FindFirstDynamicSphere(entt::registry& registry)
    {
        auto view = registry.view<TransformComponent, PhysicsBodyComponent, SphereColliderComponent>();

        for (auto [entity, transform, body, sphere] : view.each())
        {
            if (!body.enabled)
                continue;

            if (body.type != PhysicsBodyType::Dynamic)
                continue;

            return entity;
        }

        return entt::null;
    }


    void ApplyDebugCenterImpulse(entt::registry& registry)
    {
        entt::entity entity = FindFirstDynamicSphere(registry);

        if (entity == entt::null)
            return;

        auto& transform = registry.get<TransformComponent>(entity);

        const DirectX::XMFLOAT3 pointWorld =
        {
            transform.position.x,
            transform.position.y,
            transform.position.z
        };

        const DirectX::XMFLOAT3 impulse =
        {
            0.0f,
            8.0f,
            0.0f
        };

        ApplyImpulseAtPoint(registry, entity, pointWorld, impulse);
    }

    void ApplyDebugSideImpulse(entt::registry& registry)
    {
        entt::entity entity = FindFirstDynamicSphere(registry);

        if (entity == entt::null)
            return;

        auto& transform = registry.get<TransformComponent>(entity);
        auto& sphere = registry.get<SphereColliderComponent>(entity);

        const float radius = sphere.radius;

        const DirectX::XMFLOAT3 pointWorld =
        {
            transform.position.x + radius,
            transform.position.y,
            transform.position.z
        };

        const DirectX::XMFLOAT3 impulse =
        {
            0.0f,
            8.0f,
            0.0f
        };

        ApplyImpulseAtPoint(registry, entity, pointWorld, impulse);
    }


    DirectX::XMFLOAT3 MulByInvInertia(const PhysicsBodyComponent& body, const DirectX::XMFLOAT3& v)
    {
        return
        {
            v.x * body.invInertiaLocal.x,
            v.y * body.invInertiaLocal.y,
            v.z * body.invInertiaLocal.z
        };
    }


    DirectX::XMFLOAT3 GetVelocityAtPoint(const PhysicsBodyComponent& body, const DirectX::XMFLOAT3& centerOfMassWorld, const DirectX::XMFLOAT3& pointWorld)
    {
        const DirectX::XMFLOAT3 r = GameMath::Sub(pointWorld, centerOfMassWorld);
        const DirectX::XMFLOAT3 angularVelocityAtPoint = GameMath::Cross(body.angularVelocity, r);

        return GameMath::Add(body.linearVelocity, angularVelocityAtPoint);
    }


    DirectX::XMFLOAT3 GetContactPoint(const Contact& contact)
    {
        return GameMath::Mul(GameMath::Add(contact.pointOnAWorld, contact.pointOnBWorld), 0.5f);
    }



    float CombineFriction(const PhysicsBodyComponent& bodyA,const PhysicsBodyComponent& bodyB)
    {
        return std::min(bodyA.friction, bodyB.friction);
    }


    float ComputeImpulseDenominator(const PhysicsBodyComponent& bodyA, const PhysicsBodyComponent& bodyB, const DirectX::XMFLOAT3& rA, const DirectX::XMFLOAT3& rB, const DirectX::XMFLOAT3& direction)
    {
        const DirectX::XMFLOAT3 rAcrossDir = GameMath::Cross(rA, direction);
        const DirectX::XMFLOAT3 rBcrossDir = GameMath::Cross(rB, direction);
        const DirectX::XMFLOAT3 invInertiaA = MulByInvInertia(bodyA, rAcrossDir);
        const DirectX::XMFLOAT3 invInertiaB = MulByInvInertia(bodyB, rBcrossDir);
        const DirectX::XMFLOAT3 angularA = GameMath::Cross(invInertiaA, rA);
        const DirectX::XMFLOAT3 angularB = GameMath::Cross(invInertiaB, rB);
        const DirectX::XMFLOAT3 angularSum = GameMath::Add(angularA, angularB);

        return bodyA.invMass + bodyB.invMass + GameMath::Dot(angularSum, direction);
    }


    void ApplyImpulsePairAtPoint(PhysicsBodyComponent& bodyA, PhysicsBodyComponent& bodyB, const DirectX::XMFLOAT3& centerA, const DirectX::XMFLOAT3& centerB, const DirectX::XMFLOAT3& contactPoint, const DirectX::XMFLOAT3& impulse)
    {
        if (bodyA.type == PhysicsBodyType::Dynamic)
        {
            ApplyImpulseAtPoint(bodyA, centerA, contactPoint, GameMath::Mul(impulse, -1.0f));;
        }

        if (bodyB.type == PhysicsBodyType::Dynamic)
        {
            ApplyImpulseAtPoint(bodyB, centerB, contactPoint, impulse);
        }
    }



	// TODO: move this to a config file or something
    DirectX::XMFLOAT3 gravity = { 0.0f, -1.0f, 0.0f };

    float restingVelocityThreshold = 0.5f;
};
