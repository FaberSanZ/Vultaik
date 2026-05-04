#pragma once
// #include "GameMath.h"
#include <DirectXMath.h>
#include <cstdint>
#include <string>



struct TransformComponent
{
    DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
};


enum class ShapeType
{
	Triangle,
    Plane,
    Cube,
    Sphere,
    Null,
    Count
};


struct MeshComponent
{
    ShapeType shapeType = ShapeType::Null;
};

struct MaterialComponent
{
    DirectX::XMFLOAT3 baseColor = { 0.8f, 0.8f, 0.8f };
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;
    int32_t textureId = 0;
};



enum class PhysicsBodyType
{
    Static,
    Dynamic,
    Kinematic
};

struct RigidbodyComponent
{
    PhysicsBodyType type = PhysicsBodyType::Static;

    DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 linearVelocity = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 linearAcceleration = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 linearImpulse = { 0.0f, 0.0f, 0.0f };

	float mass = 1.0f;
	float invMass = 1.0f;
    float linearDamping = 0.0f;

};

