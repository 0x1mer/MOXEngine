#pragma once

#include "Block.h"
#include "Mesh.h"
#include "Model.h"

constexpr int CHUNK_SIZE = 32;

constexpr struct ChunkSize
{
	int x = CHUNK_SIZE;
	int y = CHUNK_SIZE;
	int z = CHUNK_SIZE;

} CHUNK_SIZE_3D;

struct BlockPos {
	int x;
	int y;
	int z;
	BlockPos(int x, int y, int z) : x(x), y(y), z(z) {}
};

struct ChunkPos {
	int x;
	int y;
	int z;
	ChunkPos(int x, int y, int z) : x(x), y(y), z(z) {}
};

class Chunk
{
public:
	Chunk();
	Chunk(const ChunkPos& pos);
	~Chunk();

	Block& GetBlock(const BlockPos& pos);
	const Block& GetBlock(const BlockPos& pos) const;

	void SetBlock(const BlockPos& pos, const Block& block);
	void BuildMesh();
	Model CreateModel(Shader* shader) const;

private:
	static inline int Index(int x, int y, int z)
	{
		return x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
	}

	static inline int Index(const BlockPos& pos)
	{
		return pos.x + pos.y * CHUNK_SIZE + pos.z * CHUNK_SIZE * CHUNK_SIZE;
	}

private:
	Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];;
	ChunkPos position;
	MeshGPU chunkMesh;

	bool dirty = false;
};