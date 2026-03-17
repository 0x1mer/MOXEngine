#include "pch.h"
#include "Primitives.h"

#include <glm/glm.hpp>
#include <cmath>

Mesh Primitives::CreateCube(float size)
{
    Mesh mesh;
    const float h = size * 0.5f;

    std::vector<Vertex> vertices;
    std::vector<Index>  indices;

    vertices.reserve(24);
    indices.reserve(36);

    const glm::vec3 white(1.0f);

    struct Face {
        glm::vec3 normal;
        glm::vec3 v0, v1, v2, v3; // quad
    };

    const Face faces[6] = {
        // +X
        {{ 1, 0, 0}, { h,-h,-h}, { h,-h, h}, { h, h, h}, { h, h,-h}},
        // -X
        {{-1, 0, 0}, {-h,-h, h}, {-h,-h,-h}, {-h, h,-h}, {-h, h, h}},
        // +Y
        {{ 0, 1, 0}, {-h, h,-h}, { h, h,-h}, { h, h, h}, {-h, h, h}},
        // -Y
        {{ 0,-1, 0}, {-h,-h, h}, { h,-h, h}, { h,-h,-h}, {-h,-h,-h}},
        // +Z
        {{ 0, 0, 1}, {-h,-h, h}, {-h, h, h}, { h, h, h}, { h,-h, h}},
        // -Z
        {{ 0, 0,-1}, { h,-h,-h}, { h, h,-h}, {-h, h,-h}, {-h,-h,-h}},
    };

    for (int f = 0; f < 6; ++f)
    {
        const Face& face = faces[f];
        const Index baseIndex = static_cast<Index>(vertices.size());

        const glm::vec2 uvs[4] = {
            {0,0}, {1,0}, {1,1}, {0,1}
        };

        const glm::vec3 positions[4] = {
            face.v0, face.v1, face.v2, face.v3
        };

        for (int i = 0; i < 4; ++i)
        {
            Vertex v{};
            v.pos = positions[i];
            v.normal = face.normal;
            v.texCoords = uvs[i];
            v.color = white;
            vertices.push_back(v);
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);

        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    mesh.Create(std::move(vertices), std::move(indices));
    return mesh;
}

Mesh Primitives::CreateSphere(float radius, unsigned int sectorCount, unsigned int stackCount)
{
    Mesh mesh;

    std::vector<Vertex> vertices;
    std::vector<Index>  indices;

    const float PI = 3.14159265359f;
    const glm::vec3 white(1.0f);

    vertices.reserve((stackCount + 1) * (sectorCount + 1));

    for (unsigned int i = 0; i <= stackCount; ++i)
    {
        const float stackAngle = PI * 0.5f - (static_cast<float>(i) * PI / stackCount);
        const float xy = radius * std::cos(stackAngle);
        const float z = radius * std::sin(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j)
        {
            const float sectorAngle = static_cast<float>(j) * 2.0f * PI / sectorCount;

            const float x = xy * std::cos(sectorAngle);
            const float y = xy * std::sin(sectorAngle);

            const glm::vec3 pos(x, y, z);
            const glm::vec3 nrm = glm::normalize(pos);

            const glm::vec2 uv(
                static_cast<float>(j) / sectorCount,
                static_cast<float>(i) / stackCount
            );

            Vertex v{};
            v.pos = pos;
            v.normal = nrm;
            v.texCoords = uv;
            v.color = white;
            vertices.push_back(v);
        }
    }

    indices.reserve(stackCount * sectorCount * 6);

    for (unsigned int i = 0; i < stackCount; ++i)
    {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    mesh.Create(std::move(vertices), std::move(indices));
    return mesh;
}

struct EdgeKey {
    uint32_t a;
    uint32_t b;
    bool operator==(const EdgeKey& o) const noexcept { return a == o.a && b == o.b; }
};

struct EdgeKeyHash {
    size_t operator()(const EdgeKey& k) const noexcept {
        return (static_cast<size_t>(k.a) << 32) ^ static_cast<size_t>(k.b);
    }
};

static uint32_t GetMidpointIndex(
    uint32_t i0,
    uint32_t i1,
    std::vector<glm::vec3>& unitPositions,
    std::unordered_map<EdgeKey, uint32_t, EdgeKeyHash>& cache)
{
    uint32_t a = std::min(i0, i1);
    uint32_t b = std::max(i0, i1);
    EdgeKey key{ a, b };

    if (auto it = cache.find(key); it != cache.end())
        return it->second;

    // midpoint on unit sphere
    const glm::vec3 mid = glm::normalize(unitPositions[a] + unitPositions[b]);

    unitPositions.push_back(mid);
    const uint32_t idx = static_cast<uint32_t>(unitPositions.size() - 1);
    cache.emplace(key, idx);
    return idx;
}

Mesh Primitives::CreateIcoSphere(float radius, unsigned int subdivisions)
{
    Mesh mesh;

    // clamp subdivisions to something sane to avoid exploding mesh size
    if (subdivisions > 8) subdivisions = 8;

    const glm::vec3 white(1.0f);

    // Base icosahedron (unit sphere points)
    const float t = (1.0f + std::sqrt(5.0f)) * 0.5f;

    std::vector<glm::vec3> unitPos = {
        glm::normalize(glm::vec3(-1,  t,  0)),
        glm::normalize(glm::vec3(1,  t,  0)),
        glm::normalize(glm::vec3(-1, -t,  0)),
        glm::normalize(glm::vec3(1, -t,  0)),

        glm::normalize(glm::vec3(0, -1,  t)),
        glm::normalize(glm::vec3(0,  1,  t)),
        glm::normalize(glm::vec3(0, -1, -t)),
        glm::normalize(glm::vec3(0,  1, -t)),

        glm::normalize(glm::vec3(t,  0, -1)),
        glm::normalize(glm::vec3(t,  0,  1)),
        glm::normalize(glm::vec3(-t,  0, -1)),
        glm::normalize(glm::vec3(-t,  0,  1)),
    };

    std::vector<Index> indices = {
        0,11,5,  0,5,1,   0,1,7,   0,7,10,  0,10,11,
        1,5,9,   5,11,4,  11,10,2, 10,7,6,  7,1,8,
        3,9,4,   3,4,2,   3,2,6,   3,6,8,   3,8,9,
        4,9,5,   2,4,11,  6,2,10,  8,6,7,   9,8,1
    };

    // Subdivide faces
    for (unsigned int s = 0; s < subdivisions; ++s)
    {
        std::unordered_map<EdgeKey, uint32_t, EdgeKeyHash> cache;
        cache.reserve(indices.size()); // rough

        std::vector<Index> newIndices;
        newIndices.reserve(indices.size() * 4); // each tri -> 4 tris

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            uint32_t i0 = indices[i + 0];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            uint32_t a = GetMidpointIndex(i0, i1, unitPos, cache);
            uint32_t b = GetMidpointIndex(i1, i2, unitPos, cache);
            uint32_t c = GetMidpointIndex(i2, i0, unitPos, cache);

            // 4 new triangles
            newIndices.push_back(i0); newIndices.push_back(a);  newIndices.push_back(c);
            newIndices.push_back(i1); newIndices.push_back(b);  newIndices.push_back(a);
            newIndices.push_back(i2); newIndices.push_back(c);  newIndices.push_back(b);
            newIndices.push_back(a);  newIndices.push_back(b);  newIndices.push_back(c);
        }

        indices = std::move(newIndices);
    }

    // Build Vertex list
    std::vector<Vertex> vertices;
    vertices.reserve(unitPos.size());

    for (const glm::vec3& pUnit : unitPos)
    {
        const glm::vec3 pos = pUnit * radius;
        const glm::vec3 nrm = pUnit; // already unit

        // UV for icosphere: spherical mapping
        const float u = 0.5f + std::atan2(nrm.z, nrm.x) / (2.0f * 3.14159265359f);
        const float v = 0.5f - std::asin(nrm.y) / 3.14159265359f;

        Vertex vtx{};
        vtx.pos = pos;
        vtx.normal = nrm;
        vtx.texCoords = glm::vec2(u, v);
        vtx.color = white;

        vertices.push_back(vtx);
    }

    mesh.Create(std::move(vertices), std::move(indices));
    return mesh;
}