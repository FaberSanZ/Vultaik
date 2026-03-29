#pragma once
#include "GameMath.h"
#include <string>



struct TransformComponent
{
    uint32_t id;
    Float2 position;
    Float2 scale;
    float rotation;

    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;
};

struct VelocityComponent
{
    Float2 velocity;
    VelocityComponent() = default;
    VelocityComponent(const VelocityComponent&) = default;
    VelocityComponent(const Float2& velocity) : velocity(velocity) { }
};

struct AccelerationComponent
{
    Float2 acceleration;
    AccelerationComponent() = default;
    AccelerationComponent(const AccelerationComponent&) = default;
    AccelerationComponent(const Float2& acceleration) : acceleration(acceleration) { }
};

struct Rigidbody2DComponent
{
    float mass;
    bool isKinematic;
    Rigidbody2DComponent() = default;
    Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
    Rigidbody2DComponent(float mass, bool isKinematic) : mass(mass), isKinematic(isKinematic) { }
};

struct ParticleComponent
{
    Float2 position;
    Float2 velocity;
    Float2 acceleraton;
    Float2 scale;
    float rotation;
    float mass;
    ParticleComponent() = default;
    ParticleComponent(const ParticleComponent&) = default;
    ParticleComponent(const Float2& position, const Float2& velocity, const Float2& acceleration, float mass)
		: position(position), velocity(velocity), acceleraton(acceleration), mass(mass) { }

};


enum class ShapeType
{
	Triangle,
    Cuad,
	Pentagon,
	Hexagon,
	Circle,
	Polygon,
    Null,
    Count
};

enum class MeshType
{
    Static,
    Dynamic,
    Kinematic,
    Count
};

struct MeshComponent
{
    ShapeType shapeType;
    MeshType meshType; // TODO: Implement mesh type for static and dynamic meshes, which can affect how they are rendered and updated in the physics system
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
