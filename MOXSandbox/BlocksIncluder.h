#pragma once

#include "BlockRegistry.h"
#include "TextureAtlas.h"

static bool blockRegistryInitialized = []()
    {
        BlockRegistry::Init();

        TextureAtlas::Instance().AddTexture("dirt", "C:/Users/Oximer/source/repos/MOX/MOXSandbox/Textures/dirt.png");
        TextureAtlas::Instance().AddTexture("grass_top_flowers", "C:/Users/Oximer/source/repos/MOX/MOXSandbox/Textures/grass_top_flowers.png");
        TextureAtlas::Instance().AddTexture("grass_top_lot_flowers", "C:/Users/Oximer/source/repos/MOX/MOXSandbox/Textures/grass_top_lot_flowers.png");
        TextureAtlas::Instance().AddTexture("grass_top", "C:/Users/Oximer/source/repos/MOX/MOXSandbox/Textures/grass_top.png");
		TextureAtlas::Instance().BuildAtlas();

        return true;
    }();

#include "Dirt.h"
#include "GrassBlock.h"