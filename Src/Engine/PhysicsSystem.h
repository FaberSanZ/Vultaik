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
#include "entt.hpp"

class PhysicsSystem
{
public:
    void OnInitialize(entt::registry&)
    {
    }

    void OnUpdate(entt::registry& registry, GameTime time)
    {
        auto view = registry.view<TransformComponent>();

        for (auto [entity, transform] : view.each())
        {
            auto& transform = registry.get<TransformComponent>(entity);
            auto& body = registry.get<PhysicsBodyComponent>(entity);
            auto& collider = registry.get<SphereColliderComponent>(entity);

            if (!body.enabled)
                continue;

			DirectX::XMFLOAT3 centerOfMassWorld = GetCenterOfMassWorld(transform, body, collider);
        }

    }

    void OnImGui(entt::registry& registry)
    {
  //      auto view = registry.view<TransformComponent, PhysicsBodyComponent, SphereColliderComponent>();
  //      for (auto entity : view)
  //      {
  //          auto& transform = registry.get<TransformComponent>(entity);
  //          auto& body = registry.get<PhysicsBodyComponent>(entity);
  //          auto& collider = registry.get<SphereColliderComponent>(entity);

  //          ImGui::PushID(static_cast<int>(entity));
  //          ImGui::Text("Entity %d", static_cast<int>(entity));
  //          ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
  //          ImGui::DragFloat4("Orientation (Quaternion)", &body.orientation.x, 0.1f);
  //          ImGui::DragFloat("Radius", &collider.radius, 0.1f);
  //          ImGui::Separator();
  //          ImGui::PopID();
		//}
	}


	// Helper functions can be added here for collision detection, response, etc.
    // 
    inline DirectX::XMVECTOR Load3(const DirectX::XMFLOAT3& v)
    {
        return DirectX::XMLoadFloat3(&v);
    }

    inline DirectX::XMVECTOR LoadQuat(const DirectX::XMFLOAT4& q)
    {
        return DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&q));
    }

    inline DirectX::XMFLOAT3 Store3(DirectX::FXMVECTOR v)
    {
        DirectX::XMFLOAT3 result{};
        DirectX::XMStoreFloat3(&result, v);
        return result;
    }

    inline DirectX::XMFLOAT3 LocalToWorldPoint(const TransformComponent& transform, const PhysicsBodyComponent& body, const DirectX::XMFLOAT3& localPoint)
    {
        using namespace DirectX;

        XMVECTOR position = XMLoadFloat3(&transform.position);
        XMVECTOR orientation = LoadQuat(body.orientation);
        XMVECTOR local = XMLoadFloat3(&localPoint);

        XMVECTOR world = XMVector3Rotate(local, orientation) + position;

        return Store3(world);
    }

    inline DirectX::XMFLOAT3 WorldToLocalPoint(const TransformComponent& transform, const PhysicsBodyComponent& body, const DirectX::XMFLOAT3& worldPoint)
    {
        using namespace DirectX;

        XMVECTOR position = XMLoadFloat3(&transform.position);
        XMVECTOR orientation = LoadQuat(body.orientation);
        XMVECTOR inverseOrientation = XMQuaternionConjugate(orientation);

        XMVECTOR world = XMLoadFloat3(&worldPoint);
        XMVECTOR local = XMVector3Rotate(world - position, inverseOrientation);

        return Store3(local);
    }

    inline DirectX::XMFLOAT3 GetCenterOfMassWorld(const TransformComponent& transform, const PhysicsBodyComponent& body, const SphereColliderComponent& collider)
    {
        return LocalToWorldPoint(transform, body, collider.centerOfMassLocal);
    }

};
