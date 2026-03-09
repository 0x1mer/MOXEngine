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

const double TICK_RATE = 20.0;
const double TICK_TIME = 1.0 / TICK_RATE; // 0.05 sec

int main()
{
    Engine engine;
    engine.Init();

    double accumulator = 0.0;
    double lastTime = engine.GetTime();

    TextureAtlas& atlas = TextureAtlas::Instance();
    atlas.BuildAtlas();
    atlas.UploadToOpenGL(true);

    ShaderManager shaderManager;
    shaderManager.LoadShader(
        "C:/Users/Oximer/source/repos/MOX/Shaders/4.3.shader.vs",
        "C:/Users/Oximer/source/repos/MOX/Shaders/4.3.shader.fs"
    );

    Shader* shader = &shaderManager.GetShader(0);
    Scene scene;

    ChunkController chunkController(4, &scene, shader);

    ChunkPos cachedPlayerChunkPos(UINT64_MAX, UINT64_MAX, UINT64_MAX);

    while (!engine.ShouldClose())
    {
		// frame time calculation
        double currentTime = engine.GetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

		// lag compensation: if the game was paused or something, we don't want to update 1000 times in a row
        deltaTime = std::min(deltaTime, 0.25);

        accumulator += deltaTime;

		// fixed tickrate update
        while (accumulator >= TICK_TIME)
        {
            ChunkPos playerChunk = WorldToChunk(engine.GetCamera().Position());

            if (cachedPlayerChunkPos != playerChunk)
            {
                cachedPlayerChunkPos = playerChunk;
                chunkController.UpdateChunks(playerChunk);
            }

            chunkController.ProcessQueues();

            accumulator -= TICK_TIME;
        }

		// frame update
        engine.Frame(scene);
    }

    return 0;
}