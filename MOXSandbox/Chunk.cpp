#include "Chunk.h"
#include "BlockRegistry.h"

#include <cassert>

Chunk::Chunk()
    : position(0, 0, 0), m_dirty(true)
{
    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; ++i)
    {
        blocks[i] = Block({ 0, "air" });
	}
}

Chunk::Chunk(const ChunkPos& pos)
    : position(pos), m_dirty(true)
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

    m_dirty = true;
}

bool Chunk::IsAir(const BlockPos& pos)
{
    if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
        pos.x >= CHUNK_SIZE || pos.y >= CHUNK_SIZE || pos.z >= CHUNK_SIZE)
        return true;

    return GetBlock(pos).metadata.id == 0;
}

void AddFace(const FaceMesh& face, glm::vec3 pos, std::vector<Vertex>& vertices_, std::vector<uint32_t>& indices_)
{
    uint32_t vertexOffset = vertices_.size();

    for (const Vertex& v : face.vertices)
    {
        Vertex nv = v;
        nv.pos += pos;
        vertices_.push_back(nv);
    }

    for (uint32_t i : face.indices)
    {
        indices_.push_back(i + vertexOffset);
    }
}

bool Chunk::IsFaceVisible(const BlockPos& pos) const
{
    const Chunk* chunk = this;
    BlockPos local = pos;

    if (pos.x < 0)
    {
        chunk = nearestChunks.left;
        local.x = CHUNK_SIZE - 1;
    }
    else if (pos.x >= CHUNK_SIZE)
    {
        chunk = nearestChunks.right;
        local.x = 0;
    }
    else if (pos.y < 0)
    {
        chunk = nearestChunks.down;
        local.y = CHUNK_SIZE - 1;
    }
    else if (pos.y >= CHUNK_SIZE)
    {
        chunk = nearestChunks.up;
        local.y = 0;
    }
    else if (pos.z < 0)
    {
        chunk = nearestChunks.back;
        local.z = CHUNK_SIZE - 1;
    }
    else if (pos.z >= CHUNK_SIZE)
    {
        chunk = nearestChunks.front;
        local.z = 0;
    }

    if (!chunk)
        return true;

    const Block& neighbour = chunk->GetBlock(local);

    if (neighbour.metadata.id == 0)
        return true;

    const auto& neighbourType = BlockRegistry::Get(neighbour.metadata.id);

    return !HasFlag(neighbourType.flags, BlockFlags::FullCube);
}

void Chunk::BuildMesh()
{
    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;

    vertices_.reserve(8192);
    indices_.reserve(12288);

    for (int z = 0; z < CHUNK_SIZE; ++z) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {

                const Block& block = GetBlock({ x, y, z });
                if (block.metadata.id == 0)
                    continue;

				const auto& blockType = BlockRegistry::Get(block.metadata.id);

                if (IsFaceVisible({ x, y, z - 1 }))
                    AddFace(blockType.mesh.north, { x,y,z }, vertices_, indices_);

                if (IsFaceVisible({ x, y, z + 1 }))
                    AddFace(blockType.mesh.south, { x,y,z }, vertices_, indices_);

                if (IsFaceVisible({ x - 1, y, z }))
                    AddFace(blockType.mesh.west, { x,y,z }, vertices_, indices_);

                if (IsFaceVisible({ x + 1, y, z }))
                    AddFace(blockType.mesh.east, { x,y,z }, vertices_, indices_);

                if (IsFaceVisible({ x, y + 1, z }))
                    AddFace(blockType.mesh.up, { x,y,z }, vertices_, indices_);

                if (IsFaceVisible({ x, y - 1, z }))
                    AddFace(blockType.mesh.down, { x,y,z }, vertices_, indices_);
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