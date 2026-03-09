#pragma once

#include <queue>

#include "Chunk.h"
#include "Scene.h"
#include "BlockRegistry.h"

#include "MemOx/object_pool.hpp"

constexpr int kChunkRemeshByTickLimit = 4;

static constexpr ChunkPos s_directions[6] =
{
	{ 1, 0, 0 },
	{-1, 0, 0 },
	{ 0, 1, 0 },
	{ 0,-1, 0 },
	{ 0, 0, 1 },
	{ 0, 0,-1 }
};

static constexpr Chunk * Chunk::NearestChunks:: * s_thisPtrs[6] =
{
	&Chunk::NearestChunks::right,
	&Chunk::NearestChunks::left,
	&Chunk::NearestChunks::up,
	&Chunk::NearestChunks::down,
	&Chunk::NearestChunks::front,
	&Chunk::NearestChunks::back
};

static constexpr Chunk* Chunk::NearestChunks::* s_otherPtrs[6] =
{
	&Chunk::NearestChunks::left,
	&Chunk::NearestChunks::right,
	&Chunk::NearestChunks::down,
	&Chunk::NearestChunks::up,
	&Chunk::NearestChunks::back,
	&Chunk::NearestChunks::front
};

class ChunkController
{
private:

	void MarkNeighboursDirty(Chunk* chunk)
	{
		for (int i = 0; i < 6; i++)
		{
			Chunk* neighbour = chunk->nearestChunks.*(s_thisPtrs[i]);

			if (neighbour && neighbour->MarkDirty())
				m_meshQueue.push(neighbour);
		}
	}

	void LinkNeighbours(const ChunkPos& pos, Chunk* chunk)
	{
		for (int i = 0; i < 6; i++)
		{
			ChunkPos neighbourPos =
			{
				pos.x + s_directions[i].x,
				pos.y + s_directions[i].y,
				pos.z + s_directions[i].z
			};

			auto it = m_loadedChunks.find(neighbourPos);
			if (it == m_loadedChunks.end())
				continue;

			Chunk* neighbour = it->second.get();

			chunk->nearestChunks.*(s_thisPtrs[i]) = neighbour;
			neighbour->nearestChunks.*(s_otherPtrs[i]) = chunk;
		}
	}

	void UnlinkNeighbours(const ChunkPos& pos, Chunk* chunk)
	{
		for (int i = 0; i < 6; i++)
		{
			ChunkPos neighbourPos =
			{
				pos.x + s_directions[i].x,
				pos.y + s_directions[i].y,
				pos.z + s_directions[i].z
			};

			auto it = m_loadedChunks.find(neighbourPos);
			if (it == m_loadedChunks.end())
				continue;

			Chunk* neighbour = it->second.get();

			neighbour->nearestChunks.*(s_otherPtrs[i]) = nullptr;
		}
	}

private:
	int m_chunkLoadRadius = 5;
	ObjectPool<Chunk> m_chunkPool;
	Scene* m_scene = nullptr;
	Shader* m_chunkShader = nullptr;

	std::unordered_map<ChunkPos, PoolHandle<Chunk>, std::hash<ChunkPos>> m_loadedChunks;

	std::queue<ChunkPos> m_loadQueue;
	std::queue<ChunkPos> m_unloadQueue;
	std::queue<Chunk*> m_meshQueue;

	std::unordered_set<ChunkPos> m_loadSet;
	std::unordered_set<ChunkPos> m_unloadSet;

	int m_maxChunkLoadsPerTick = 4;
	int m_maxChunkUnloadsPerTick = 8;

public:
	explicit ChunkController(size_t chunkLoadRadius_, Scene* scene, Shader* chunkShader) :
		m_chunkLoadRadius(chunkLoadRadius_),
		m_chunkPool((chunkLoadRadius_ * 2 + 1)* (chunkLoadRadius_ * 2 + 1)* (chunkLoadRadius_ * 2 + 1) +
			0.2 * (chunkLoadRadius_ * 2 + 1) * (chunkLoadRadius_ * 2 + 1) * (chunkLoadRadius_ * 2 + 1)),
		m_scene(scene),
		m_chunkShader(chunkShader)
	{

	}
	// For now, later we need to correct unload chunks
	~ChunkController() = default;

	Chunk* GetChunk(const ChunkPos& pos)
	{
		auto it = m_loadedChunks.find(pos);
		if (it == m_loadedChunks.end())
			return nullptr;

		return it->second.get();
	}

	Chunk* GetOrCreateChunk(const ChunkPos& pos)
	{
		auto it = m_loadedChunks.find(pos);

		if (it != m_loadedChunks.end())
			return it->second.get();

		auto handle = m_chunkPool.emplace(pos);

		auto [iter, inserted] = m_loadedChunks.emplace(pos, std::move(handle));

		// Generate simple terrain for demonstration
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int y = 0; y < CHUNK_SIZE; y++)
				for (int z = 0; z < CHUNK_SIZE; z++)
				{
					if (y + pos.y * CHUNK_SIZE < 16)
						iter->second->SetBlock({ x,y,z }, BlockRegistry::GetBlock("dirt"));
					else if (y + pos.y * CHUNK_SIZE == 16)
						iter->second->SetBlock({ x,y,z }, BlockRegistry::GetBlock("grass_block"));
				}


		Chunk* chunk = iter->second.get();

		chunk->MarkDirty();
		m_meshQueue.push(chunk);

		LinkNeighbours(pos, chunk);
		MarkNeighboursDirty(chunk);

		return chunk;
	}

	void UnloadChunk(const ChunkPos& pos)
	{
		auto it = m_loadedChunks.find(pos);
		if (it == m_loadedChunks.end())
			return;

		Chunk* chunk = it->second.get();

		if (chunk->sceneNode != UINT64_MAX)
			m_scene->RemoveModel(chunk->sceneNode);

		for (int i = 0; i < 6; i++)
		{
			Chunk* neighbour = chunk->nearestChunks.*(s_thisPtrs[i]);

			if (!chunk || !chunk->IsDirty())
				continue;

			if (neighbour && neighbour->MarkDirty())
				m_meshQueue.push(neighbour);
		}
		UnlinkNeighbours(pos, chunk);

		it->second.reset();
		m_loadedChunks.erase(it);
	}

	void UpdateMeshes()
	{
		int limit = kChunkRemeshByTickLimit;
		while (!m_meshQueue.empty() && limit--)
		{
			Chunk* chunk = m_meshQueue.front();
			m_meshQueue.pop();

			if (!chunk->IsDirty())
				continue;

			chunk->BuildMesh();

			auto model = chunk->CreateModel(m_chunkShader);

			if (chunk->sceneNode != UINT64_MAX)
				m_scene->RemoveModel(chunk->sceneNode);

			chunk->sceneNode = m_scene->AddModel(std::move(model));

			chunk->MarkClean();
		}
	}

	void UpdateChunks(const ChunkPos& playerChunkPos)
	{
		std::unordered_set<ChunkPos> requiredChunks;

		for (int x = -m_chunkLoadRadius; x <= m_chunkLoadRadius; x++)
			for (int y = -m_chunkLoadRadius; y <= m_chunkLoadRadius; y++)
				for (int z = -m_chunkLoadRadius; z <= m_chunkLoadRadius; z++)
					requiredChunks.emplace(
						playerChunkPos.x + x,
						playerChunkPos.y + y,
						playerChunkPos.z + z
					);

		// planning unload
		for (const auto& [pos, handle] : m_loadedChunks)
		{
			if (!requiredChunks.contains(pos) && !m_unloadSet.contains(pos))
			{
				m_unloadQueue.push(pos);
				m_unloadSet.insert(pos);
			}
		}

		// planning load
		for (const auto& pos : requiredChunks)
		{
			if (!m_loadedChunks.contains(pos) && !m_loadSet.contains(pos))
			{
				m_loadQueue.push(pos);
				m_loadSet.insert(pos);
			}
		}
	}

	void ProcessQueues()
	{

		UpdateMeshes();

		// unload
		int unloads = 0;
		while (!m_unloadQueue.empty() && unloads < m_maxChunkUnloadsPerTick)
		{
			ChunkPos pos = m_unloadQueue.front();
			m_unloadQueue.pop();
			m_unloadSet.erase(pos);

			UnloadChunk(pos);

			unloads++;
		}

		// load
		int loads = 0;
		while (!m_loadQueue.empty() && loads < m_maxChunkLoadsPerTick)
		{
			ChunkPos pos = m_loadQueue.front();
			m_loadQueue.pop();
			m_loadSet.erase(pos);

			GetOrCreateChunk(pos);

			loads++;
		}
	}
};