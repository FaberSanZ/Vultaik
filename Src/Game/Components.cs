using System.Numerics;

namespace Vultaik;

public struct NameComponent
{
    public string? Name;
}

public struct TransformComponent
{
    public Vector2 Position;
    public float Rotation;
    public Vector2 Scale;
}

public struct CameraComponent
{
    public float Size;
    public float Zoom;
    public float NearPlane;
    public float FarPlane;
}

public struct PrimaryCameraComponent
{
}

public enum BodyType
{
    Static,
    Kinematic,
    Dynamic
}

public struct RigidbodyComponent
{
    public BodyType Type;

    public Vector2 LinearVelocity;
    public float AngularVelocity;

    public Vector2 Force;
    public float Torque;

    public float Mass;
    public float Inertia;

    public float InverseMass;
    public float InverseInertia;

    public float LinearDamping;
    public float AngularDamping;

    public float GravityScale;
}

public struct CircleColliderComponent
{
    public float Radius;
    public float Density;
    public float Friction;
    public float Restitution;
}