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
#include "Logger.h"

const double TICK_RATE = 20.0;
const double TICK_TIME = 1.0 / TICK_RATE; // 0.05 sec

int main()
{
    Engine engine;
    engine.Init();
	LOG("Engine created");

    TextureAtlas& atlas = TextureAtlas::Instance();
    atlas.BuildAtlas();
    atlas.UploadToOpenGL(/*generateMipMaps*/ true);
	LOG("Texture atlas built and uploaded to OpenGL");

    ShaderManager shaderManager;
    shaderManager.LoadShader(
        "C:/Users/Oximer/source/repos/MOX/Shaders/4.3.shader.vs",
        "C:/Users/Oximer/source/repos/MOX/Shaders/4.3.shader.fs"
    );

    Shader* worldShader = &shaderManager.GetShader(0);

#ifdef DEBUG
    World world(2, worldShader);
#else
    World world(8, worldShader);
#endif // DEBUG

    int tickCounter = 0;
    int yForTest = 30;

	Block pickedBlock = BlockRegistry::GetBlock("dirt");

	// TODO: remove this, move to a separate InputAction logic (class, component, etc.)
    // Cause now i use concret numbers except GLFW defines, thats not good
    engine.GetRenderer().SetMouseButtonCallback(
        [&](int button, int action, int mods)
        {
            auto hit = world.Raycast(engine.GetCamera().Position(), engine.GetCamera().Forward(), 5.0f);
            if (button == 0 && action == 1)
            {
                if (hit.hit) {
                    world.SetBlock(hit.blockPos, AIR_BLOCK);
                }
            }

            if (button == 1 && action == 1)
            {
                if (hit.hit) {
                    BlockPos placePos = hit.previousPos;
                    world.SetBlock(placePos, pickedBlock);
				}
            }

            if (button == 2 && action == 1)
            {
				std::cout << "Middle click\n";
                if (hit.hit) {
                    pickedBlock = BlockRegistry::GetBlock(world.GetBlock(hit.blockPos)->metadata.name);
                }
            }
        }
    );

    while (!engine.ShouldClose()) {
        while (engine.ShouldTick(TICK_TIME)) {
            world.Tick(WorldToChunk(engine.GetCamera().Position()));

            tickCounter++;

            if (tickCounter % 40 == 0)
            {
                world.SetBlock({ 0, yForTest, 0 }, BlockRegistry::GetBlock("dirt"));
                yForTest++;
				tickCounter = 0;
            }
        }
        engine.Frame(world.GetScene());
    }

    return 0;
}