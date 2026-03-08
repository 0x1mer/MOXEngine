#pragma once

#include "BlockRegistry.h"
#include "BlockAutoRegister.h"
#include "BlocksMeshes.h"

struct GrassBlock
{
    BlockType operator()() const
    {
        BlockType block;

        BlockbenchModel bbModel =
            bb::ParseBlockbenchModelFromFile(
                "C:/Users/Oximer/source/repos/MOX/MOXSandbox/BlockModels/grass_block.json"
            );

		block.flags = BlockFlags::Default;

		if (bbModel.textureVariants.size() > 0)
        {
			block.flags |= BlockFlags::TextureVariants;
            block.textureVariants = bbModel.textureVariants;
        }

        block.metadata.name = "grass_block";
        block.flags |= BlockFlags::FullCube;
        block.mesh = bbModel.mesh;

        return block;
    }
};

REGISTER_BLOCK(GrassBlock)