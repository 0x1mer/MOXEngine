#include "pch.h"
#include "TextureAtlas.h"

#include <stdexcept>
#include <cmath>
#include <iostream>
#include <algorithm>

// stb_image (PNG)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TextureAtlas& TextureAtlas::Instance()
{
    static TextureAtlas instance;
    return instance;
}

TextureAtlas::TextureAtlas(int tileSize, int padding)
    : m_tileSize(tileSize), m_padding(padding)
{
    if (m_tileSize <= 0) throw std::runtime_error("TextureAtlas: tileSize must be > 0");
    if (m_padding < 0) throw std::runtime_error("TextureAtlas: padding must be >= 0");
}

bool TextureAtlas::AddTexture(std::string name, const std::string& pngPath)
{
    if (name.empty()) return false;
    if (Has(name)) return false;

    Tile t;
    t.name = std::move(name);

    if (!LoadPng16x16RGBA(pngPath, t.rgba))
        return false;

    m_tiles.emplace_back(std::move(t));

    LOG("Added tile:" + std::string(name) + "from" + std::string(pngPath));

    return true;
}

bool TextureAtlas::Has(const std::string& name) const
{
    for (const auto& t : m_tiles)
        if (t.name == name) return true;
    return false;
}

TextureAtlas::UVRect TextureAtlas::GetUV(const std::string& name) const
{
    return m_uvByName.at(name);
}

void TextureAtlas::Clear()
{
    m_tiles.clear();
    m_uvByName.clear();
    m_atlasW = 0;
    m_atlasH = 0;
    DestroyGL();
}

bool TextureAtlas::LoadPng16x16RGBA(const std::string& path, std::vector<uint8_t>& outRGBA) const
{
    int w = 0, h = 0, comp = 0;
    stbi_uc* data = stbi_load(path.c_str(), &w, &h, &comp, 4); // force RGBA
    if (!data) return false;

    const bool ok = (w == m_tileSize && h == m_tileSize);
    if (!ok)
    {
        stbi_image_free(data);
        return false;
    }

    outRGBA.assign(data, data + (w * h * 4));
    stbi_image_free(data);
    return true;
}

void TextureAtlas::DestroyGL()
{
    if (m_glTex != 0)
    {
        glDeleteTextures(1, &m_glTex);
        m_glTex = 0;
    }
}

void TextureAtlas::UploadToOpenGL(bool generateMipmaps)
{
    // === Upload to OpenGL ===
    if (m_glTex == 0)
        glGenTextures(1, &m_glTex);

    glBindTexture(GL_TEXTURE_2D, m_glTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        m_atlasW,
        m_atlasH,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        m_atlas.data()
    );

    if (generateMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

bool TextureAtlas::BuildAtlas()
{
    if (m_tiles.empty())
        return false;

    const int N = static_cast<int>(m_tiles.size());
    const int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(N))));
    const int rows = static_cast<int>(std::ceil(static_cast<float>(N) / static_cast<float>(cols)));

    const int cell = m_tileSize + 2 * m_padding;
    m_atlasW = cols * cell;
    m_atlasH = rows * cell;

    m_atlas = std::vector<uint8_t>(static_cast<size_t>(m_atlasW) * static_cast<size_t>(m_atlasH) * 4, 0);

    m_uvByName.clear();
    m_uvByName.reserve(m_tiles.size());

    auto blitTile = [&](int dstX, int dstY, const std::vector<uint8_t>& tileRGBA)
        {
            for (int y = 0; y < m_tileSize; ++y)
            {
                const int srcRow = y * m_tileSize * 4;
                const int dstRow = (dstY + y) * m_atlasW * 4 + dstX * 4;
                std::copy_n(tileRGBA.data() + srcRow, m_tileSize * 4, m_atlas.data() + dstRow);
            }

            if (m_padding > 0)
            {
                for (int y = 0; y < m_tileSize; ++y)
                {
                    const uint8_t* leftPx = tileRGBA.data() + (y * m_tileSize + 0) * 4;
                    const uint8_t* rightPx = tileRGBA.data() + (y * m_tileSize + (m_tileSize - 1)) * 4;

                    for (int p = 1; p <= m_padding; ++p)
                    {
                        std::copy_n(leftPx, 4, m_atlas.data() + ((dstY + y) * m_atlasW + (dstX - p)) * 4);
                        std::copy_n(rightPx, 4, m_atlas.data() + ((dstY + y) * m_atlasW + (dstX + (m_tileSize - 1) + p)) * 4);
                    }
                }

                for (int x = -m_padding; x < m_tileSize + m_padding; ++x)
                {
                    const int clampedX = std::clamp(x, 0, m_tileSize - 1);

                    const uint8_t* topPx = tileRGBA.data() + (0 * m_tileSize + clampedX) * 4;
                    const uint8_t* bottomPx = tileRGBA.data() + ((m_tileSize - 1) * m_tileSize + clampedX) * 4;

                    for (int p = 1; p <= m_padding; ++p)
                    {
                        std::copy_n(topPx, 4, m_atlas.data() + (((dstY - p) * m_atlasW + (dstX + x)) * 4));
                        std::copy_n(bottomPx, 4, m_atlas.data() + (((dstY + (m_tileSize - 1) + p) * m_atlasW + (dstX + x)) * 4));
                    }
                }
            }
        };

    for (int i = 0; i < N; ++i)
    {
        const int cx = i % cols;
        const int cy = i / cols;

        const int tileX = cx * cell + m_padding;
        const int tileY = cy * cell + m_padding;

        blitTile(tileX, tileY, m_tiles[i].rgba);

        const float u0 = static_cast<float>(tileX) / static_cast<float>(m_atlasW);
        const float v0 = static_cast<float>(tileY) / static_cast<float>(m_atlasH);
        const float u1 = static_cast<float>(tileX + m_tileSize) / static_cast<float>(m_atlasW);
        const float v1 = static_cast<float>(tileY + m_tileSize) / static_cast<float>(m_atlasH);

        m_uvByName[m_tiles[i].name] = UVRect{ {u0, v0}, {u1, v1} };
    }

    return true;
}