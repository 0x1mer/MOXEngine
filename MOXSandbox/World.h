#pragma once

#include <limits>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "ChunkController.h"

struct RaycastHit
{
    bool hit = false;

    BlockPos blockPos{ 0,0,0 };
    BlockPos previousPos{ 0,0,0 };

    glm::ivec3 normal{ 0,0,0 };
    glm::vec3 hitPoint{ 0,0,0 };

    float distance = 0.0f;
};

class World {
private:
	ChunkController m_chunkController;
	Scene m_scene;

	ChunkPos m_cachedPlayerChunkPos{ INT_MAX, INT_MAX, INT_MAX };

public:
	World(size_t chunkLoadRadius_, Shader* worldShader)
		: m_scene(),
		m_chunkController(chunkLoadRadius_, &m_scene, worldShader)
	{

	}

	// Todo: write correct world save\unload
	~World() = default;

	Scene& GetScene() { return m_scene; }

	// must be called from app, by every tick
	void Tick(const ChunkPos& playerChunkPosition) {
		if (m_cachedPlayerChunkPos != playerChunkPosition)
		{
			m_cachedPlayerChunkPos = playerChunkPosition;
			m_chunkController.UpdateChunks(playerChunkPosition);
		}
		m_chunkController.ProcessQueues();
	}

	// block utils //
	const Block* GetBlock(const BlockPos& worldPos)
	{
		ChunkPos chunkPos = WorldToChunk(glm::vec3(worldPos.x, worldPos.y, worldPos.z));

		Chunk* chunk = m_chunkController.GetChunk(chunkPos);
		if (!chunk) return nullptr;

		BlockPos local = WorldToLocal(worldPos);

		return &chunk->GetBlock(local);
	}

    void SetBlock(const BlockPos& pos, const Block& block)
    {
        ChunkPos chunkPos = WorldToChunk(glm::vec3(pos.x, pos.y, pos.z));
        Chunk* chunk = m_chunkController.GetOrCreateChunk(chunkPos);

        assert(chunk);

        BlockPos local = WorldToLocal(pos);

        chunk->SetBlock(local, block);
        m_chunkController.ForceRemeshChunk(chunk);

        constexpr int max = CHUNK_SIZE - 1;

        // X-
        if (local.x == 0)
        {
            if (Chunk* n = m_chunkController.GetChunk({ chunkPos.x - 1, chunkPos.y, chunkPos.z }))
                m_chunkController.ForceRemeshChunk(n);
        }

        // X+
        if (local.x == max)
        {
            if (Chunk* n = m_chunkController.GetChunk({ chunkPos.x + 1, chunkPos.y, chunkPos.z }))
                m_chunkController.ForceRemeshChunk(n);
        }

        // Y-
        if (local.y == 0)
        {
            if (Chunk* n = m_chunkController.GetChunk({ chunkPos.x, chunkPos.y - 1, chunkPos.z }))
                m_chunkController.ForceRemeshChunk(n);
        }

        // Y+
        if (local.y == max)
        {
            if (Chunk* n = m_chunkController.GetChunk({ chunkPos.x, chunkPos.y + 1, chunkPos.z }))
                m_chunkController.ForceRemeshChunk(n);
        }

        // Z-
        if (local.z == 0)
        {
            if (Chunk* n = m_chunkController.GetChunk({ chunkPos.x, chunkPos.y, chunkPos.z - 1 }))
                m_chunkController.ForceRemeshChunk(n);
        }

        // Z+
        if (local.z == max)
        {
            if (Chunk* n = m_chunkController.GetChunk({ chunkPos.x, chunkPos.y, chunkPos.z + 1 }))
                m_chunkController.ForceRemeshChunk(n);
        }
    }

    RaycastHit Raycast(glm::vec3 origin, glm::vec3 dir, float maxDistance)
    {
        RaycastHit hit{};

        if (glm::length2(dir) == 0.0f)
            return hit;

        dir = glm::normalize(dir);

        glm::ivec3 pos = glm::floor(origin);

        BlockPos startBlock{ pos.x, pos.y, pos.z };

        const Block* start = GetBlock(startBlock);
        if (start && start->metadata.id != 0)
        {
            hit.hit = true;
            hit.blockPos = startBlock;
            hit.previousPos = startBlock;
            hit.normal = { 0,0,0 };
            hit.hitPoint = origin;
            hit.distance = 0.0f;
            return hit;
        }

        glm::vec3 deltaDist{
            dir.x != 0 ? std::abs(1.0f / dir.x) : std::numeric_limits<float>::infinity(),
            dir.y != 0 ? std::abs(1.0f / dir.y) : std::numeric_limits<float>::infinity(),
            dir.z != 0 ? std::abs(1.0f / dir.z) : std::numeric_limits<float>::infinity()
        };

        glm::ivec3 step;
        glm::vec3 sideDist;

        for (int i = 0; i < 3; ++i)
        {
            if (dir[i] < 0)
            {
                step[i] = -1;
                sideDist[i] = (origin[i] - pos[i]) * deltaDist[i];
            }
            else
            {
                step[i] = 1;
                sideDist[i] = (pos[i] + 1.0f - origin[i]) * deltaDist[i];
            }
        }

        float distance = 0.0f;
        glm::ivec3 normal{ 0 };

        int maxSteps = int(maxDistance * 3.0f);
        int steps = 0;

        while (distance < maxDistance && steps++ < maxSteps)
        {
            if (sideDist.x < sideDist.y && sideDist.x < sideDist.z)
            {
                pos.x += step.x;
                distance = sideDist.x;
                sideDist.x += deltaDist.x;
                normal = { -step.x, 0, 0 };
            }
            else if (sideDist.y < sideDist.z)
            {
                pos.y += step.y;
                distance = sideDist.y;
                sideDist.y += deltaDist.y;
                normal = { 0, -step.y, 0 };
            }
            else
            {
                pos.z += step.z;
                distance = sideDist.z;
                sideDist.z += deltaDist.z;
                normal = { 0, 0, -step.z };
            }

            BlockPos blockPos{ pos.x, pos.y, pos.z };

            const Block* block = GetBlock(blockPos);

            if (block && block->metadata.id != 0)
            {
                hit.hit = true;
                hit.blockPos = blockPos;
                hit.normal = normal;
                hit.distance = distance;
                hit.hitPoint = origin + dir * distance;

                hit.previousPos =
                {
                    blockPos.x + normal.x,
                    blockPos.y + normal.y,
                    blockPos.z + normal.z
                };

                return hit;
            }
        }

        return hit;
    }

};