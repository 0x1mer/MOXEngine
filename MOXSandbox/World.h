#pragma once

#include "ChunkController.h"

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

};