#pragma once

#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <DirectXMath.h>
#include "Components.h"

class GameMath
{
public:
    static constexpr float PI = 3.141592654f;
    static constexpr float PI2 = 6.283185307f;
    static constexpr float PIDIV2 = 1.570796327f;
    static constexpr float PIDIV4 = 0.785398163f;

public:
    static inline float DegreesToRadians(float degrees)
    {
        return degrees * (PI / 180.0f);
    }

    static inline DirectX::XMVECTOR Load3(const DirectX::XMFLOAT3& v)
    {
        return DirectX::XMLoadFloat3(&v);
    }

    static inline DirectX::XMVECTOR Load4(const DirectX::XMFLOAT4& v)
    {
        return DirectX::XMLoadFloat4(&v);
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

    static inline DirectX::XMFLOAT4 Store4(DirectX::FXMVECTOR v)
    {
        DirectX::XMFLOAT4 result{};
        DirectX::XMStoreFloat4(&result, v);
        return result;
    }

public:
    static inline DirectX::XMVECTOR AddV(DirectX::FXMVECTOR a, DirectX::FXMVECTOR b)
    {
        return DirectX::XMVectorAdd(a, b);
    }

    static inline DirectX::XMVECTOR SubV(DirectX::FXMVECTOR a, DirectX::FXMVECTOR b)
    {
        return DirectX::XMVectorSubtract(a, b);
    }

    static inline DirectX::XMVECTOR MulV(DirectX::FXMVECTOR v, float s)
    {
        return DirectX::XMVectorScale(v, s);
    }

    static inline DirectX::XMVECTOR DivV(DirectX::FXMVECTOR v, float s)
    {
        return DirectX::XMVectorScale(v, 1.0f / s);
    }

    static inline DirectX::XMVECTOR CrossV(DirectX::FXMVECTOR a, DirectX::FXMVECTOR b)
    {
        return DirectX::XMVector3Cross(a, b);
    }

    static inline float DotV(DirectX::FXMVECTOR a, DirectX::FXMVECTOR b)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector3Dot(a, b));
    }

    static inline float LengthSqV(DirectX::FXMVECTOR v)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(v));
    }

    static inline float LengthV(DirectX::FXMVECTOR v)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector3Length(v));
    }

    static inline DirectX::XMVECTOR NormalizeV(DirectX::FXMVECTOR v)
    {
        const float lenSq = LengthSqV(v);

        if (lenSq <= 0.0000001f)
            return DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        return DirectX::XMVector3Normalize(v);
    }

    static inline DirectX::XMVECTOR LerpV(DirectX::FXMVECTOR a, DirectX::FXMVECTOR b, float t)
    {
        return DirectX::XMVectorLerp(a, b, t);
    }

public:
    static inline DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return Store3(AddV(Load3(a), Load3(b)));
    }

    static inline DirectX::XMFLOAT3 Sub(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return Store3(SubV(Load3(a), Load3(b)));
    }

    static inline DirectX::XMFLOAT3 Mul(const DirectX::XMFLOAT3& v, float s)
    {
        return Store3(MulV(Load3(v), s));
    }

    static inline DirectX::XMFLOAT3 Div(const DirectX::XMFLOAT3& v, float s)
    {
        return Store3(DivV(Load3(v), s));
    }

    static inline float Dot(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return DotV(Load3(a), Load3(b));
    }

    static inline float LengthSq(const DirectX::XMFLOAT3& v)
    {
        return LengthSqV(Load3(v));
    }

    static inline float Length(const DirectX::XMFLOAT3& v)
    {
        return LengthV(Load3(v));
    }

    static inline DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& v)
    {
        return Store3(NormalizeV(Load3(v)));
    }

    static inline DirectX::XMFLOAT3 Cross(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return Store3(CrossV(Load3(a), Load3(b)));
    }

    static inline DirectX::XMFLOAT3 Lerp(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, float t)
    {
        return Store3(LerpV(Load3(a), Load3(b), t));
    }

    static inline float Max3(float x, float y, float z)
    {
        return std::max(x, std::max(y, z));
    }

public:
    static inline DirectX::XMFLOAT3 LocalToWorldPoint(const TransformComponent& transform, const PhysicsBodyComponent& body, const DirectX::XMFLOAT3& localPoint)
    {
        using namespace DirectX;

        XMVECTOR position = XMLoadFloat3(&transform.position);
        XMVECTOR orientation = LoadQuat(body.orientation);
        XMVECTOR local = XMLoadFloat3(&localPoint);
        XMVECTOR rotated = XMVector3Rotate(local, orientation);
        XMVECTOR world = XMVectorAdd(rotated, position);

        return Store3(world);
    }

    static inline DirectX::XMFLOAT3 WorldToLocalPoint(const TransformComponent& transform, const PhysicsBodyComponent& body, const DirectX::XMFLOAT3& worldPoint)
    {
        using namespace DirectX;

        XMVECTOR position = XMLoadFloat3(&transform.position);
        XMVECTOR orientation = LoadQuat(body.orientation);
        XMVECTOR inverseOrientation = XMQuaternionConjugate(orientation);
        XMVECTOR world = XMLoadFloat3(&worldPoint);
        XMVECTOR relative = XMVectorSubtract(world, position);
        XMVECTOR local = XMVector3Rotate(relative, inverseOrientation);

        return Store3(local);
    }

    static inline DirectX::XMFLOAT3 GetCenterOfMassWorld(const TransformComponent& transform, const PhysicsBodyComponent& body, const SphereColliderComponent& collider)
    {
        return LocalToWorldPoint(transform, body, collider.centerOfMassLocal);
    }

public:
    static inline float RandomFloat(float min, float max)
    {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }
};