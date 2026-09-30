using System;
using System.Numerics;

namespace Vultaik;

public sealed class Game : GameBase
{
    private Entity _camera;
    private Entity _circle;
    private Mesh2D _circleMesh = null!;

    protected override string Title => "Vultaik Physics";

    protected override void OnInitialize(World world)
    {
        _camera = CreateEntity("Camera");

        ref CameraComponent camera = ref world.Set<CameraComponent>(_camera);
        camera.Size = 10.0f;
        camera.Zoom = 1.0f;
        camera.NearPlane = 0.0f;
        camera.FarPlane = 100.0f;

        world.Set<PrimaryCameraComponent>(_camera);

        _circle = CreateEntity("Circle");

        ref TransformComponent transform = ref world.Get<TransformComponent>(_circle);
        transform.Position = new Vector2(0.0f, 3.0f);
        transform.Rotation = 0.0f;
        transform.Scale = Vector2.One;

        ref RigidbodyComponent rigidbody = ref world.Set<RigidbodyComponent>(_circle);
        rigidbody.Type = BodyType.Dynamic;
        rigidbody.LinearVelocity = Vector2.Zero;
        rigidbody.AngularVelocity = 0.0f;
        rigidbody.Force = Vector2.Zero;
        rigidbody.Torque = 0.0f;

        rigidbody.Mass = 1.0f;
        rigidbody.Inertia = 1.0f;
        rigidbody.InverseMass = 1.0f / rigidbody.Mass;
        rigidbody.InverseInertia = 1.0f / rigidbody.Inertia;

        rigidbody.LinearDamping = 0.0f;
        rigidbody.AngularDamping = 0.0f;
        rigidbody.GravityScale = 1.0f;

        ref CircleColliderComponent collider = ref world.Set<CircleColliderComponent>(_circle);
        collider.Radius = 1.0f;

        _circleMesh = Mesh2D.CreateCircle(
            Renderer.Device,
            1.0f,
            32,
            new Vector4(0.9f, 0.25f, 0.15f, 1.0f)
        );
    }

    protected override void OnUpdate(World world, float deltaTime)
    {
        UpdateCamera(world, deltaTime);
    }

    protected override void OnRender(World world)
    {
        ref TransformComponent transform = ref world.Get<TransformComponent>(_circle);
        Renderer.Draw(_circleMesh, in transform);
    }

    protected override void OnDrawImGui(World world)
    {
        ref TransformComponent transform = ref world.Get<TransformComponent>(_circle);
        ref RigidbodyComponent rigidbody = ref world.Get<RigidbodyComponent>(_circle);

        ImGuiNET.ImGui.Begin("Vultaik Physics");
        ImGuiNET.ImGui.Text($"Position: {transform.Position.X:F2}, {transform.Position.Y:F2}");
        ImGuiNET.ImGui.Text($"Velocity: {rigidbody.LinearVelocity.X:F2}, {rigidbody.LinearVelocity.Y:F2}");
        ImGuiNET.ImGui.Text($"Angular Velocity: {rigidbody.AngularVelocity:F2}");
        ImGuiNET.ImGui.End();
    }

    protected override void OnDestroy(World world)
    {
        _circleMesh.Dispose();
    }

    private void UpdateCamera(World world, float deltaTime)
    {
        ref TransformComponent transform = ref world.Get<TransformComponent>(_camera);
        ref CameraComponent camera = ref world.Get<CameraComponent>(_camera);

        float speed = 5.0f / camera.Zoom;

        if (GameInput.IsKeyDown(KeyCode.W)) transform.Position.Y += speed * deltaTime;
        if (GameInput.IsKeyDown(KeyCode.S)) transform.Position.Y -= speed * deltaTime;
        if (GameInput.IsKeyDown(KeyCode.A)) transform.Position.X -= speed * deltaTime;
        if (GameInput.IsKeyDown(KeyCode.D)) transform.Position.X += speed * deltaTime;

        camera.Zoom += GameInput.ScrollDelta * 0.1f;
        camera.Zoom = Math.Clamp(camera.Zoom, 0.1f, 20.0f);
    }
}