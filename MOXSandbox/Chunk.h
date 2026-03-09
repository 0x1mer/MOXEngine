#pragma once

#include "Block.h"
#include "Mesh.h"
#include "Model.h"

constexpr int CHUNK_SIZE = 16;

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

	bool operator==(const ChunkPos&) const = default;
};

inline ChunkPos WorldToChunk(const glm::vec3& pos)
{
	return ChunkPos{
		(int)std::floor(pos.x / CHUNK_SIZE),
		(int)std::floor(pos.y / CHUNK_SIZE),
		(int)std::floor(pos.z / CHUNK_SIZE)
	};
}

template<>
struct std::hash<ChunkPos>
{
	size_t operator()(const ChunkPos& p) const noexcept
	{
		return ((size_t)p.x * 73856093) ^
			((size_t)p.y * 19349663) ^
			((size_t)p.z * 83492791);
	}
};

class Chunk
{
public:
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

	bool dirty = false;
};