using System.Numerics;

namespace Vultaik;

public sealed class PhysicsSystem
{
    public Vector2 Gravity = new(0.0f, -9.81f);

    public void AddForce(World world, Entity entity, Vector2 force, Vector2 point)
    {
        ref TransformComponent transform = ref world.Get<TransformComponent>(entity);
        ref RigidbodyComponent rigidbody = ref world.Get<RigidbodyComponent>(entity);

        if (rigidbody.Type != BodyType.Dynamic)
            return;

        rigidbody.Force += force;
        rigidbody.Torque += Cross(point - transform.Position, force);
    }

    public void AddForceToCenter(World world, Entity entity, Vector2 force)
    {
        ref RigidbodyComponent rigidbody = ref world.Get<RigidbodyComponent>(entity);

        if (rigidbody.Type != BodyType.Dynamic)
            return;

        rigidbody.Force += force;
    }

    public void AddTorque(World world, Entity entity, float torque)
    {
        ref RigidbodyComponent rigidbody = ref world.Get<RigidbodyComponent>(entity);

        if (rigidbody.Type != BodyType.Dynamic)
            return;

        rigidbody.Torque += torque;
    }

    public void AddLinearImpulse(World world, Entity entity, Vector2 impulse, Vector2 point)
    {
        ref TransformComponent transform = ref world.Get<TransformComponent>(entity);
        ref RigidbodyComponent rigidbody = ref world.Get<RigidbodyComponent>(entity);

        if (rigidbody.Type != BodyType.Dynamic)
            return;

        rigidbody.LinearVelocity += impulse * rigidbody.InverseMass;
        rigidbody.AngularVelocity += rigidbody.InverseInertia * Cross(point - transform.Position, impulse);
    }

    public void AddLinearImpulseToCenter(World world, Entity entity, Vector2 impulse)
    {
        ref RigidbodyComponent rigidbody = ref world.Get<RigidbodyComponent>(entity);

        if (rigidbody.Type != BodyType.Dynamic)
            return;

        rigidbody.LinearVelocity += impulse * rigidbody.InverseMass;
    }

    public void ClearForces(World world, Entity entity)
    {
        ref RigidbodyComponent rigidbody = ref world.Get<RigidbodyComponent>(entity);

        rigidbody.Force = Vector2.Zero;
        rigidbody.Torque = 0.0f;
    }

    public void Step(World world, float deltaTime)
    {
        foreach (Entity entity in world.Query<TransformComponent, RigidbodyComponent>())
        {
            ref TransformComponent transform = ref world.Get<TransformComponent>(entity);
            ref RigidbodyComponent rigidbody = ref world.Get<RigidbodyComponent>(entity);

            switch (rigidbody.Type)
            {
                case BodyType.Static:
                    break;

                case BodyType.Kinematic:
                    transform.Position += rigidbody.LinearVelocity * deltaTime;
                    transform.Rotation += rigidbody.AngularVelocity * deltaTime;
                    break;

                case BodyType.Dynamic:
                    {
                        Vector2 linearVelocityDelta =
                            rigidbody.Force * rigidbody.InverseMass * deltaTime +
                            Gravity * rigidbody.GravityScale * deltaTime;

                        float angularVelocityDelta =
                            rigidbody.Torque * rigidbody.InverseInertia * deltaTime;

                        float linearDamping = 1.0f / (1.0f + deltaTime * rigidbody.LinearDamping);
                        float angularDamping = 1.0f / (1.0f + deltaTime * rigidbody.AngularDamping);

                        rigidbody.LinearVelocity = (rigidbody.LinearVelocity + linearVelocityDelta) * linearDamping;
                        rigidbody.AngularVelocity = (rigidbody.AngularVelocity + angularVelocityDelta) * angularDamping;

                        transform.Position += rigidbody.LinearVelocity * deltaTime;
                        transform.Rotation += rigidbody.AngularVelocity * deltaTime;

                        rigidbody.Force = Vector2.Zero;
                        rigidbody.Torque = 0.0f;

                        break;
                    }
            }
        }
    }

    private static float Cross(Vector2 a, Vector2 b)
    {
        return a.X * b.Y - a.Y * b.X;
    }
}