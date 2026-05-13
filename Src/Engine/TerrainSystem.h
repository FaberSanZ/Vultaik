#pragma once

#include <cstdint>
#include <vector>
#include <DirectXMath.h>


struct VertexTerrain
{
    float position[3];
    float uv[2];
    float normal[3];
};


class TerrainChunk
{
public:
    TerrainChunk() = default;

    void Initialize(int chunkX, int chunkZ, uint32_t cellsPerSide, float cellSize, const DirectX::XMFLOAT3& worldPosition)
    {
        m_ChunkX = chunkX;
        m_ChunkZ = chunkZ;
        m_CellsPerSide = cellsPerSide;
        m_CellSize = cellSize;
        m_WorldPosition = worldPosition;

        m_VerticesPerSide = m_CellsPerSide + 1;

        const uint32_t heightCount = m_VerticesPerSide * m_VerticesPerSide;
        m_Heights.clear();
        m_Heights.resize(heightCount, 0.0f);

        BuildVertices();
        BuildIndices();
    }


    void BuildVertices()
    {
        m_Vertices.clear();

        const uint32_t vertexCount = m_VerticesPerSide * m_VerticesPerSide;
        m_Vertices.reserve(vertexCount);

        for (uint32_t z = 0; z < m_VerticesPerSide; ++z)
        {
            for (uint32_t x = 0; x < m_VerticesPerSide; ++x)
            {
                const float localX = static_cast<float>(x) * m_CellSize;
                const float localZ = static_cast<float>(z) * m_CellSize;
                const float localY = GetHeight(x, z);

                const float u = static_cast<float>(x) / static_cast<float>(m_CellsPerSide);
                const float v = static_cast<float>(z) / static_cast<float>(m_CellsPerSide);

                VertexTerrain vertex{};
                vertex.position[0] = m_WorldPosition.x + localX;
                vertex.position[1] = m_WorldPosition.y + localY;
                vertex.position[2] = m_WorldPosition.z + localZ;

                vertex.uv[0] = u;
                vertex.uv[1] = v;

                vertex.normal[0] = 0.0f;
                vertex.normal[1] = 1.0f;
                vertex.normal[2] = 0.0f;

                m_Vertices.push_back(vertex);
            }
        }
    }


    void BuildIndices()
    {
        m_Indices.clear();

        const uint32_t quadCount = m_CellsPerSide * m_CellsPerSide;
        const uint32_t indexCount = quadCount * 6;

        m_Indices.reserve(indexCount);

        for (uint32_t z = 0; z < m_CellsPerSide; ++z)
        {
            for (uint32_t x = 0; x < m_CellsPerSide; ++x)
            {
                const uint32_t topLeft = z * m_VerticesPerSide + x;
                const uint32_t topRight = topLeft + 1;

                const uint32_t bottomLeft = (z + 1) * m_VerticesPerSide + x;
                const uint32_t bottomRight = bottomLeft + 1;

                // Triángulo 1
                m_Indices.push_back(topLeft);
                m_Indices.push_back(bottomLeft);
                m_Indices.push_back(topRight);

                // Triángulo 2
                m_Indices.push_back(topRight);
                m_Indices.push_back(bottomLeft);
                m_Indices.push_back(bottomRight);
            }
        }
    }

    int GetChunkX() const
    {
        return m_ChunkX;
    }

    int GetChunkZ() const
    {
        return m_ChunkZ;
    }

    uint32_t GetCellsPerSide() const
    {
        return m_CellsPerSide;
    }

    uint32_t GetVerticesPerSide() const
    {
        return m_VerticesPerSide;
    }

    float GetCellSize() const
    {
        return m_CellSize;
    }

    float GetWorldSize() const
    {
        return static_cast<float>(m_CellsPerSide) * m_CellSize;
    }

    const DirectX::XMFLOAT3& GetWorldPosition() const
    {
        return m_WorldPosition;
    }

    uint32_t GetHeightIndex(uint32_t x, uint32_t z) const
    {
        return z * m_VerticesPerSide + x;
    }

    float GetHeight(uint32_t x, uint32_t z) const
    {
        return m_Heights[GetHeightIndex(x, z)];
    }

    void SetHeight(uint32_t x, uint32_t z, float height)
    {
        m_Heights[GetHeightIndex(x, z)] = height;
    }

    uint32_t GetHeightCount() const
    {
        return static_cast<uint32_t>(m_Heights.size());
    }


    const std::vector<VertexTerrain>& GetVertices() const
    {
        return m_Vertices;
    }

    uint32_t GetVertexCount() const
    {
        return static_cast<uint32_t>(m_Vertices.size());
    }


    const std::vector<uint32_t>& GetIndices() const
    {
        return m_Indices;
    }

    uint32_t GetIndexCount() const
    {
        return static_cast<uint32_t>(m_Indices.size());
    }


private:
    int m_ChunkX = 0;
    int m_ChunkZ = 0;

    uint32_t m_CellsPerSide = 0;
    uint32_t m_VerticesPerSide = 0;

    float m_CellSize = 1.0f;

    DirectX::XMFLOAT3 m_WorldPosition = { 0.0f, 0.0f, 0.0f };

    std::vector<float> m_Heights;
    std::vector<VertexTerrain> m_Vertices;
    std::vector<uint32_t> m_Indices;
};