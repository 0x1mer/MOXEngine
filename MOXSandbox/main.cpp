#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "Engine.h"
#include "Scene.h"

#include "BlocksMeshes.h"
#include "TextureAtlas.h"
#include "Chunk.h"
#include "BlockRegistry.h"

#include "BlocksIncluder.h"

int main()
{
    Engine engine;
    engine.Init();

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

    Chunk chunk({ 0,0,0 });

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            float fx = (float)x * 0.5f;
            float fz = (float)z * 0.5f;

            float noise =
                sinf(fx * 0.8f) +
                cosf(fz * 0.8f);

            int height = 3 + (int)(noise * 2.0f);

            for (int y = 0; y <= height; y++)
            {
                if (y == height)
                    chunk.SetBlock({ x,y,z }, BlockRegistry::GetBlock("grass_block"));
                else
                    chunk.SetBlock({ x,y,z }, BlockRegistry::GetBlock("dirt"));
            }
        }
    }

    chunk.BuildMesh();

    scene.AddModel(chunk.CreateModel(shader));

    while (!engine.ShouldClose())
    {
        engine.Frame(scene);
    }

    return 0;
}