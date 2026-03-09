#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <algorithm>

#include "Engine.h"
#include "Scene.h"

#include "BlocksMeshes.h"
#include "TextureAtlas.h"
#include "Chunk.h"
#include "BlockRegistry.h"

#include "BlocksIncluder.h"
#include "ChunkController.h"

#include "World.h"

const double TICK_RATE = 20.0;
const double TICK_TIME = 1.0 / TICK_RATE; // 0.05 sec

int main()
{
    Engine engine;
    engine.Init();

    TextureAtlas& atlas = TextureAtlas::Instance();
    atlas.BuildAtlas();
    atlas.UploadToOpenGL(/*generateMipMaps*/ true);

    ShaderManager shaderManager;
    shaderManager.LoadShader(
        "C:/Users/Oximer/source/repos/MOX/Shaders/4.3.shader.vs",
        "C:/Users/Oximer/source/repos/MOX/Shaders/4.3.shader.fs"
    );

    Shader* worldShader = &shaderManager.GetShader(0);

	World world(5, worldShader);

    while (!engine.ShouldClose()) {
        while (engine.ShouldTick(TICK_TIME)) {
            world.Tick(WorldToChunk(engine.GetCamera().Position()));
        }
        engine.Frame(world.GetScene());
    }

    return 0;
}