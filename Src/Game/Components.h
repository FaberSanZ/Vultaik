#pragma once
#include "Buffer.h"
#include "EngineData.h"
#include "GameMath.h"


using namespace Games;



struct Transform
{
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
};

struct InstanceComponent
{
	std::vector<Transform> words;
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

struct TerrainComponent
{
    uint32_t width;
    uint32_t height;
    float scale;
	std::vector<float> heightMap;
};


struct CameraComponent
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 target;
    DirectX::XMFLOAT3 up;
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
    Mesh(const Graphics::Buffer& vBuffer, const Graphics::Buffer& iBuffer, uint32_t iCount, Graphics::Buffer instanceBuffer)
		: vertexBuffer(vBuffer), indexBuffer(iBuffer), indexCount(iCount), InstanceBuffer(instanceBuffer)
    {
    }
    Graphics::Buffer vertexBuffer;
    Graphics::Buffer indexBuffer;
    uint32_t indexCount = 0;
    Graphics::Buffer InstanceBuffer; // For instanced rendering

    std::string name{ "mesh empy" };
};

struct MeshComponent
{
	ShapeType shapeType;
    std::vector<uint32_t> Indices{};
    std::vector<Graphics::VertexPositionColor> Vertices{};
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