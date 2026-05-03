#pragma once
#include <cmath>
#include <DirectXMath.h>
#include "Components.h"


struct Float2
{
	float x;
	float y;
	constexpr Float2(float x, float y) noexcept : x(x), y(y) {}
	Float2() : x(0.0f), y(0.0f) {}
};


struct Float3
{
	float x;
	float y;
	float z;
	constexpr Float3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
	Float3() : x(0.0f), y(0.0f), z(0.0f) {}
};

class GameMath
{
public:
	static inline float DegreesToRadians(float degrees)
	{
		return degrees * (PI / 180.0f);
	}

	static constexpr float PI = 3.141592654f;
	static constexpr float PI2 = 6.283185307f;
	static constexpr float PIDIV2 = 1.570796327f;
	static constexpr float PIDIV4 = 0.785398163f;


    static inline DirectX::XMVECTOR Load3(const DirectX::XMFLOAT3& v)
    {
        return DirectX::XMLoadFloat3(&v);
    }

    static inline DirectX::XMVECTOR LoadQuat(const DirectX::XMFLOAT4& q)
    {
        return DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&q));
    }

    static inline DirectX::XMFLOAT3 Store3(DirectX::FXMVECTOR v)
    {
        DirectX::XMFLOAT3 result{};
        DirectX::XMStoreFloat3(&result, v);
        return result;
    }

    static inline DirectX::XMFLOAT3 LocalToWorldPoint(const TransformComponent& transform, const PhysicsBodyComponent& body, const DirectX::XMFLOAT3& localPoint)
    {
        using namespace DirectX;

        XMVECTOR position = XMLoadFloat3(&transform.position);
        XMVECTOR orientation = LoadQuat(body.orientation);
        XMVECTOR local = XMLoadFloat3(&localPoint);

        XMVECTOR world = XMVector3Rotate(local, orientation) + position;

        return Store3(world);
    }

    static inline DirectX::XMFLOAT3 WorldToLocalPoint(const TransformComponent& transform, const PhysicsBodyComponent& body, const DirectX::XMFLOAT3& worldPoint)
    {
        using namespace DirectX;

        XMVECTOR position = XMLoadFloat3(&transform.position);
        XMVECTOR orientation = LoadQuat(body.orientation);
        XMVECTOR inverseOrientation = XMQuaternionConjugate(orientation);

        XMVECTOR world = XMLoadFloat3(&worldPoint);
        XMVECTOR local = XMVector3Rotate(world - position, inverseOrientation);

        return Store3(local);
    }

    static inline DirectX::XMFLOAT3 GetCenterOfMassWorld(const TransformComponent& transform, const PhysicsBodyComponent& body, const SphereColliderComponent& collider)
    {
        return LocalToWorldPoint(transform, body, collider.centerOfMassLocal);
    }


    static inline DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return { a.x + b.x, a.y + b.y,a.z + b.z };
    }

    static inline DirectX::XMFLOAT3 Mul(const DirectX::XMFLOAT3& v, float s)
    {
        return { v.x * s, v.y * s, v.z * s };
    }


    static inline DirectX::XMFLOAT3 Sub(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return { a.x - b.x,a.y - b.y,a.z - b.z };
    }

    static inline float Dot(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static inline float LengthSq(const DirectX::XMFLOAT3& v)
    {
        return Dot(v, v);
    }

    static inline float Max3(float x, float y, float z)
    {
        return std::max(x, std::max(y, z));
    }


    static inline float Length(const DirectX::XMFLOAT3& v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    static inline DirectX::XMFLOAT3 Div(const DirectX::XMFLOAT3& v, float s)
    {
        return { v.x / s, v.y / s, v.z / s };
    }

    static inline DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& v)
    {
        const float len = Length(v);

        if (len <= 0.00001f)
            return { 0.0f, 1.0f, 0.0f };

        return Div(v, len);
    }


    static inline DirectX::XMFLOAT3 Cross(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return
        {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

};



