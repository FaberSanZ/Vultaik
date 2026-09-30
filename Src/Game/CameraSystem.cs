using System;
using System.Numerics;

namespace Vultaik;

public sealed class CameraSystem
{
    public bool TryGetMatrices(World world, float aspectRatio, out Matrix4x4 view, out Matrix4x4 projection)
    {
        foreach (Entity entity in world.Query<TransformComponent, CameraComponent, PrimaryCameraComponent>())
        {
            ref TransformComponent transform = ref world.Get<TransformComponent>(entity);
            ref CameraComponent camera = ref world.Get<CameraComponent>(entity);

            float zoom = MathF.Max(camera.Zoom, 0.01f);
            float height = camera.Size / zoom;
            float width = height * aspectRatio;

            view = Matrix4x4.CreateTranslation(-transform.Position.X, -transform.Position.Y, 0.0f);
            projection = CreateOrthographicLH(width, height, camera.NearPlane, camera.FarPlane);

            return true;
        }

        view = Matrix4x4.Identity;
        projection = Matrix4x4.Identity;
        return false;
    }

    private static Matrix4x4 CreateOrthographicLH(float width, float height, float nearPlane, float farPlane)
    {
        return new Matrix4x4(
            2.0f / width, 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / height, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f / (farPlane - nearPlane), 0.0f,
            0.0f, 0.0f, -nearPlane / (farPlane - nearPlane), 1.0f
        );
    }
}