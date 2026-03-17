#pragma once

#include <glm/glm.hpp>
#include "MathUtils.h"

constexpr int CHUNK_SIZE = 16;
constexpr struct ChunkSize
{
    int x = CHUNK_SIZE;
    int y = CHUNK_SIZE;
    int z = CHUNK_SIZE;

} CHUNK_SIZE_3D;

struct ChunkPos;

struct BlockPos
{
    int x;
    int y;
    int z;

    constexpr BlockPos() noexcept : x(0), y(0), z(0) {}
    constexpr BlockPos(int x, int y, int z) noexcept : x(x), y(y), z(z) {}

    BlockPos(const glm::vec3& v) noexcept // constexpr only on C++23+
    {
        x = (int)std::floor(v.x);
        y = (int)std::floor(v.y);
        z = (int)std::floor(v.z);
    }
    explicit constexpr BlockPos(const ChunkPos& c) noexcept;

    operator glm::vec3() const noexcept
    {
        return glm::vec3((float)x, (float)y, (float)z);
    }

    [[nodiscard]]
    constexpr BlockPos operator+(const BlockPos& other) const noexcept
    {
        return { x + other.x, y + other.y, z + other.z };
    }

    [[nodiscard]]
    constexpr BlockPos operator-(const BlockPos& other) const noexcept
    {
        return { x - other.x, y - other.y, z - other.z };
    }

    constexpr bool operator==(const BlockPos& other) const noexcept
    {
        return x == other.x && y == other.y && z == other.z;
    }

    constexpr bool operator!=(const BlockPos& other) const noexcept
    {
        return !(*this == other);
    }

    constexpr BlockPos& operator+=(const BlockPos& other) noexcept
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr BlockPos& operator-=(const BlockPos& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
};

struct ChunkPos
{
    int x;
    int y;
    int z;

    constexpr ChunkPos() noexcept : x(0), y(0), z(0) {}
    constexpr ChunkPos(int x, int y, int z) noexcept : x(x), y(y), z(z) {}

    constexpr ChunkPos(const BlockPos& b) noexcept
        : x(math::floorDiv(b.x, CHUNK_SIZE)),
        y(math::floorDiv(b.y, CHUNK_SIZE)),
        z(math::floorDiv(b.z, CHUNK_SIZE))
    {
    }

    [[nodiscard]]
    constexpr ChunkPos operator+(const ChunkPos& other) const noexcept
    {
        return { x + other.x, y + other.y, z + other.z };
    }

    [[nodiscard]]
    constexpr ChunkPos operator-(const ChunkPos& other) const noexcept
    {
        return { x - other.x, y - other.y, z - other.z };
    }

    constexpr bool operator==(const ChunkPos& other) const noexcept
    {
        return x == other.x && y == other.y && z == other.z;
    }

    constexpr bool operator!=(const ChunkPos& other) const noexcept
    {
        return !(*this == other);
    }

    constexpr ChunkPos& operator+=(const ChunkPos& other) noexcept
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr ChunkPos& operator-=(const ChunkPos& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
};

constexpr BlockPos::BlockPos(const ChunkPos& c) noexcept
    : x(c.x* CHUNK_SIZE),
    y(c.y* CHUNK_SIZE),
    z(c.z* CHUNK_SIZE)
{
}

[[nodiscard]]
constexpr BlockPos localPos(BlockPos b) noexcept // 12 bytes better to copy than take the ref
{
    return {
        math::mod(b.x, CHUNK_SIZE),
        math::mod(b.y, CHUNK_SIZE),
        math::mod(b.z, CHUNK_SIZE)
    };
}

// returns local block pos in the chunk
[[nodiscard]]
constexpr inline BlockPos WorldToLocal(const BlockPos& pos) noexcept
{
    return {
        (pos.x % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE,
        (pos.y % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE,
        (pos.z % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE
    };
}

// returns chunk pos in the world
[[nodiscard]]
constexpr inline ChunkPos WorldToChunk(const glm::vec3& pos) noexcept
{
    return ChunkPos{
        (int)std::floor(pos.x / CHUNK_SIZE),
        (int)std::floor(pos.y / CHUNK_SIZE),
        (int)std::floor(pos.z / CHUNK_SIZE)
    };
}

namespace std
{
    template<>
    struct hash<BlockPos>
    {
        size_t operator()(const BlockPos& p) const noexcept
        {
            return (static_cast<size_t>(p.x) * 73856093) ^
                (static_cast<size_t>(p.y) * 19349663) ^
                (static_cast<size_t>(p.z) * 83492791);
        }
    };

    template<>
    struct hash<ChunkPos>
    {
        size_t operator()(const ChunkPos& p) const noexcept
        {
            return (static_cast<size_t>(p.x) * 83492791) ^
                (static_cast<size_t>(p.y) * 19349663) ^
                (static_cast<size_t>(p.z) * 73856093);
        }
    };
}