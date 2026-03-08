#include "Chunk.h"
#include "BlockRegistry.h"

#include <cassert>

Chunk::Chunk()
    : position(0, 0, 0), dirty(true)
{
    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; ++i)
    {
        blocks[i] = Block({ 0, "air" });
	}
}

Chunk::Chunk(const ChunkPos& pos)
    : position(pos), dirty(true)
{
    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; ++i)
    {
        blocks[i] = Block({ 0, "air" });
    }
}

Chunk::~Chunk() = default;


Block& Chunk::GetBlock(const BlockPos& pos)
{
    assert(pos.x >= 0 && pos.x < CHUNK_SIZE);
    assert(pos.y >= 0 && pos.y < CHUNK_SIZE);
    assert(pos.z >= 0 && pos.z < CHUNK_SIZE);

    return blocks[Index(pos.x, pos.y, pos.z)];
}


const Block& Chunk::GetBlock(const BlockPos& pos) const
{
    assert(pos.x >= 0 && pos.x < CHUNK_SIZE);
    assert(pos.y >= 0 && pos.y < CHUNK_SIZE);
    assert(pos.z >= 0 && pos.z < CHUNK_SIZE);

    return blocks[Index(pos)];
}


void Chunk::SetBlock(const BlockPos& pos, const Block& block)
{
    assert(pos.x >= 0 && pos.x < CHUNK_SIZE);
    assert(pos.y >= 0 && pos.y < CHUNK_SIZE);
    assert(pos.z >= 0 && pos.z < CHUNK_SIZE);

    blocks[Index(pos)] = block;

    dirty = true;
}


void Chunk::BuildMesh()
{
    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;

    vertices_.reserve(32768 * 24);
    indices_.reserve(32768 * 36);

    for (int z = 0; z < CHUNK_SIZE; ++z) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {

                const Block& block = GetBlock({ x, y, z });
                if (block.metadata.id == 0 || block.metadata.name == "air")
                    continue;

                uint32_t vertexOffset = static_cast<uint32_t>(vertices_.size());

				const auto& blockType = BlockRegistry::Get(block.metadata.name);

                for (const Vertex& v : blockType.mesh.vertices)
                {
                    Vertex newVertex = v;
                    newVertex.pos += glm::vec3(x, y, z);
                    vertices_.push_back(newVertex);
                }

                for (uint32_t idx : blockType.mesh.indices)
                {
                    indices_.push_back(idx + vertexOffset);
                }
            }
        }
    }

    chunkMesh.Create(vertices_, indices_);
}

Model Chunk::CreateModel(Shader* shader) const
{
    Model model;

    model.AddMesh(
        chunkMesh,
        Transform{
            Position(
                position.x * CHUNK_SIZE,
                position.y * CHUNK_SIZE,
                position.z * CHUNK_SIZE
            )
        },
        shader
    );

    return model;
}