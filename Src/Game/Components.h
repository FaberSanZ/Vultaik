#pragma once
#include <entt.hpp>


struct TransformComponent
{
    float x;
    float y;
    float z;

	float rotationX;
    float rotationY;
	float rotationZ;
};

struct InstanceComponent
{
	std::vector<DirectX::XMFLOAT3> instancePositions;
};

struct MeshComponent
{
	std::vector<DirectX::XMFLOAT3> vertices;
	std::vector<uint32_t> indices;
};

enum class ShapeType
{
    Cube,
    Sphere,
    Cylinder,
    Plane
};


struct ShapeComponent
{
	ShapeType type;
	uint32_t subdivisions; // For sphere or cylinder detail
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