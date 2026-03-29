#pragma once
#include "GameMath.h"
#include <string>



struct TransformComponent
{
    uint32_t id;
    Float3 position;
    Float3 rotation;
    Float3 scale;

    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;
};




struct CameraComponent
{
    Float3 position;
    Float3 target;
    Float3 up;
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
