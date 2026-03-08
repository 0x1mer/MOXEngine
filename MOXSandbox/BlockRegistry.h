#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <stdexcept>
#include <cassert>

#include "Block.h"

static const Block AIR_BLOCK{
    {
        .id = 0,
        .name = "air"
    }
};

class BlockRegistry
{
public:

    static void Init()
    {
        if (initialized)
            return;

        BlockType air;
        air.metadata.name = "air";
        air.flags = BlockFlags::None;

        auto& nameMap = GetNameToIdMap();
        auto& blocks = GetBlocks();

        blocks.push_back(air);
        nameMap.emplace(air.metadata.name, 0);

        initialized = true;
    }

    static BlockTypeId Register(BlockType blockType)
    {
        EnsureInitialized();

        assert(!blockType.metadata.name.empty() && "Block name cannot be empty");

        auto& nameMap = GetNameToIdMap();
        auto& blocks = GetBlocks();

        assert(nameMap.find(std::string(blockType.metadata.name)) == nameMap.end() &&
            "Block already registered");

        BlockTypeId id = static_cast<BlockTypeId>(blocks.size());
         
        blockType.metadata.id = id;

        blocks.push_back(blockType);
        nameMap.emplace(blockType.metadata.name, id);

        return id;
    }

    static const BlockType& Get(BlockTypeId id)
    {
        EnsureInitialized();

        const auto& blocks = GetBlocks();

        assert(id < blocks.size() && "Invalid Block ID");

        return blocks[id];
    }

    static BlockTypeId GetId(std::string_view name)
    {
        EnsureInitialized();

        const auto& map = GetNameToIdMap();
        auto it = map.find(std::string(name));

        assert(it != map.end() && "Unknown block");

        return it->second;
    }

    static const BlockType& Get(std::string_view name)
    {
        return Get(GetId(name));
    }

    static const Block& GetBlock(std::string_view name)
    {
        const auto& type = Get(name);
        return Block{ type.metadata };
	}

private:

    static void EnsureInitialized()
    {
        if (!initialized)
            throw std::runtime_error("BlockRegistry not initialized");
    }

    static std::vector<BlockType>& GetBlocks()
    {
        static std::vector<BlockType> blocks;
        return blocks;
    }

    static std::unordered_map<std::string, BlockTypeId>& GetNameToIdMap()
    {
        static std::unordered_map<std::string, BlockTypeId> nameToId;
        return nameToId;
    }

private:
    static inline bool initialized = false;
};