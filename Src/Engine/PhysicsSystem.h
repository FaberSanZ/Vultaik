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
        // -------------------------
        // Small dynamic sphere
        // -------------------------
        {
            auto entity = registry.create();

            TransformComponent transform{};
            transform.position = { -3.0f, 0.0f, 2.0f };
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
            body.enableCCD = true;

            SphereColliderComponent collider{};
            collider.radius = 1.0f;
            collider.centerOfMassLocal = { 0.0f, 0.0f, 0.0f };


            registry.emplace<TransformComponent>(entity, transform);
            registry.emplace<MeshComponent>(entity, mesh);
            registry.emplace<MaterialComponent>(entity, material);
            registry.emplace<PhysicsBodyComponent>(entity, body);
            registry.emplace<SphereColliderComponent>(entity, collider);

        }


        {
            auto entity = registry.create();

            TransformComponent transform{};
            transform.position = { 0.0f, 0.0f, 2.0f };
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
            body.enableCCD = true;

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
            transform.position = { 0.0f, -22.5f, 0.0f };
            transform.scale = { 20.0f, 20.0f, 20.0f };
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
            body.enableCCD = true;

            SphereColliderComponent collider{};
            collider.radius = 20.0f;
            collider.centerOfMassLocal = { 0.0f, 0.0f, 0.0f };

            registry.emplace<TransformComponent>(entity, transform);
            registry.emplace<MeshComponent>(entity, mesh);
            registry.emplace<MaterialComponent>(entity, material);
            registry.emplace<PhysicsBodyComponent>(entity, body);
            registry.emplace<SphereColliderComponent>(entity, collider);
        }


        {
            auto entity = registry.create();

            TransformComponent transform{};
            transform.position = { 3.0f, 4.0f, 0.0f };
            transform.scale = { 2.0f, 1.0f, 1.0f };
            transform.rotation = { 0.0f, 0.0f, 0.0f };

            MeshComponent mesh{};
            mesh.shapeType = ShapeType::Cube;

            MaterialComponent material{};
            material.baseColor = { 80.0f, 180.0f, 255.0f };
            material.metallic = 0.1f;
            material.roughness = 0.5f;
            material.ao = 1.0f;
            material.textureId = 1;

            PhysicsBodyComponent body{};
            body.type = PhysicsBodyType::Dynamic;
            body.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };
            body.mass = 2.0f;
            body.invMass = 1.0f / body.mass;
            body.linearVelocity = { 0.0f, 0.0f, 0.0f };
            body.angularVelocity = { 0.0f, 1.0f, 0.0f };
            body.useGravity = true;
            body.restitution = 0.2f;
            body.friction = 0.7f;
            body.enableCCD = false;
            body.massPropertiesDirty = true;

            BoxColliderComponent box{};
            box.halfExtents = { 1.0f, 0.5f, 0.5f };
            box.centerLocal = { 0.0f, 0.0f, 0.0f };

            registry.emplace<TransformComponent>(entity, transform);
            registry.emplace<MeshComponent>(entity, mesh);
            registry.emplace<MaterialComponent>(entity, material);
            registry.emplace<PhysicsBodyComponent>(entity, body);
            registry.emplace<BoxColliderComponent>(entity, box);
        }
    }

    void OnUpdate(entt::registry& registry, const GameTime& time)
    {
        const float dt = time.FixedDeltaTime();
        UpdateMassProperties(registry);
        StorePreviousPositions(registry);

        ApplyGravityImpulse(registry, dt);
        IntegratePositions(registry, dt);
        StoreTargetPositions(registry);

        UpdateBounds(registry);

        SolveTOI(registry, dt);
        SolveContacts(registry);

        IntegrateOrientations(registry, dt);
        UpdateBounds(registry);
    }

    void OnImGui(entt::registry& registry)
    {
		// resets scene
        if (ImGui::Button("Reset Scene"))
        {
            registry.clear();
            OnInitialize(registry);
		}

		// generate random spheres
        if (ImGui::Button("Generate Random Spheres"))
        {
            for (int i = 0; i < 15; ++i)
            {
                auto entity = registry.create();
                TransformComponent transform{};
                transform.position = { GameMath::RandomFloat(-5.0f, 5.0f), GameMath::RandomFloat(0.0f, 5.0f), GameMath::RandomFloat(-5.0f, 5.0f) };
                transform.scale = { 1.0f, 1.0f, 1.0f };
                transform.rotation = { 0.0f, 0.0f, 0.0f };

                MeshComponent mesh{};
                mesh.shapeType = ShapeType::Sphere;

                MaterialComponent material{};
                material.baseColor = { 255.0f, 80.0f, 80.0f };
                material.metallic = 0.1f;
                material.roughness = 0.5f;
                material.ao = 1.0f;
                material.textureId = GameMath::RandomFloat(1.0f, 4.0f);

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
                body.enableCCD = false;

                SphereColliderComponent collider{};
                collider.radius = 1.0f;
                collider.centerOfMassLocal = { 0.0f, 0.0f, 0.0f };

                registry.emplace<TransformComponent>(entity, transform);
                registry.emplace<MeshComponent>(entity, mesh);
                registry.emplace<MaterialComponent>(entity, material);
                registry.emplace<PhysicsBodyComponent>(entity, body);
                registry.emplace<SphereColliderComponent>(entity, collider);
            }
        }


        ImGui::SeparatorText("Physics Pipeline");
        ImGui::Text("Potential pairs: %u", m_lastPotentialPairs);
        ImGui::Text("Broadphase pairs: %u", m_lastBroadphasePairs);
        ImGui::Text("Narrowphase contacts: %u", m_lastNarrowphaseContacts);
        ImGui::Text("Narrowphase tests: %u", m_lastNarrowphaseTests);
        ImGui::Text("Sphere/Sphere tests: %u", m_lastSphereSphereTests);
        ImGui::Text("Generated contacts: %u", m_lastGeneratedContacts);


    }


    

    void ApplyGravityImpulse(entt::registry& registry, float dt)
    {
        auto view = registry.view<PhysicsBodyComponent>();

        const DirectX::XMVECTOR gravityV = DirectX::XMLoadFloat3(&gravity);

        for (auto entity : view)
        {
            auto& body = view.get<PhysicsBodyComponent>(entity);

            if (!body.enabled)
                continue;

            if (body.type != PhysicsBodyType::Dynamic)
                continue;

            if (!body.useGravity)
                continue;

            DirectX::XMVECTOR velocity = DirectX::XMLoadFloat3(&body.linearVelocity);
            DirectX::XMVECTOR deltaVelocity = DirectX::XMVectorScale(gravityV, dt);

            velocity = DirectX::XMVectorAdd(velocity, deltaVelocity);

            DirectX::XMStoreFloat3(&body.linearVelocity, velocity);
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

            DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&transform.position);
            DirectX::XMVECTOR velocity = DirectX::XMLoadFloat3(&body.linearVelocity);
            DirectX::XMVECTOR delta = DirectX::XMVectorScale(velocity, dt);

            position = DirectX::XMVectorAdd(position, delta);

            DirectX::XMStoreFloat3(&transform.position, position);
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


    //void ApplyGravityImpulse(entt::registry& registry, float dt)
    //{
    //    auto view = registry.view<PhysicsBodyComponent>();

    //    for (auto entity : view)
    //    {
    //        auto& body = view.get<PhysicsBodyComponent>(entity);

    //        if (!body.enabled)
    //            continue;

    //        if (body.type != PhysicsBodyType::Dynamic)
    //            continue;

    //        if (!body.useGravity)
    //            continue;

    //        // F = m * g
    //        const DirectX::XMFLOAT3 force = GameMath::Mul(gravity, body.mass);

    //        // J = F * dt
    //        const DirectX::XMFLOAT3 impulse = GameMath::Mul(force, dt);

    //        ApplyLinearImpulse(body, impulse);
    //    }
    //}




    bool BuildSphereSphereContact(entt::entity entityA, entt::entity entityB, const TransformComponent& transformA, const PhysicsBodyComponent& bodyA, const SphereColliderComponent& sphereA, const TransformComponent& transformB, const PhysicsBodyComponent& bodyB, const SphereColliderComponent& sphereB, Contact& contact)
    {
        const DirectX::XMFLOAT3 centerA = GetSphereCenterWorld(transformA, bodyA, sphereA);
        const DirectX::XMFLOAT3 centerB = GetSphereCenterWorld(transformB, bodyB, sphereB);

        const DirectX::XMFLOAT3 ab = GameMath::Sub(centerB, centerA);
        const float distance = GameMath::Length(ab);

        const float radiusA = sphereA.radius;
        const float radiusB = sphereB.radius;
        const float radiusSum = radiusA + radiusB;

        const float separation = distance - radiusSum;

        if (separation > 0.0f)
            return false;

        const DirectX::XMFLOAT3 normal = distance > 0.00001f ? GameMath::Div(ab, distance) : DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f };

        contact.entityA = entityA;
        contact.entityB = entityB;
        contact.normal = normal;
        contact.pointOnAWorld = GameMath::Add(centerA, GameMath::Mul(normal, radiusA));
        contact.pointOnBWorld = GameMath::Sub(centerB, GameMath::Mul(normal, radiusB));
        contact.separationDistance = separation;

        return true;
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
        constexpr int projectionIterations = 4;

        std::vector<CollisionPair> pairs;
        BuildBroadphasePairs(registry, pairs);

        for (int iteration = 0; iteration < projectionIterations; ++iteration)
        {
            std::vector<Contact> contacts;
            BuildContactList(registry, pairs, contacts);

            for (const Contact& contact : contacts)
                ResolveContactProjection(registry, contact);
        }

        std::vector<Contact> contacts;
        BuildContactList(registry, pairs, contacts);

        for (const Contact& contact : contacts)
            ResolveContactVelocity(registry, contact);
    }

    void ResolveContactVelocity(entt::registry& registry, const Contact& contact)
    {
        if (!registry.all_of<TransformComponent, PhysicsBodyComponent>(contact.entityA))
            return;

        if (!registry.all_of<TransformComponent, PhysicsBodyComponent>(contact.entityB))
            return;

        auto& bodyA = registry.get<PhysicsBodyComponent>(contact.entityA);
        auto& bodyB = registry.get<PhysicsBodyComponent>(contact.entityB);

        if (!bodyA.enabled || !bodyB.enabled)
            return;

        if (bodyA.invMass + bodyB.invMass <= 0.0f)
            return;

        DirectX::XMFLOAT3 centerA{};
        DirectX::XMFLOAT3 centerB{};

        if (!GetEntityCenterOfMassWorld(registry, contact.entityA, centerA))
            return;

        if (!GetEntityCenterOfMassWorld(registry, contact.entityB, centerB))
            return;

        const DirectX::XMFLOAT3 normal = contact.normal;
        const DirectX::XMFLOAT3 contactPoint = GetContactPoint(contact);
        const DirectX::XMFLOAT3 rA = GameMath::Sub(contactPoint, centerA);
        const DirectX::XMFLOAT3 rB = GameMath::Sub(contactPoint, centerB);

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

    void BuildContactList(entt::registry& registry, const std::vector<CollisionPair>& pairs, std::vector<Contact>& contacts)
    {
        contacts.clear();

        m_lastNarrowphaseTests = 0;
        m_lastSphereSphereTests = 0;
        m_lastGeneratedContacts = 0;

        for (const CollisionPair& pair : pairs)
        {
            Contact contact{};

            if (GenerateContact(registry, pair.entityA, pair.entityB, contact))
            {
                contacts.push_back(contact);
                ++m_lastGeneratedContacts;
            }
        }

        m_lastNarrowphaseContacts = static_cast<uint32_t>(contacts.size());
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

        DirectX::XMFLOAT3 deltaAngularVelocity = MulByInvInertia(body, angularImpulse);
        body.angularVelocity = GameMath::Add(body.angularVelocity, deltaAngularVelocity);

        const float maxAngularSpeed = 30.0f;
        const float speedSq = GameMath::LengthSq(body.angularVelocity);

        if (speedSq > maxAngularSpeed * maxAngularSpeed)
            body.angularVelocity = GameMath::Mul(GameMath::Normalize(body.angularVelocity), maxAngularSpeed);
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
        auto view = registry.view<TransformComponent, PhysicsBodyComponent>();

        for (auto [entity, transform, body] : view.each())
        {
            if (!body.massPropertiesDirty)
                continue;

            PhysicsShapeType type = GetPhysicsShapeType(registry, entity);

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

            switch (type)
            {
            case PhysicsShapeType::Sphere:
            {
                if (!registry.all_of<SphereColliderComponent>(entity))
                    break;

                auto& sphere = registry.get<SphereColliderComponent>(entity);

                body.mass = std::max(body.mass, 0.0001f);
                body.invMass = 1.0f / body.mass;

                const float r = sphere.radius;
                const float inertia = (2.0f / 5.0f) * body.mass * r * r;
                const float invI = inertia > 0.00001f ? 1.0f / inertia : 0.0f;

                body.invInertiaLocal = { invI, invI, invI };
                body.massPropertiesDirty = false;
                break;
            }

            case PhysicsShapeType::Box:
            {
                if (!registry.all_of<BoxColliderComponent>(entity))
                    break;

                auto& box = registry.get<BoxColliderComponent>(entity);
                UpdateBoxMassProperties(body, box);
                break;
            }

            default:
                body.massPropertiesDirty = false;
                break;
            }
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

        if (!registry.all_of<TransformComponent, PhysicsBodyComponent>(entity))
            return;

        auto& body = registry.get<PhysicsBodyComponent>(entity);

        DirectX::XMFLOAT3 centerOfMassWorld{};

        if (!GetEntityCenterOfMassWorld(registry, entity, centerOfMassWorld))
            return;

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


    DirectX::XMFLOAT3 MulByInvInertia(const PhysicsBodyComponent& body, const DirectX::XMFLOAT3& vWorld)
    {
        using namespace DirectX;

        XMVECTOR v = XMLoadFloat3(&vWorld);
        XMVECTOR q = GameMath::LoadQuat(body.orientation);
        XMVECTOR invQ = XMQuaternionConjugate(q);

        XMVECTOR vLocal = XMVector3Rotate(v, invQ);
        XMVECTOR invInertia = XMLoadFloat3(&body.invInertiaLocal);
        XMVECTOR resultLocal = XMVectorMultiply(vLocal, invInertia);
        XMVECTOR resultWorld = XMVector3Rotate(resultLocal, q);

        DirectX::XMFLOAT3 result{};
        XMStoreFloat3(&result, resultWorld);
        return result;
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

    DirectX::XMFLOAT3 GetSphereCenterWorld(const TransformComponent& transform, const PhysicsBodyComponent& body, const SphereColliderComponent& sphere)
    {
        DirectX::XMVECTOR localCenter = DirectX::XMLoadFloat3(&sphere.centerOfMassLocal);
        DirectX::XMVECTOR orientation = DirectX::XMLoadFloat4(&body.orientation);
        orientation = DirectX::XMQuaternionNormalize(orientation);
        DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&transform.position);
        DirectX::XMVECTOR rotatedCenter = DirectX::XMVector3Rotate(localCenter, orientation);
        DirectX::XMVECTOR worldCenter = DirectX::XMVectorAdd(position, rotatedCenter);
        DirectX::XMFLOAT3 result{};
        DirectX::XMStoreFloat3(&result, worldCenter);
        return result;
    }


    void StorePreviousPositions(entt::registry& registry)
    {
        auto view = registry.view<TransformComponent, PhysicsBodyComponent>();

        for (auto [entity, transform, body] : view.each())
        {
            body.previousPosition = transform.position;
        }
    }


    bool SweptSphereSphere(entt::entity entityA, entt::entity entityB, const TransformComponent& transformA, const PhysicsBodyComponent& bodyA, const SphereColliderComponent& sphereA, const TransformComponent& transformB, const PhysicsBodyComponent& bodyB, const SphereColliderComponent& sphereB, Contact& contact, float& toi)
    {
        const DirectX::XMFLOAT3 centerA0 = bodyA.previousPosition;
        const DirectX::XMFLOAT3 centerB0 = bodyB.previousPosition;

        const DirectX::XMFLOAT3 centerA1 = bodyA.targetPosition;
        const DirectX::XMFLOAT3 centerB1 = bodyB.targetPosition;

        const DirectX::XMFLOAT3 relativeStart = GameMath::Sub(centerA0, centerB0);
        const DirectX::XMFLOAT3 moveA = GameMath::Sub(centerA1, centerA0);
        const DirectX::XMFLOAT3 moveB = GameMath::Sub(centerB1, centerB0);
        const DirectX::XMFLOAT3 relativeMove = GameMath::Sub(moveA, moveB);

        const float radiusSum = sphereA.radius + sphereB.radius;

        const float a = GameMath::Dot(relativeMove, relativeMove);
        const float b = 2.0f * GameMath::Dot(relativeStart, relativeMove);
        const float c = GameMath::Dot(relativeStart, relativeStart) - radiusSum * radiusSum;

        if (c <= 0.0f)
            return false;

        if (a <= 0.000001f)
            return false;

        const float discriminant = b * b - 4.0f * a * c;

        if (discriminant < 0.0f)
            return false;

        const float sqrtDiscriminant = std::sqrt(discriminant);
        const float t = (-b - sqrtDiscriminant) / (2.0f * a);

        if (t < 0.0f || t > 1.0f)
            return false;

        toi = t;

        const DirectX::XMFLOAT3 hitCenterA = GameMath::Lerp(centerA0, centerA1, toi);
        const DirectX::XMFLOAT3 hitCenterB = GameMath::Lerp(centerB0, centerB1, toi);

        const DirectX::XMFLOAT3 ab = GameMath::Sub(hitCenterB, hitCenterA);
        const float distance = GameMath::Length(ab);
        const DirectX::XMFLOAT3 normal = distance > 0.00001f ? GameMath::Div(ab, distance) : DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f };

        contact.entityA = entityA;
        contact.entityB = entityB;
        contact.normal = normal;
        contact.pointOnAWorld = GameMath::Add(hitCenterA, GameMath::Mul(normal, sphereA.radius));
        contact.pointOnBWorld = GameMath::Sub(hitCenterB, GameMath::Mul(normal, sphereB.radius));
        contact.separationDistance = 0.0f;

        return true;
    }



    void MoveBodiesToTOI(entt::registry& registry, const Contact& contact, float toi)
    {
        auto& transformA = registry.get<TransformComponent>(contact.entityA);
        auto& bodyA = registry.get<PhysicsBodyComponent>(contact.entityA);

        auto& transformB = registry.get<TransformComponent>(contact.entityB);
        auto& bodyB = registry.get<PhysicsBodyComponent>(contact.entityB);

        if (bodyA.type == PhysicsBodyType::Dynamic)
            transformA.position = GameMath::Lerp(bodyA.previousPosition, bodyA.targetPosition, toi);

        if (bodyB.type == PhysicsBodyType::Dynamic)
            transformB.position = GameMath::Lerp(bodyB.previousPosition, bodyB.targetPosition, toi);
    }




    void SolveCCD(entt::registry& registry)
    {
        auto view = registry.view<TransformComponent, PhysicsBodyComponent, SphereColliderComponent>();

        std::vector<entt::entity> entities;
        entities.reserve(view.size_hint());

        for (auto [entity, transform, body, sphere] : view.each())
            entities.push_back(entity);

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

                if (!bodyA.enableCCD && !bodyB.enableCCD)
                    continue;

                if (bodyA.invMass == 0.0f && bodyB.invMass == 0.0f)
                    continue;

                Contact contact{};
                float toi = 1.0f;

                if (SweptSphereSphere(entityA, entityB, transformA, bodyA, sphereA, transformB, bodyB, sphereB, contact, toi))
                {
                    MoveBodiesToTOI(registry, contact, toi);
                    ResolveContactVelocity(registry, contact);
                }
            }
        }
    }

    void StoreTargetPositions(entt::registry& registry)
    {
        auto view = registry.view<TransformComponent, PhysicsBodyComponent>();

        for (auto [entity, transform, body] : view.each())
        {
            body.targetPosition = transform.position;
        }
    }





    void IntegratePositionsForEntities(entt::registry& registry, entt::entity entityA, entt::entity entityB, float dt)
    {
        if (registry.valid(entityA) && registry.all_of<TransformComponent, PhysicsBodyComponent>(entityA))
        {
            auto& transformA = registry.get<TransformComponent>(entityA);
            auto& bodyA = registry.get<PhysicsBodyComponent>(entityA);

            if (bodyA.enabled && bodyA.type == PhysicsBodyType::Dynamic)
                transformA.position = GameMath::Add(transformA.position, GameMath::Mul(bodyA.linearVelocity, dt));
        }

        if (registry.valid(entityB) && registry.all_of<TransformComponent, PhysicsBodyComponent>(entityB))
        {
            auto& transformB = registry.get<TransformComponent>(entityB);
            auto& bodyB = registry.get<PhysicsBodyComponent>(entityB);

            if (bodyB.enabled && bodyB.type == PhysicsBodyType::Dynamic)
                transformB.position = GameMath::Add(transformB.position, GameMath::Mul(bodyB.linearVelocity, dt));
        }
    }


    void SolveTOI(entt::registry& registry, float dt)
    {
        std::vector<CollisionPair> pairs;
        BuildBroadphasePairs(registry, pairs);

        float bestTOI = 1.0f;
        Contact bestContact{};
        bool foundHit = false;

        for (const CollisionPair& pair : pairs)
        {
            entt::entity entityA = pair.entityA;
            entt::entity entityB = pair.entityB;

            if (!registry.all_of<PhysicsBodyComponent>(entityA) || !registry.all_of<PhysicsBodyComponent>(entityB))
                continue;

            auto& bodyA = registry.get<PhysicsBodyComponent>(entityA);
            auto& bodyB = registry.get<PhysicsBodyComponent>(entityB);

            if (!bodyA.enableCCD && !bodyB.enableCCD)
                continue;

            Contact contact{};
            float toi = 1.0f;

            if (GenerateSweptContact(registry, entityA, entityB, contact, toi))
            {
                if (toi < bestTOI)
                {
                    bestTOI = toi;
                    bestContact = contact;
                    foundHit = true;
                }
            }
        }

        if (!foundHit)
            return;

        MoveBodiesToTOI(registry, bestContact, bestTOI);
        ResolveContactVelocity(registry, bestContact);

        const float remainingTime = dt * (1.0f - bestTOI);
        IntegratePositionsForEntities(registry, bestContact.entityA, bestContact.entityB, remainingTime);
    }


    BoundsComponent BuildSphereBounds(const TransformComponent& transform, const PhysicsBodyComponent& body, const SphereColliderComponent& sphere)
    {
        BoundsComponent bounds{};

        const DirectX::XMFLOAT3 center = GetSphereCenterWorld(transform, body, sphere);
        const float r = sphere.radius;

        bounds.center = center;
        bounds.extents = { r, r, r };

        bounds.mins = { center.x - r, center.y - r, center.z - r };
        bounds.maxs = { center.x + r, center.y + r, center.z + r };

        return bounds;
    }


    void UpdateBounds(entt::registry& registry)
    {
        auto view = registry.view<TransformComponent, PhysicsBodyComponent>();

        for (auto [entity, transform, body] : view.each())
        {
            BoundsComponent bounds{};

            if (!BuildEntityBounds(registry, entity, bounds))
                continue;

            if (registry.all_of<BoundsComponent>(entity))
            {
                auto& existingBounds = registry.get<BoundsComponent>(entity);
                existingBounds = bounds;
            }
            else
            {
                registry.emplace<BoundsComponent>(entity, bounds);
            }
        }
    }


    bool BoundsIntersect(const BoundsComponent& a, const BoundsComponent& b)
    {
        if (a.maxs.x < b.mins.x || a.mins.x > b.maxs.x)
            return false;

        if (a.maxs.y < b.mins.y || a.mins.y > b.maxs.y)
            return false;

        if (a.maxs.z < b.mins.z || a.mins.z > b.maxs.z)
            return false;

        return true;
    }

    void BuildBroadphasePairs(entt::registry& registry, std::vector<CollisionPair>& pairs)
    {
        pairs.clear();

        auto view = registry.view<TransformComponent, PhysicsBodyComponent, BoundsComponent>();

        std::vector<entt::entity> entities;
        entities.reserve(view.size_hint());

        for (auto [entity, transform, body, bounds] : view.each())
            entities.push_back(entity);

        m_lastPotentialPairs = 0;
        m_lastBroadphasePairs = 0;

        for (size_t i = 0; i < entities.size(); ++i)
        {
            for (size_t j = i + 1; j < entities.size(); ++j)
            {
                ++m_lastPotentialPairs;

                entt::entity entityA = entities[i];
                entt::entity entityB = entities[j];

                auto& bodyA = registry.get<PhysicsBodyComponent>(entityA);
                auto& bodyB = registry.get<PhysicsBodyComponent>(entityB);

                if (!bodyA.enabled || !bodyB.enabled)
                    continue;

                if (bodyA.invMass == 0.0f && bodyB.invMass == 0.0f)
                    continue;

                auto& boundsA = registry.get<BoundsComponent>(entityA);
                auto& boundsB = registry.get<BoundsComponent>(entityB);

                if (!BoundsIntersect(boundsA, boundsB))
                    continue;

                pairs.push_back(CollisionPair{ entityA, entityB });
                ++m_lastBroadphasePairs;
            }
        }
    }

    bool GenerateContact(entt::registry& registry, entt::entity entityA, entt::entity entityB, Contact& contact)
    {
        ++m_lastNarrowphaseTests;

        if (!registry.valid(entityA) || !registry.valid(entityB))
            return false;

        if (!registry.all_of<TransformComponent, PhysicsBodyComponent>(entityA))
            return false;

        if (!registry.all_of<TransformComponent, PhysicsBodyComponent>(entityB))
            return false;

        const bool aIsSphere = registry.all_of<SphereColliderComponent>(entityA);
        const bool bIsSphere = registry.all_of<SphereColliderComponent>(entityB);

        if (aIsSphere && bIsSphere)
        {
            ++m_lastSphereSphereTests;

            auto& transformA = registry.get<TransformComponent>(entityA);
            auto& bodyA = registry.get<PhysicsBodyComponent>(entityA);
            auto& sphereA = registry.get<SphereColliderComponent>(entityA);

            auto& transformB = registry.get<TransformComponent>(entityB);
            auto& bodyB = registry.get<PhysicsBodyComponent>(entityB);
            auto& sphereB = registry.get<SphereColliderComponent>(entityB);

            return BuildSphereSphereContact(entityA, entityB, transformA, bodyA, sphereA, transformB, bodyB, sphereB, contact);
        }

        return false;
    }


    bool GenerateSweptContact(entt::registry& registry, entt::entity entityA, entt::entity entityB, Contact& contact, float& toi)
    {
        if (!registry.valid(entityA) || !registry.valid(entityB))
            return false;

        const bool aIsSphere = registry.all_of<TransformComponent, PhysicsBodyComponent, SphereColliderComponent>(entityA);
        const bool bIsSphere = registry.all_of<TransformComponent, PhysicsBodyComponent, SphereColliderComponent>(entityB);

        if (aIsSphere && bIsSphere)
        {
            auto& transformA = registry.get<TransformComponent>(entityA);
            auto& bodyA = registry.get<PhysicsBodyComponent>(entityA);
            auto& sphereA = registry.get<SphereColliderComponent>(entityA);

            auto& transformB = registry.get<TransformComponent>(entityB);
            auto& bodyB = registry.get<PhysicsBodyComponent>(entityB);
            auto& sphereB = registry.get<SphereColliderComponent>(entityB);

            return SweptSphereSphere(entityA, entityB, transformA, bodyA, sphereA, transformB, bodyB, sphereB, contact, toi);
        }

        return false;
    }


    PhysicsShapeType GetPhysicsShapeType(entt::registry& registry, entt::entity entity)
    {
        if (!registry.valid(entity))
            return PhysicsShapeType::None;

        if (registry.all_of<SphereColliderComponent>(entity))
            return PhysicsShapeType::Sphere;

        if (registry.all_of<BoxColliderComponent>(entity))
            return PhysicsShapeType::Box;

        return PhysicsShapeType::None;
    }


    bool SupportSphere(entt::registry& registry, entt::entity entity, const DirectX::XMFLOAT3& directionWorld, float bias, DirectX::XMFLOAT3& outPoint)
    {
        if (!registry.all_of<TransformComponent, PhysicsBodyComponent, SphereColliderComponent>(entity))
            return false;

        auto& transform = registry.get<TransformComponent>(entity);
        auto& body = registry.get<PhysicsBodyComponent>(entity);
        auto& sphere = registry.get<SphereColliderComponent>(entity);

        DirectX::XMFLOAT3 dir = GameMath::Normalize(directionWorld);
        DirectX::XMFLOAT3 center = GetSphereCenterWorld(transform, body, sphere);
        float radius = sphere.radius + bias;

        outPoint = GameMath::Add(center, GameMath::Mul(dir, radius));
        return true;
    }


    bool SupportPoint(entt::registry& registry, entt::entity entity, const DirectX::XMFLOAT3& directionWorld, float bias, DirectX::XMFLOAT3& outPoint)
    {
        PhysicsShapeType type = GetPhysicsShapeType(registry, entity);

        switch (type)
        {
        case PhysicsShapeType::Sphere:
            return SupportSphere(registry, entity, directionWorld, bias, outPoint);

        case PhysicsShapeType::Box:
            return SupportBox(registry, entity, directionWorld, bias, outPoint);

        default:
            return false;
        }
    }


    bool SupportMinkowski(entt::registry& registry, entt::entity entityA, entt::entity entityB, const DirectX::XMFLOAT3& directionWorld, float bias, DirectX::XMFLOAT3& outPoint)
    {
        DirectX::XMFLOAT3 pointA{};
        DirectX::XMFLOAT3 pointB{};
        DirectX::XMFLOAT3 opposite = GameMath::Mul(directionWorld, -1.0f);

        if (!SupportPoint(registry, entityA, directionWorld, bias, pointA))
            return false;

        if (!SupportPoint(registry, entityB, opposite, bias, pointB))
            return false;

        outPoint = GameMath::Sub(pointA, pointB);
        return true;
    }

    float FastestLinearSpeedSphere(const PhysicsBodyComponent& body, const DirectX::XMFLOAT3& directionWorld)
    {
        return 0.0f;
    }


    float GetFastestLinearSpeed(entt::registry& registry, entt::entity entity, const DirectX::XMFLOAT3& directionWorld)
    {
        if (!registry.valid(entity))
            return 0.0f;

        PhysicsShapeType type = GetPhysicsShapeType(registry, entity);

        switch (type)
        {
        case PhysicsShapeType::Sphere:
            return 0.0f;

        case PhysicsShapeType::Box:
            return FastestLinearSpeedBox(registry, entity, directionWorld);

        default:
            return 0.0f;
        }
    }


    bool BuildEntityBounds(entt::registry& registry, entt::entity entity, BoundsComponent& outBounds)
    {
        PhysicsShapeType type = GetPhysicsShapeType(registry, entity);

        switch (type)
        {
        case PhysicsShapeType::Sphere:
        {
            if (!registry.all_of<TransformComponent, PhysicsBodyComponent, SphereColliderComponent>(entity))
                return false;

            auto& transform = registry.get<TransformComponent>(entity);
            auto& body = registry.get<PhysicsBodyComponent>(entity);
            auto& sphere = registry.get<SphereColliderComponent>(entity);

            outBounds = BuildSphereBounds(transform, body, sphere);
            return true;
        }

        case PhysicsShapeType::Box:
        {
            if (!registry.all_of<TransformComponent, PhysicsBodyComponent, BoxColliderComponent>(entity))
                return false;

            auto& transform = registry.get<TransformComponent>(entity);
            auto& body = registry.get<PhysicsBodyComponent>(entity);
            auto& box = registry.get<BoxColliderComponent>(entity);

            outBounds = BuildBoxBounds(transform, body, box);
            return true;
        }

        default:
            return false;
        }
    }

    bool GetEntityCenterOfMassWorld(entt::registry& registry, entt::entity entity, DirectX::XMFLOAT3& outCenter)
    {
        if (!registry.all_of<TransformComponent, PhysicsBodyComponent>(entity))
            return false;

        auto& transform = registry.get<TransformComponent>(entity);
        auto& body = registry.get<PhysicsBodyComponent>(entity);

        PhysicsShapeType type = GetPhysicsShapeType(registry, entity);

        switch (type)
        {
        case PhysicsShapeType::Sphere:
        {
            if (!registry.all_of<SphereColliderComponent>(entity))
                return false;

            auto& sphere = registry.get<SphereColliderComponent>(entity);
            outCenter = GameMath::LocalToWorldPoint(transform, body, sphere.centerOfMassLocal);
            return true;
        }

        case PhysicsShapeType::Box:
        {
            if (!registry.all_of<BoxColliderComponent>(entity))
                return false;

            auto& box = registry.get<BoxColliderComponent>(entity);
            outCenter = GameMath::LocalToWorldPoint(transform, body, box.centerLocal);
            return true;
        }

        case PhysicsShapeType::Convex:
            return false;

        default:
            return false;
        }
    }


    std::array<DirectX::XMFLOAT3, 8> GetBoxLocalCorners(const BoxColliderComponent& box)
    {
        const float cx = box.centerLocal.x;
        const float cy = box.centerLocal.y;
        const float cz = box.centerLocal.z;

        const float hx = box.halfExtents.x;
        const float hy = box.halfExtents.y;
        const float hz = box.halfExtents.z;

        return
        {
            DirectX::XMFLOAT3{ cx - hx, cy - hy, cz - hz },
            DirectX::XMFLOAT3{ cx + hx, cy - hy, cz - hz },
            DirectX::XMFLOAT3{ cx - hx, cy + hy, cz - hz },
            DirectX::XMFLOAT3{ cx + hx, cy + hy, cz - hz },
            DirectX::XMFLOAT3{ cx - hx, cy - hy, cz + hz },
            DirectX::XMFLOAT3{ cx + hx, cy - hy, cz + hz },
            DirectX::XMFLOAT3{ cx - hx, cy + hy, cz + hz },
            DirectX::XMFLOAT3{ cx + hx, cy + hy, cz + hz }
        };
    }


    bool SupportBox(entt::registry& registry, entt::entity entity, const DirectX::XMFLOAT3& directionWorld, float bias, DirectX::XMFLOAT3& outPoint)
    {
        if (!registry.all_of<TransformComponent, PhysicsBodyComponent, BoxColliderComponent>(entity))
            return false;

        auto& transform = registry.get<TransformComponent>(entity);
        auto& body = registry.get<PhysicsBodyComponent>(entity);
        auto& box = registry.get<BoxColliderComponent>(entity);

        DirectX::XMFLOAT3 dir = GameMath::Normalize(directionWorld);
        auto corners = GetBoxLocalCorners(box);

        DirectX::XMFLOAT3 bestPoint = GameMath::LocalToWorldPoint(transform, body, corners[0]);
        float bestDistance = GameMath::Dot(bestPoint, dir);

        for (int i = 1; i < 8; ++i)
        {
            DirectX::XMFLOAT3 point = GameMath::LocalToWorldPoint(transform, body, corners[i]);
            float distance = GameMath::Dot(point, dir);

            if (distance > bestDistance)
            {
                bestDistance = distance;
                bestPoint = point;
            }
        }

        if (bias != 0.0f)
            bestPoint = GameMath::Add(bestPoint, GameMath::Mul(dir, bias));

        outPoint = bestPoint;
        return true;
    }


    BoundsComponent BuildBoxBounds(const TransformComponent& transform, const PhysicsBodyComponent& body, const BoxColliderComponent& box)
    {
        BoundsComponent bounds{};
        auto corners = GetBoxLocalCorners(box);

        DirectX::XMFLOAT3 first = GameMath::LocalToWorldPoint(transform, body, corners[0]);
        bounds.center = first;
        bounds.mins = first;
        bounds.maxs = first;
        bounds.extents = { 0.0f, 0.0f, 0.0f };

        for (int i = 1; i < 8; ++i)
        {
            DirectX::XMFLOAT3 p = GameMath::LocalToWorldPoint(transform, body, corners[i]);

            bounds.mins.x = std::min(bounds.mins.x, p.x);
            bounds.mins.y = std::min(bounds.mins.y, p.y);
            bounds.mins.z = std::min(bounds.mins.z, p.z);

            bounds.maxs.x = std::max(bounds.maxs.x, p.x);
            bounds.maxs.y = std::max(bounds.maxs.y, p.y);
            bounds.maxs.z = std::max(bounds.maxs.z, p.z);
        }

        bounds.center = GameMath::Mul(GameMath::Add(bounds.mins, bounds.maxs), 0.5f);
        bounds.extents = GameMath::Mul(GameMath::Sub(bounds.maxs, bounds.mins), 0.5f);

        return bounds;
    }


    void UpdateBoxMassProperties(PhysicsBodyComponent& body, const BoxColliderComponent& box)
    {
        if (body.type == PhysicsBodyType::Static || body.invMass == 0.0f)
        {
            body.mass = 0.0f;
            body.invMass = 0.0f;
            body.invInertiaLocal = { 0.0f, 0.0f, 0.0f };
            body.linearVelocity = { 0.0f, 0.0f, 0.0f };
            body.angularVelocity = { 0.0f, 0.0f, 0.0f };
            body.massPropertiesDirty = false;
            return;
        }

        body.mass = std::max(body.mass, 0.0001f);
        body.invMass = 1.0f / body.mass;

        const float w = box.halfExtents.x * 2.0f;
        const float h = box.halfExtents.y * 2.0f;
        const float d = box.halfExtents.z * 2.0f;

        const float ix = (1.0f / 12.0f) * body.mass * (h * h + d * d);
        const float iy = (1.0f / 12.0f) * body.mass * (w * w + d * d);
        const float iz = (1.0f / 12.0f) * body.mass * (w * w + h * h);

        body.invInertiaLocal =
        {
            ix > 0.00001f ? 1.0f / ix : 0.0f,
            iy > 0.00001f ? 1.0f / iy : 0.0f,
            iz > 0.00001f ? 1.0f / iz : 0.0f
        };
        
        body.massPropertiesDirty = false;
    }


    DirectX::XMFLOAT3 GetBoxCenterWorld(const TransformComponent& transform, const PhysicsBodyComponent& body, const BoxColliderComponent& box)
    {
        return GameMath::LocalToWorldPoint(transform, body, box.centerLocal);
    }


    float FastestLinearSpeedBox(entt::registry& registry, entt::entity entity, const DirectX::XMFLOAT3& directionWorld)
    {
        if (!registry.all_of<TransformComponent, PhysicsBodyComponent, BoxColliderComponent>(entity))
            return 0.0f;

        auto& transform = registry.get<TransformComponent>(entity);
        auto& body = registry.get<PhysicsBodyComponent>(entity);
        auto& box = registry.get<BoxColliderComponent>(entity);

        auto corners = GetBoxLocalCorners(box);
        DirectX::XMFLOAT3 centerWorld = GetBoxCenterWorld(transform, body, box);
        DirectX::XMFLOAT3 dir = GameMath::Normalize(directionWorld);

        float maxSpeed = 0.0f;

        for (int i = 0; i < 8; ++i)
        {
            DirectX::XMFLOAT3 pointWorld = GameMath::LocalToWorldPoint(transform, body, corners[i]);
            DirectX::XMFLOAT3 r = GameMath::Sub(pointWorld, centerWorld);
            DirectX::XMFLOAT3 velocityAtPoint = GameMath::Cross(body.angularVelocity, r);
            float speed = GameMath::Dot(velocityAtPoint, dir);

            if (speed > maxSpeed)
                maxSpeed = speed;
        }
        
        return maxSpeed;
    }


	// TODO: move this to a config file or something
    DirectX::XMFLOAT3 gravity = { 0.0f, -1.0f, 0.0f };

    float restingVelocityThreshold = 0.5f;

	// counters
    uint32_t m_lastPotentialPairs = 0;
    uint32_t m_lastBroadphasePairs = 0;
    uint32_t m_lastNarrowphaseContacts = 0;

    uint32_t m_lastNarrowphaseTests = 0;
    uint32_t m_lastSphereSphereTests = 0;
    uint32_t m_lastGeneratedContacts = 0;
};
