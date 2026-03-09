#pragma once

#include "Block.h"
#include "Mesh.h"
#include "Model.h"

#include "Coords.h"

class Chunk
{
public:

	struct NearestChunks {
		Chunk* left = nullptr;
		Chunk* right = nullptr;
		Chunk* front = nullptr;
		Chunk* back = nullptr;
		Chunk* up = nullptr;
		Chunk* down = nullptr;
	} nearestChunks;

	Chunk();
	Chunk(const ChunkPos& pos);
	~Chunk();

	using SceneId = uint64_t;
	SceneId sceneNode = UINT64_MAX;

	Block& GetBlock(const BlockPos& pos);
	const Block& GetBlock(const BlockPos& pos) const;
	void SetBlock(const BlockPos& pos, const Block& block);

	void BuildMesh();
	Model CreateModel(Shader* shader) const;

	bool MarkDirty()
	{
		if (m_dirty)
			return false;

		m_dirty = true;
		return true;
	}
	void MarkClean() { m_dirty = false; }
	bool IsDirty() const { return m_dirty; }

private:
	static inline int Index(int x, int y, int z)
	{
		return x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
	}

	static inline int Index(const BlockPos& pos)
	{
		return pos.x + pos.y * CHUNK_SIZE + pos.z * CHUNK_SIZE * CHUNK_SIZE;
	}

	bool IsFaceVisible(const BlockPos& pos) const;
	bool IsAir(const BlockPos& pos);

private:
	Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];;
	ChunkPos position;
	MeshGPU chunkMesh;

	bool m_dirty = false;
};