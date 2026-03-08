#pragma once

#include "BlockTypes.h"
#include "BlocksMeshes.h"

#include <string_view>

enum class BlockFlags : uint8_t
{
    None = 0x00,
    Default = 0x01,
    FullCube = 0x02,
    TextureVariants = 0x04,
    //D = 0x08,
    //E = 0x10,
    //F = 0x20,
    //G = 0x40,
    //H = 0x80
};

inline bool HasFlag(BlockFlags flags, BlockFlags flag)
{
    return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flag)) != 0;
}

inline BlockFlags operator|(BlockFlags a, BlockFlags b)
{
    return static_cast<BlockFlags>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
        );
}

inline BlockFlags operator|=(BlockFlags& a, BlockFlags b)
{
    a = a | b;
    return a;
}

inline BlockFlags operator&(BlockFlags a, BlockFlags b)
{
    return static_cast<BlockFlags>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
        );
}

struct BlockMetadata {
    BlockTypeId id;
    std::string_view name;
};

struct BlockType {
    BlockMetadata metadata;
    BlockMesh mesh;
    BlockFlags flags;

    std::unordered_map<std::string, TextureVariants> textureVariants;
};

struct Block
{
    BlockMetadata metadata;

    Block() = default;
    Block(BlockMetadata metadata) : metadata(metadata) {}
};