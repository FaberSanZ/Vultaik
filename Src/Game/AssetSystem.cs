using glTFLoader;
using glTFLoader.Schema;
using System.Buffers.Binary;
using System.Numerics;
using System.Runtime.CompilerServices;
using Vortice.Direct3D;
using Vortice.Direct3D11;
using Vortice.DXGI;
using GltfAnimation = glTFLoader.Schema.Animation;
using GltfAnimationChannel = glTFLoader.Schema.AnimationChannel;
using GltfAnimationSampler = glTFLoader.Schema.AnimationSampler;
using GltfSkin = glTFLoader.Schema.Skin;

namespace Vultaik;

public sealed class AssetSystem : IDisposable
{
    private readonly ID3D11Device _device;
    private readonly Dictionary<string, Model> _models = new(StringComparer.OrdinalIgnoreCase);

    public AssetSystem(ID3D11Device device)
    {
        _device = device;
    }

    public Model LoadModel(string filePath)
    {
        string fullPath = Path.GetFullPath(filePath);

        if (_models.TryGetValue(fullPath, out Model? cached))
            return cached;

        Gltf gltf = Interface.LoadModel(fullPath);

        Model model = new()
        {
            Nodes = CreateNodes(gltf),
            Materials = CreateMaterials(gltf),
            Skins = CreateSkins(gltf, fullPath),
            Animations = CreateAnimations(gltf, fullPath)
        };

        model.Meshes = CreateMeshes(gltf, model, fullPath);

        if (model.Meshes.Length == 0 || model.Meshes.All(mesh => mesh.Parts.Length == 0))
            throw new InvalidDataException($"The model '{filePath}' contains no supported triangle meshes.");

        _models.Add(fullPath, model);
        return model;
    }

    private static Node[] CreateNodes(Gltf gltf)
    {
        if (gltf.Nodes is null || gltf.Nodes.Length == 0)
            return [];

        Node[] nodes = new Node[gltf.Nodes.Length];

        for (int i = 0; i < nodes.Length; i++)
        {
            glTFLoader.Schema.Node source = gltf.Nodes[i];

            Vector3 translation = Vector3.Zero;
            Quaternion rotation = Quaternion.Identity;
            Vector3 scale = Vector3.One;

            if (source.ShouldSerializeMatrix())
            {
                Matrix4x4 matrix = CreateMatrix(source.Matrix);

                if (!Matrix4x4.Decompose(matrix, out scale, out rotation, out translation))
                {
                    translation = Vector3.Zero;
                    rotation = Quaternion.Identity;
                    scale = Vector3.One;
                }
            }
            else
            {
                translation = new Vector3(source.Translation[0], source.Translation[1], source.Translation[2]);
                rotation = new Quaternion(source.Rotation[0], source.Rotation[1], source.Rotation[2], source.Rotation[3]);
                scale = new Vector3(source.Scale[0], source.Scale[1], source.Scale[2]);
            }

            nodes[i] = new Node
            {
                Translation = translation,
                Rotation = rotation,
                Scale = scale,

                BaseTranslation = translation,
                BaseRotation = rotation,
                BaseScale = scale,

                Parent = -1,
                Children = source.Children is null ? [] : source.Children.Select(index => (uint)index).ToArray(),

                MeshIndex = source.Mesh ?? -1,
                SkinIndex = source.Skin ?? -1
            };
        }

        for (int parentIndex = 0; parentIndex < nodes.Length; parentIndex++)
        {
            foreach (uint childIndex in nodes[parentIndex].Children)
            {
                if (childIndex >= (uint)nodes.Length)
                    continue;

                Node child = nodes[childIndex];
                child.Parent = parentIndex;
                nodes[childIndex] = child;
            }
        }

        return nodes;
    }

    private static Material[] CreateMaterials(Gltf gltf)
    {
        if (gltf.Materials is null || gltf.Materials.Length == 0)
            return [];

        Material[] materials = new Material[gltf.Materials.Length];

        for (int i = 0; i < materials.Length; i++)
            materials[i] = Material.Default;

        return materials;
    }

    private static Skin[] CreateSkins(Gltf gltf, string filePath)
    {
        if (gltf.Skins is null || gltf.Skins.Length == 0)
            return [];

        Skin[] skins = new Skin[gltf.Skins.Length];

        for (int skinIndex = 0; skinIndex < gltf.Skins.Length; skinIndex++)
        {
            GltfSkin source = gltf.Skins[skinIndex];

            Skin skin = new()
            {
                Joints = source.Joints.Select(index => (uint)index).ToArray(),
                InverseBindMatrices = new Matrix4x4[source.Joints.Length]
            };

            Array.Fill(skin.InverseBindMatrices, Matrix4x4.Identity);

            if (source.InverseBindMatrices.HasValue)
            {
                Matrix4x4[] matrices = ReadMatrix4x4Accessor(gltf, source.InverseBindMatrices.Value, filePath);
                int count = Math.Min(matrices.Length, skin.InverseBindMatrices.Length);

                Array.Copy(matrices, skin.InverseBindMatrices, count);
            }

            skins[skinIndex] = skin;
        }

        return skins;
    }

    private static AnimationClip[] CreateAnimations(Gltf gltf, string filePath)
    {
        if (gltf.Animations is null || gltf.Animations.Length == 0)
            return [];

        AnimationClip[] clips = new AnimationClip[gltf.Animations.Length];

        for (int animationIndex = 0; animationIndex < gltf.Animations.Length; animationIndex++)
        {
            GltfAnimation source = gltf.Animations[animationIndex];

            AnimationClip clip = new()
            {
                Name = string.IsNullOrWhiteSpace(source.Name) ? $"Animation_{animationIndex}" : source.Name,
                Samplers = new AnimationSampler[source.Samplers.Length]
            };

            for (int samplerIndex = 0; samplerIndex < source.Samplers.Length; samplerIndex++)
            {
                GltfAnimationSampler sourceSampler = source.Samplers[samplerIndex];

                AnimationPath path = AnimationPath.Translation;

                foreach (GltfAnimationChannel sourceChannel in source.Channels)
                {
                    if (sourceChannel.Sampler != samplerIndex)
                        continue;

                    if (sourceChannel.Target.Path == glTFLoader.Schema.AnimationChannelTarget.PathEnum.rotation)
                        path = AnimationPath.Rotation;
                    else if (sourceChannel.Target.Path == glTFLoader.Schema.AnimationChannelTarget.PathEnum.scale)
                        path = AnimationPath.Scale;

                    break;
                }

                float[] times = ReadFloatAccessor(gltf, sourceSampler.Input, filePath);
                Vector4[] values = path == AnimationPath.Rotation
                    ? ReadVector4Accessor(gltf, sourceSampler.Output, filePath)
                    : ReadVector3AsVector4Accessor(gltf, sourceSampler.Output, filePath);

                if (sourceSampler.Interpolation == GltfAnimationSampler.InterpolationEnum.CUBICSPLINE)
                    throw new NotSupportedException("CUBICSPLINE animation is not supported yet.");

                AnimationSampler sampler = new()
                {
                    Times = times,
                    Values = values,
                    Interpolation = sourceSampler.Interpolation == GltfAnimationSampler.InterpolationEnum.STEP
                        ? AnimationInterpolation.Step
                        : AnimationInterpolation.Linear
                };

                clip.Samplers[samplerIndex] = sampler;

                for (int i = 0; i < times.Length; i++)
                    clip.Duration = Math.Max(clip.Duration, times[i]);
            }

            List<AnimationChannel> channels = new(source.Channels.Length);

            foreach (GltfAnimationChannel sourceChannel in source.Channels)
            {
                if (!sourceChannel.Target.Node.HasValue)
                    continue;

                if (sourceChannel.Target.Path == glTFLoader.Schema.AnimationChannelTarget.PathEnum.weights)
                    continue;

                AnimationPath path;

                if (sourceChannel.Target.Path == glTFLoader.Schema.AnimationChannelTarget.PathEnum.translation)
                    path = AnimationPath.Translation;
                else if (sourceChannel.Target.Path == glTFLoader.Schema.AnimationChannelTarget.PathEnum.rotation)
                    path = AnimationPath.Rotation;
                else if (sourceChannel.Target.Path == glTFLoader.Schema.AnimationChannelTarget.PathEnum.scale)
                    path = AnimationPath.Scale;
                else
                    continue;

                channels.Add(new AnimationChannel
                {
                    SamplerIndex = (uint)sourceChannel.Sampler,
                    NodeIndex = (uint)sourceChannel.Target.Node.Value,
                    Path = path
                });
            }

            clip.Channels = channels.ToArray();
            clips[animationIndex] = clip;
        }

        return clips;
    }

    private Mesh[] CreateMeshes(Gltf gltf, Model model, string filePath)
    {
        if (gltf.Meshes is null || gltf.Meshes.Length == 0)
            return [];

        int[] meshNodeIndices = new int[gltf.Meshes.Length];
        Array.Fill(meshNodeIndices, -1);

        for (int nodeIndex = 0; nodeIndex < model.Nodes.Length; nodeIndex++)
        {
            int meshIndex = model.Nodes[nodeIndex].MeshIndex;

            if ((uint)meshIndex < (uint)meshNodeIndices.Length && meshNodeIndices[meshIndex] == -1)
                meshNodeIndices[meshIndex] = nodeIndex;
        }

        Mesh[] meshes = new Mesh[gltf.Meshes.Length];

        for (int meshIndex = 0; meshIndex < gltf.Meshes.Length; meshIndex++)
        {
            glTFLoader.Schema.Mesh sourceMesh = gltf.Meshes[meshIndex];
            List<MeshPart> parts = new(sourceMesh.Primitives.Length);

            foreach (MeshPrimitive primitive in sourceMesh.Primitives)
            {
                if (primitive.Mode != MeshPrimitive.ModeEnum.TRIANGLES)
                    continue;

                if (!primitive.Attributes.TryGetValue("POSITION", out int positionAccessorIndex))
                    continue;

                Vector3[] positions = ReadVector3Accessor(gltf, positionAccessorIndex, filePath);
                Vector3[] normals = primitive.Attributes.TryGetValue("NORMAL", out int normalAccessorIndex)
                    ? ReadVector3Accessor(gltf, normalAccessorIndex, filePath)
                    : Enumerable.Repeat(Vector3.UnitY, positions.Length).ToArray();

                UInt4[] joints = primitive.Attributes.TryGetValue("JOINTS_0", out int jointsAccessorIndex)
                    ? ReadUInt4Accessor(gltf, jointsAccessorIndex, filePath)
                    : new UInt4[positions.Length];

                Vector4[] weights = primitive.Attributes.TryGetValue("WEIGHTS_0", out int weightsAccessorIndex)
                    ? ReadWeightsAccessor(gltf, weightsAccessorIndex, filePath)
                    : CreateDefaultWeights(positions.Length);

                Vertex[] vertices = new Vertex[positions.Length];

                for (int i = 0; i < vertices.Length; i++)
                {
                    vertices[i] = new Vertex
                    {
                        Position = positions[i],
                        Normal = i < normals.Length ? normals[i] : Vector3.UnitY,
                        Joints = i < joints.Length ? joints[i] : default,
                        Weights = i < weights.Length ? weights[i] : new Vector4(1.0f, 0.0f, 0.0f, 0.0f)
                    };
                }

                uint[] indices;

                if (primitive.Indices.HasValue)
                {
                    indices = ReadIndexAccessor(gltf, primitive.Indices.Value, filePath);
                }
                else
                {
                    indices = new uint[vertices.Length];

                    for (uint i = 0; i < indices.Length; i++)
                        indices[i] = i;
                }

                parts.Add(new MeshPart
                {
                    Vertex = CreateVertexResource(vertices),
                    Index = CreateIndexResource(indices),
                    NodeIndex = meshNodeIndices[meshIndex],
                    MaterialIndex = primitive.Material ?? -1
                });
            }

            meshes[meshIndex] = new Mesh
            {
                Parts = parts.ToArray()
            };
        }

        return meshes;
    }

    private Resource CreateVertexResource(Vertex[] vertices)
    {
        ID3D11Buffer buffer = _device.CreateBuffer(
            vertices,
            BindFlags.ShaderResource,
            ResourceUsage.Default,
            CpuAccessFlags.None,
            ResourceOptionFlags.BufferStructured
        );

        ShaderResourceViewDescription viewDescription = new(
            ShaderResourceViewDimension.Buffer,
            Format.Unknown,
            0,
            (uint)vertices.Length
        );

        ID3D11ShaderResourceView view = _device.CreateShaderResourceView(buffer, viewDescription);

        return new Resource
        {
            Buffer = buffer,
            View = view,
            Stride = (uint)Unsafe.SizeOf<Vertex>(),
            Count = (uint)vertices.Length
        };
    }

    private Resource CreateIndexResource(uint[] indices)
    {
        ID3D11Buffer buffer = _device.CreateBuffer(indices, BindFlags.IndexBuffer);

        return new Resource
        {
            Buffer = buffer,
            View = null,
            Stride = sizeof(uint),
            Count = (uint)indices.Length
        };
    }

    private static Vector4[] CreateDefaultWeights(int count)
    {
        Vector4[] weights = new Vector4[count];

        for (int i = 0; i < weights.Length; i++)
            weights[i] = new Vector4(1.0f, 0.0f, 0.0f, 0.0f);

        return weights;
    }

    private static float[] ReadFloatAccessor(Gltf gltf, int accessorIndex, string filePath)
    {
        Accessor accessor = gltf.Accessors[accessorIndex];

        if (accessor.ComponentType != Accessor.ComponentTypeEnum.FLOAT)
            throw new NotSupportedException("Animation time accessors must use FLOAT.");

        (byte[] buffer, int start, int stride) = GetAccessorData(gltf, accessor, filePath, sizeof(float));

        float[] values = new float[accessor.Count];

        for (int i = 0; i < values.Length; i++)
            values[i] = BinaryPrimitives.ReadSingleLittleEndian(buffer.AsSpan(start + i * stride, 4));

        return values;
    }

    private static Vector3[] ReadVector3Accessor(Gltf gltf, int accessorIndex, string filePath)
    {
        Accessor accessor = gltf.Accessors[accessorIndex];

        if (accessor.ComponentType != Accessor.ComponentTypeEnum.FLOAT)
            throw new NotSupportedException("VEC3 accessor must use FLOAT.");

        (byte[] buffer, int start, int stride) = GetAccessorData(gltf, accessor, filePath, 12);

        Vector3[] values = new Vector3[accessor.Count];

        for (int i = 0; i < values.Length; i++)
        {
            int offset = start + i * stride;

            values[i] = new Vector3(
                BinaryPrimitives.ReadSingleLittleEndian(buffer.AsSpan(offset, 4)),
                BinaryPrimitives.ReadSingleLittleEndian(buffer.AsSpan(offset + 4, 4)),
                BinaryPrimitives.ReadSingleLittleEndian(buffer.AsSpan(offset + 8, 4))
            );
        }

        return values;
    }

    private static Vector4[] ReadVector3AsVector4Accessor(Gltf gltf, int accessorIndex, string filePath)
    {
        Vector3[] source = ReadVector3Accessor(gltf, accessorIndex, filePath);
        Vector4[] values = new Vector4[source.Length];

        for (int i = 0; i < values.Length; i++)
            values[i] = new Vector4(source[i], 0.0f);

        return values;
    }

    private static Vector4[] ReadVector4Accessor(Gltf gltf, int accessorIndex, string filePath)
    {
        Accessor accessor = gltf.Accessors[accessorIndex];

        if (accessor.ComponentType != Accessor.ComponentTypeEnum.FLOAT)
            throw new NotSupportedException("VEC4 accessor must use FLOAT.");

        (byte[] buffer, int start, int stride) = GetAccessorData(gltf, accessor, filePath, 16);

        Vector4[] values = new Vector4[accessor.Count];

        for (int i = 0; i < values.Length; i++)
        {
            int offset = start + i * stride;

            values[i] = new Vector4(
                BinaryPrimitives.ReadSingleLittleEndian(buffer.AsSpan(offset, 4)),
                BinaryPrimitives.ReadSingleLittleEndian(buffer.AsSpan(offset + 4, 4)),
                BinaryPrimitives.ReadSingleLittleEndian(buffer.AsSpan(offset + 8, 4)),
                BinaryPrimitives.ReadSingleLittleEndian(buffer.AsSpan(offset + 12, 4))
            );
        }

        return values;
    }

    private static UInt4[] ReadUInt4Accessor(Gltf gltf, int accessorIndex, string filePath)
    {
        Accessor accessor = gltf.Accessors[accessorIndex];

        int componentSize = accessor.ComponentType switch
        {
            Accessor.ComponentTypeEnum.UNSIGNED_BYTE => 1,
            Accessor.ComponentTypeEnum.UNSIGNED_SHORT => 2,
            _ => throw new NotSupportedException("JOINTS_0 must use UNSIGNED_BYTE or UNSIGNED_SHORT.")
        };

        (byte[] buffer, int start, int stride) = GetAccessorData(gltf, accessor, filePath, componentSize * 4);

        UInt4[] values = new UInt4[accessor.Count];

        for (int i = 0; i < values.Length; i++)
        {
            int offset = start + i * stride;

            values[i] = new UInt4(
                ReadUnsignedComponent(buffer, offset, accessor.ComponentType),
                ReadUnsignedComponent(buffer, offset + componentSize, accessor.ComponentType),
                ReadUnsignedComponent(buffer, offset + componentSize * 2, accessor.ComponentType),
                ReadUnsignedComponent(buffer, offset + componentSize * 3, accessor.ComponentType)
            );
        }

        return values;
    }

    private static Vector4[] ReadWeightsAccessor(Gltf gltf, int accessorIndex, string filePath)
    {
        Accessor accessor = gltf.Accessors[accessorIndex];

        int componentSize = accessor.ComponentType switch
        {
            Accessor.ComponentTypeEnum.FLOAT => 4,
            Accessor.ComponentTypeEnum.UNSIGNED_BYTE => 1,
            Accessor.ComponentTypeEnum.UNSIGNED_SHORT => 2,
            _ => throw new NotSupportedException("WEIGHTS_0 must use FLOAT, UNSIGNED_BYTE, or UNSIGNED_SHORT.")
        };

        (byte[] buffer, int start, int stride) = GetAccessorData(gltf, accessor, filePath, componentSize * 4);

        Vector4[] values = new Vector4[accessor.Count];

        for (int i = 0; i < values.Length; i++)
        {
            int offset = start + i * stride;

            Vector4 value = new(
                ReadWeightComponent(buffer, offset, accessor.ComponentType),
                ReadWeightComponent(buffer, offset + componentSize, accessor.ComponentType),
                ReadWeightComponent(buffer, offset + componentSize * 2, accessor.ComponentType),
                ReadWeightComponent(buffer, offset + componentSize * 3, accessor.ComponentType)
            );

            float sum = value.X + value.Y + value.Z + value.W;

            if (sum > 0.0f)
                value /= sum;
            else
                value = new Vector4(1.0f, 0.0f, 0.0f, 0.0f);

            values[i] = value;
        }

        return values;
    }

    private static Matrix4x4[] ReadMatrix4x4Accessor(Gltf gltf, int accessorIndex, string filePath)
    {
        Accessor accessor = gltf.Accessors[accessorIndex];

        if (accessor.ComponentType != Accessor.ComponentTypeEnum.FLOAT)
            throw new NotSupportedException("Inverse bind matrices must use FLOAT.");

        (byte[] buffer, int start, int stride) = GetAccessorData(gltf, accessor, filePath, 64);

        Matrix4x4[] values = new Matrix4x4[accessor.Count];

        for (int i = 0; i < values.Length; i++)
        {
            int offset = start + i * stride;

            values[i] = new Matrix4x4(
                ReadFloat(buffer, offset + 0),
                ReadFloat(buffer, offset + 4),
                ReadFloat(buffer, offset + 8),
                ReadFloat(buffer, offset + 12),

                ReadFloat(buffer, offset + 16),
                ReadFloat(buffer, offset + 20),
                ReadFloat(buffer, offset + 24),
                ReadFloat(buffer, offset + 28),

                ReadFloat(buffer, offset + 32),
                ReadFloat(buffer, offset + 36),
                ReadFloat(buffer, offset + 40),
                ReadFloat(buffer, offset + 44),

                ReadFloat(buffer, offset + 48),
                ReadFloat(buffer, offset + 52),
                ReadFloat(buffer, offset + 56),
                ReadFloat(buffer, offset + 60)
            );
        }

        return values;
    }

    private static uint[] ReadIndexAccessor(Gltf gltf, int accessorIndex, string filePath)
    {
        Accessor accessor = gltf.Accessors[accessorIndex];

        int componentSize = accessor.ComponentType switch
        {
            Accessor.ComponentTypeEnum.UNSIGNED_BYTE => 1,
            Accessor.ComponentTypeEnum.UNSIGNED_SHORT => 2,
            Accessor.ComponentTypeEnum.UNSIGNED_INT => 4,
            _ => throw new NotSupportedException("Unsupported index component type.")
        };

        (byte[] buffer, int start, int stride) = GetAccessorData(gltf, accessor, filePath, componentSize);

        uint[] indices = new uint[accessor.Count];

        for (int i = 0; i < indices.Length; i++)
            indices[i] = ReadUnsignedComponent(buffer, start + i * stride, accessor.ComponentType);

        return indices;
    }

    private static (byte[] Buffer, int Start, int Stride) GetAccessorData(Gltf gltf, Accessor accessor, string filePath, int elementSize)
    {
        if (!accessor.BufferView.HasValue)
            throw new NotSupportedException("Sparse/accessor-without-bufferView is not supported yet.");

        BufferView bufferView = gltf.BufferViews[accessor.BufferView.Value];
        byte[] buffer = gltf.LoadBinaryBuffer(bufferView.Buffer, filePath);

        int start = bufferView.ByteOffset + accessor.ByteOffset;
        int stride = bufferView.ByteStride ?? elementSize;

        return (buffer, start, stride);
    }

    private static uint ReadUnsignedComponent(byte[] buffer, int offset, Accessor.ComponentTypeEnum componentType)
    {
        return componentType switch
        {
            Accessor.ComponentTypeEnum.UNSIGNED_BYTE => buffer[offset],
            Accessor.ComponentTypeEnum.UNSIGNED_SHORT => BinaryPrimitives.ReadUInt16LittleEndian(buffer.AsSpan(offset, 2)),
            Accessor.ComponentTypeEnum.UNSIGNED_INT => BinaryPrimitives.ReadUInt32LittleEndian(buffer.AsSpan(offset, 4)),
            _ => throw new NotSupportedException("Unsigned component type expected.")
        };
    }

    private static float ReadWeightComponent(byte[] buffer, int offset, Accessor.ComponentTypeEnum componentType)
    {
        return componentType switch
        {
            Accessor.ComponentTypeEnum.FLOAT => ReadFloat(buffer, offset),
            Accessor.ComponentTypeEnum.UNSIGNED_BYTE => buffer[offset] / 255.0f,
            Accessor.ComponentTypeEnum.UNSIGNED_SHORT => BinaryPrimitives.ReadUInt16LittleEndian(buffer.AsSpan(offset, 2)) / 65535.0f,
            _ => 0.0f
        };
    }

    private static float ReadFloat(byte[] buffer, int offset)
    {
        return BinaryPrimitives.ReadSingleLittleEndian(buffer.AsSpan(offset, 4));
    }

    private static Matrix4x4 CreateMatrix(float[] matrix)
    {
        return new Matrix4x4(
            matrix[0], matrix[1], matrix[2], matrix[3],
            matrix[4], matrix[5], matrix[6], matrix[7],
            matrix[8], matrix[9], matrix[10], matrix[11],
            matrix[12], matrix[13], matrix[14], matrix[15]
        );
    }

    public void Dispose()
    {
        foreach (Model model in _models.Values)
        {
            foreach (Mesh mesh in model.Meshes)
            {
                foreach (MeshPart part in mesh.Parts)
                {
                    part.Vertex.Dispose();
                    part.Index.Dispose();
                }
            }
        }

        _models.Clear();
    }
}
