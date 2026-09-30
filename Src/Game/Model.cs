using System.Numerics;

namespace Vultaik;

public struct UInt4
{
    public uint X;
    public uint Y;
    public uint Z;
    public uint W;

    public UInt4(uint x, uint y, uint z, uint w)
    {
        X = x;
        Y = y;
        Z = z;
        W = w;
    }
}

public struct Vertex
{
    public Vector3 Position;
    public Vector3 Normal;

    public UInt4 Joints;
    public Vector4 Weights;
}

public struct Node
{
    public Vector3 Translation;
    public Quaternion Rotation;
    public Vector3 Scale;

    public Vector3 BaseTranslation;
    public Quaternion BaseRotation;
    public Vector3 BaseScale;

    public int Parent;
    public uint[] Children;

    public int MeshIndex;
    public int SkinIndex;

    public static Node Default => new()
    {
        Translation = Vector3.Zero,
        Rotation = Quaternion.Identity,
        Scale = Vector3.One,
        BaseTranslation = Vector3.Zero,
        BaseRotation = Quaternion.Identity,
        BaseScale = Vector3.One,
        Parent = -1,
        Children = [],
        MeshIndex = -1,
        SkinIndex = -1
    };
}

public sealed class Skin
{
    public uint[] Joints { get; set; } = [];
    public Matrix4x4[] InverseBindMatrices { get; set; } = [];
}

public enum AnimationPath
{
    Translation,
    Rotation,
    Scale
}

public enum AnimationInterpolation
{
    Linear,
    Step
}

public sealed class AnimationSampler
{
    public float[] Times { get; set; } = [];
    public Vector4[] Values { get; set; } = [];
    public AnimationInterpolation Interpolation { get; set; } = AnimationInterpolation.Linear;
}

public struct AnimationChannel
{
    public uint SamplerIndex;
    public uint NodeIndex;
    public AnimationPath Path;
}

public sealed class AnimationClip
{
    public string Name { get; set; } = string.Empty;
    public float Duration { get; set; }

    public AnimationSampler[] Samplers { get; set; } = [];
    public AnimationChannel[] Channels { get; set; } = [];
}

public struct Material
{
    public Vector4 BaseColor;

    public static Material Default => new()
    {
        BaseColor = Vector4.One
    };
}

public sealed class MeshPart
{
    public Resource Vertex = new();
    public Resource Index = new();

    public int NodeIndex = -1;
    public int MaterialIndex = -1;
}

public sealed class Mesh
{
    public MeshPart[] Parts { get; set; } = [];
}

public struct NodePose
{
    public Vector3 Translation;
    public Quaternion Rotation;
    public Vector3 Scale;

    public static NodePose Default => new()
    {
        Translation = Vector3.Zero,
        Rotation = Quaternion.Identity,
        Scale = Vector3.One
    };
}

public sealed class Model
{
    public Node[] Nodes { get; set; } = [];
    public Mesh[] Meshes { get; set; } = [];
    public Material[] Materials { get; set; } = [];
    public Skin[] Skins { get; set; } = [];
    public AnimationClip[] Animations { get; set; } = [];
}
