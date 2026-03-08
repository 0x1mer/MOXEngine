#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Vertex.h"
#include "TextureAtlas.h"

#include <nlohmann/json.hpp>

struct FaceMesh
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct BlockMesh
{
    FaceMesh north;
    FaceMesh south;
    FaceMesh west;
    FaceMesh east;
    FaceMesh up;
    FaceMesh down;
};

struct TextureVariant
{
    std::string name;
    float weight;

    TextureAtlas::UVRect uv;
};

struct TextureVariants
{
    std::vector<TextureVariant> variants;
};

struct BlockbenchModel
{
    BlockMesh mesh;

    std::unordered_map<std::string, std::string> textures;
    std::unordered_map<std::string, TextureVariants> textureVariants;

    std::vector<std::string> usedTextures;

    Shader* shader;
};

namespace bb
{
    using json = nlohmann::json;

    inline std::string Trim(std::string s)
    {
        s.erase(0, s.find_first_not_of(" \t\n\r"));
        s.erase(s.find_last_not_of(" \t\n\r") + 1);
        return s;
    }

    inline std::string ReadTextFile(const std::string& path)
    {
        std::ifstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("Failed to open file: " + path);
        std::ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    inline glm::vec3 ReadVec3(const json& arr)
    {
        return glm::vec3(
            arr.at(0).get<float>(),
            arr.at(1).get<float>(),
            arr.at(2).get<float>()
        );
    }

    inline glm::vec2 ReadVec2(const json& arr)
    {
        return glm::vec2(
            arr.at(0).get<float>(),
            arr.at(1).get<float>()
        );
    }

    inline glm::mat4 BuildRotationMatrix(const json& rotation, float unitScale)
    {
        // ѕоддержка двух форматов:
        // 1) {"angle": -27.5, "axis":"y", "origin":[...]}
        // 2) {"x":0,"y":-137.5,"z":0,"origin":[...]}  (как в твоЄм примере)
        glm::vec3 origin(0.0f);
        if (rotation.contains("origin"))
            origin = ReadVec3(rotation.at("origin")) * unitScale;

        glm::mat4 M(1.0f);
        M = glm::translate(M, origin);

        if (rotation.contains("angle") && rotation.contains("axis"))
        {
            float angleDeg = rotation.at("angle").get<float>();
            std::string axis = rotation.at("axis").get<std::string>();
            glm::vec3 ax(0.0f);

            if (axis == "x") ax = glm::vec3(1, 0, 0);
            else if (axis == "y") ax = glm::vec3(0, 1, 0);
            else if (axis == "z") ax = glm::vec3(0, 0, 1);
            else throw std::runtime_error("Unknown rotation axis: " + axis);

            M = glm::rotate(M, glm::radians(angleDeg), ax);
        }
        else
        {
            float x = rotation.value("x", 0.0f);
            float y = rotation.value("y", 0.0f);
            float z = rotation.value("z", 0.0f);

            // ѕор€док можно мен€ть Ч Blockbench обычно ожидает XYZ (как записано)
            M = glm::rotate(M, glm::radians(x), glm::vec3(1, 0, 0));
            M = glm::rotate(M, glm::radians(y), glm::vec3(0, 1, 0));
            M = glm::rotate(M, glm::radians(z), glm::vec3(0, 0, 1));
        }

        M = glm::translate(M, -origin);
        return M;
    }

    inline glm::vec3 TransformPos(const glm::mat4& M, const glm::vec3& p)
    {
        glm::vec4 v = M * glm::vec4(p, 1.0f);
        return glm::vec3(v);
    }

    inline glm::vec3 TransformNormal(const glm::mat4& M, const glm::vec3& n)
    {
        // дл€ чистой ротации можно просто M * vec4(n,0)
        glm::vec4 v = M * glm::vec4(n, 0.0f);
        return glm::normalize(glm::vec3(v));
    }

    struct FaceUV
    {
        // raw uv rectangle [u1,v1,u2,v2] from json
        float u1{}, v1{}, u2{}, v2{};
    };

    inline FaceUV ReadUV4(const json& uvArr)
    {
        FaceUV r;
        r.u1 = uvArr.at(0).get<float>();
        r.v1 = uvArr.at(1).get<float>();
        r.u2 = uvArr.at(2).get<float>();
        r.v2 = uvArr.at(3).get<float>();
        return r;
    }

    inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    inline glm::vec2 MapTileUVToAtlas(const TextureAtlas::UVRect& tileRect,
        float uTile, float vTile,
        float uvDivisor)
    {
        const float tu = uTile / uvDivisor; // 0..1
        const float tv = vTile / uvDivisor; // 0..1

        return glm::vec2(
            Lerp(tileRect.uvMin.x, tileRect.uvMax.x, tu),
            Lerp(tileRect.uvMin.y, tileRect.uvMax.y, tv)
        );
    }

    inline void AddQuad(
        FaceMesh& mesh,
        const glm::vec3& p0, const glm::vec3& p1,
        const glm::vec3& p2, const glm::vec3& p3,
        const glm::vec3& normal,
        const FaceUV& uvTile,
        const TextureAtlas::UVRect& tileRect,
        float uvDivisor,
        const glm::vec3& color = glm::vec3(1.0f))
    {
        const uint32_t base = static_cast<uint32_t>(mesh.vertices.size());

        const glm::vec2 t0 = MapTileUVToAtlas(tileRect, uvTile.u1, uvTile.v1, uvDivisor);
        const glm::vec2 t1 = MapTileUVToAtlas(tileRect, uvTile.u2, uvTile.v1, uvDivisor);
        const glm::vec2 t2 = MapTileUVToAtlas(tileRect, uvTile.u2, uvTile.v2, uvDivisor);
        const glm::vec2 t3 = MapTileUVToAtlas(tileRect, uvTile.u1, uvTile.v2, uvDivisor);

        mesh.vertices.push_back(Vertex{ p0, normal, t0, color });
        mesh.vertices.push_back(Vertex{ p1, normal, t1, color });
        mesh.vertices.push_back(Vertex{ p2, normal, t2, color });
        mesh.vertices.push_back(Vertex{ p3, normal, t3, color });

        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
    }

    inline std::string ResolveTextureRef(const std::string& texRef,
        const std::unordered_map<std::string, std::string>& textures)
    {
        // "#0" -> textures["0"]
        // "test" (встречаетс€ редко) -> "test"
        if (texRef.empty()) return {};

        if (texRef[0] == '#')
        {
            std::string key = texRef.substr(1);
            auto it = textures.find(key);
            if (it != textures.end()) return it->second;
            return {}; // неизвестна€ ссылка
        }

        return texRef;
    }

    inline std::vector<std::pair<std::string, float>> ParseWeightedTextures(const std::string& s)
    {
        std::vector<std::pair<std::string, float>> result;

        std::stringstream ss(s);
        std::string item;

        while (std::getline(ss, item, ','))
        {
            item = Trim(item);

            size_t colon = item.find(':');

            if (colon == std::string::npos)
            {
                result.emplace_back(item, 1.0f);
            }
            else
            {
                std::string name = Trim(item.substr(0, colon));
                float weight = std::stof(item.substr(colon + 1));

                result.emplace_back(name, weight);
            }
        }

        return result;
    }

    inline BlockbenchModel ParseBlockbenchModelFromJsonText(std::string_view text,
        TextureAtlas& atlas,
        float unitScale = 1.0f / 16.0f,
        float uvDivisor = 16.0f)
    {
        BlockbenchModel out;

        json root = json::parse(text.begin(), text.end());

        // textures
        if (root.contains("textures"))
        {
            for (auto it = root["textures"].begin(); it != root["textures"].end(); ++it)
            {
                std::string value = it.value().get<std::string>();

                out.textures[it.key()] = value;

                auto parsed = ParseWeightedTextures(value);

                TextureVariants variants;

                for (auto& [name, weight] : parsed)
                {
                    if (!atlas.Has(name))
                        throw std::runtime_error("TextureAtlas missing tile: " + name);

                    TextureVariant v;
                    v.name = name;
                    v.weight = weight;
                    v.uv = atlas.GetUV(name);

                    variants.variants.push_back(v);
                }

                out.textureVariants[it.key()] = std::move(variants);
            }
        }

        std::unordered_set<std::string> usedTexSet;

        // elements
        if (!root.contains("elements") || !root["elements"].is_array())
            throw std::runtime_error("No elements[] in model");

        for (const auto& el : root["elements"])
        {
            glm::vec3 from = ReadVec3(el.at("from")) * unitScale;
            glm::vec3 to = ReadVec3(el.at("to")) * unitScale;

            // AABB corners (local, before rotation)
            const glm::vec3 p000(from.x, from.y, from.z);
            const glm::vec3 p001(from.x, from.y, to.z);
            const glm::vec3 p010(from.x, to.y, from.z);
            const glm::vec3 p011(from.x, to.y, to.z);

            const glm::vec3 p100(to.x, from.y, from.z);
            const glm::vec3 p101(to.x, from.y, to.z);
            const glm::vec3 p110(to.x, to.y, from.z);
            const glm::vec3 p111(to.x, to.y, to.z);

            glm::mat4 R(1.0f);
            bool hasRotation = el.contains("rotation");
            if (hasRotation)
                R = BuildRotationMatrix(el["rotation"], unitScale);

            auto addFace = [&](FaceMesh& target,
                const char* faceName,
                glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
                glm::vec3 nLocal)
                {
                    if (!el.contains("faces")) return;
                    const auto& faces = el["faces"];
                    if (!faces.contains(faceName)) return;

                    const auto& f = faces[faceName];

                    std::string texRef = f.value("texture", "");

                    if (texRef == "#missing" || texRef.empty())
                        return;

                    std::string key;

                    if (texRef[0] == '#')
                        key = texRef.substr(1);
                    else
                        key = texRef;

                    auto it = out.textureVariants.find(key);
                    if (it == out.textureVariants.end())
                        return;

                    const auto& variants = it->second.variants;
                    if (variants.empty())
                        return;

                    // используем первый вариант дл€ UV mesh
                    const auto& v = variants.front();
                    const auto tileRect = v.uv;

                    // сохран€ем все текстуры как используемые
                    for (const auto& t : variants)
                        usedTexSet.insert(t.name);

                    FaceUV uv{};
                    if (f.contains("uv")) uv = ReadUV4(f["uv"]);
                    else uv = FaceUV{ 0,0, 16,16 };

                    if (hasRotation)
                    {
                        a = TransformPos(R, a);
                        b = TransformPos(R, b);
                        c = TransformPos(R, c);
                        d = TransformPos(R, d);
                        nLocal = TransformNormal(R, nLocal);
                    }

                    AddQuad(target, a, b, c, d, nLocal, uv, tileRect, uvDivisor);
                };

            // north (-Z)
            addFace(out.mesh.north, "north",
                p100, p000, p010, p110,
                glm::vec3(0, 0, -1));

            addFace(out.mesh.south, "south",
                p001, p101, p111, p011,
                glm::vec3(0, 0, 1));

            addFace(out.mesh.west, "west",
                p000, p001, p011, p010,
                glm::vec3(-1, 0, 0));

            addFace(out.mesh.east, "east",
                p101, p100, p110, p111,
                glm::vec3(1, 0, 0));

            addFace(out.mesh.down, "down",
                p000, p100, p101, p001,
                glm::vec3(0, -1, 0));

            addFace(out.mesh.up, "up",
                p010, p011, p111, p110,
                glm::vec3(0, 1, 0));
        }

        out.usedTextures.assign(usedTexSet.begin(), usedTexSet.end());
        return out;
    }

    inline BlockbenchModel ParseBlockbenchModelFromFile(const std::string& path,
        float unitScale = 1.0f / 16.0f,
        float uvDivisor = 16.0f)
    {
        const std::string txt = ReadTextFile(path);
        return ParseBlockbenchModelFromJsonText(txt, TextureAtlas::Instance(), unitScale, uvDivisor);
    }
} // namespace bb