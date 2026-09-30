using System;
using System.Numerics;
using System.Runtime.CompilerServices;
using Vortice.Direct3D11;

namespace Vultaik;

public struct Vertex2D
{
    public Vector2 Position;
    public Vector4 Color;

    public Vertex2D(Vector2 position, Vector4 color)
    {
        Position = position;
        Color = color;
    }
}

public sealed class Mesh2D : IDisposable
{
    public ID3D11Buffer VertexBuffer { get; }
    public ID3D11Buffer IndexBuffer { get; }
    public uint VertexStride => (uint)Unsafe.SizeOf<Vertex2D>();
    public uint IndexCount { get; }

    public Mesh2D(ID3D11Device device, ReadOnlySpan<Vertex2D> vertices, ReadOnlySpan<uint> indices)
    {
        VertexBuffer = device.CreateBuffer(vertices, BindFlags.VertexBuffer);
        IndexBuffer = device.CreateBuffer(indices, BindFlags.IndexBuffer);
        IndexCount = (uint)indices.Length;
    }

    public static Mesh2D CreateCircle(ID3D11Device device, float radius, int segments, Vector4 color)
    {
        if (segments < 3) throw new ArgumentOutOfRangeException(nameof(segments));

        Vertex2D[] vertices = new Vertex2D[segments + 1];
        uint[] indices = new uint[segments * 3];

        vertices[0] = new Vertex2D(Vector2.Zero, color);

        for (int i = 0; i < segments; i++)
        {
            float angle = i / (float)segments * MathF.Tau;
            Vector2 position = new(MathF.Cos(angle) * radius, MathF.Sin(angle) * radius);
            vertices[i + 1] = new Vertex2D(position, color);
        }

        for (int i = 0; i < segments; i++)
        {
            int next = (i + 1) % segments;

            indices[i * 3 + 0] = 0;
            indices[i * 3 + 1] = (uint)(i + 1);
            indices[i * 3 + 2] = (uint)(next + 1);
        }

        return new Mesh2D(device, vertices, indices);
    }

    public void Dispose()
    {
        IndexBuffer.Dispose();
        VertexBuffer.Dispose();
    }
}
