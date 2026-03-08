#pragma once

#include "BlockRegistry.h"
#include "BlockAutoRegister.h"
#include "BlocksMeshes.h"

struct DirtBlock
{
    BlockType operator()() const
    {
        BlockType block;

        BlockbenchModel bbModel =
            bb::ParseBlockbenchModelFromFile(
                "C:/Users/Oximer/source/repos/MOX/MOXSandbox/BlockModels/dirt.json"
            );

        block.metadata.name = "dirt";
        block.flags = BlockFlags::FullCube;
        block.mesh = bbModel.mesh;

        return block;
    }
};

REGISTER_BLOCK(DirtBlock)