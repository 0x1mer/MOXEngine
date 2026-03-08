#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "MOX_API.h"

class MOX_API TextureAtlas
{
public:
    struct UVRect
    {
        glm::vec2 uvMin; // (u0, v0)
        glm::vec2 uvMax; // (u1, v1)
    };

public:
    static TextureAtlas& Instance();

    explicit TextureAtlas(int tileSize = 16, int padding = 1);

    bool AddTexture(std::string name, const std::string& pngPath);

    bool BuildAtlas();
    void UploadToOpenGL(bool generateMipmaps = true);

    GLuint GetGLTexture() const { return m_glTex; }

    int Width() const { return m_atlasW; }
    int Height() const { return m_atlasH; }

    UVRect GetUV(const std::string& name) const;

    bool Has(const std::string& name) const;

    void Clear();

private:
    struct Tile
    {
        std::string name;
        std::vector<uint8_t> rgba;
    };

private:
    bool LoadPng16x16RGBA(const std::string& path, std::vector<uint8_t>& outRGBA) const;
    void DestroyGL();

private:
    int m_tileSize = 16;

    // 8 cause in later i want to add more mip maps
    int m_padding = 8;

    std::vector<Tile> m_tiles;
    std::unordered_map<std::string, UVRect> m_uvByName;

    GLuint m_glTex = 0;
    int m_atlasW = 0;
    int m_atlasH = 0;

    std::vector<uint8_t> m_atlas;
};