#pragma once
// #include "GameMath.h"
#include <DirectXMath.h>
#include <cstdint>
#include <string>



struct TransformComponent
{
    uint32_t id = 0;
    DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };

    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;
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

    MaterialComponent() = default;
    MaterialComponent(const DirectX::XMFLOAT3& baseColor, float metallic, float roughness, float ao, int32_t textureId = 0)
        : baseColor(baseColor), metallic(metallic), roughness(roughness), ao(ao), textureId(textureId) {}
};


struct TagComponent
{
    std::string Tag;

    TagComponent() = default;
    TagComponent(const TagComponent&) = default;
    TagComponent(const std::string& tag)
        : Tag(tag) {
    }
};


//  physics 

enum class ColliderType
{
    Sphere,
};

struct SphereColliderComponent 
{
    float radius = 0.5f;

    DirectX::XMFLOAT3 centerOfMassLocal = { 0.0f, 0.0f, 0.0f };
};

enum class PhysicsBodyType 
{
    Static,
    Dynamic,
    Kinematic
};

struct PhysicsBodyComponent
{
    PhysicsBodyType type = PhysicsBodyType::Static;

    DirectX::XMFLOAT4 orientation = { 0.0f, 0.0f, 0.0f, 1.0f };
    DirectX::XMFLOAT3 linearVelocity = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 angularVelocity = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 invInertiaLocal = { 1.0f, 1.0f, 1.0f };


    float restitution = 0.4f;
    float mass = 1.0f;
    float invMass = 1.0f;

    bool useGravity = true;
    bool enabled = true;
    bool massPropertiesDirty = true;

};
