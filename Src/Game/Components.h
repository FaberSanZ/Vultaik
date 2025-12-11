#pragma once
#include "Buffer.h"
#include "EngineData.h"
#include "GameMath.h"


using namespace Games;



struct Transform
{
	uint32_t id;
	std::string name;
    Float3 position;
    Float3 rotation;
    Float3 scale;
};

struct InstanceComponent
{
	std::vector<Transform> words;
};



struct TerrainComponent
{
    uint32_t width;
    uint32_t height;
    float scale;
	std::vector<float> heightMap;
};


struct CameraComponent
{
    Float3 position;
    Float3 target;
    Float3 up;
};

// TODO: Expand MeshPart to include node, sub-mesh info, etc.
//class MeshPart 
//{
//    public:
//    MeshPart() = default;
//    ~MeshPart() = default;
//
//};


class Mesh
{
public:
    Mesh() = default;
    ~Mesh() = default;

};

enum class ShapeType
{
    Cube,
    Sphere,
    Cylinder,
    Plane,
    Polygon,
    Null,
    Count
};

enum class MeshType
{
    Static,
    Dynamic,
    Count
};

struct MeshComponent
{
	ShapeType shapeType;
	MeshType meshType;
    //std::vector<uint32_t> Indices{};
    //std::vector<Graphics::VertexPositionColor> Vertices{};
	Mesh mesh;
	bool dirty = false; // Indicates if the mesh needs to be updated
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