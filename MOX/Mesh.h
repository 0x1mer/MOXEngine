#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>     // offsetof
#include <type_traits> // is_standard_layout_v

#include <glad/glad.h>

#include "Vertex.h"
#include "VAO.h"
#include "VBO.h"
#include "MOX_API.h"

using Index = uint32_t;

struct MOX_API MeshGPU {
    VAO    vao;
    VBO    vbo;
    GLuint ebo = 0;

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    [[nodiscard]] bool IsValid() const noexcept { return vbo.IsValid(); }
    [[nodiscard]] bool IsIndexed() const noexcept { return ebo != 0 && indexCount > 0; }

    MeshGPU() = default;
    MeshGPU(const MeshGPU&) = delete;
    MeshGPU& operator=(const MeshGPU&) = delete;
    MeshGPU(MeshGPU&&) noexcept = default;
    MeshGPU& operator=(MeshGPU&&) noexcept = default;
	~MeshGPU() { Destroy(); }

    void Create(const std::vector<Vertex>& vertices,
        const std::vector<Index>& indices = {})
    {
        static_assert(std::is_standard_layout_v<Vertex>, "Vertex must be standard-layout");

        Destroy();

        vertexCount = static_cast<uint32_t>(vertices.size());
        indexCount = static_cast<uint32_t>(indices.size());

        // 1) Build layout (reserve 4, not 3)
        LayersArray layout;
        layout.layers.reserve(4);
        layout.Add("aPos", 0, sizeof(Vertex), offsetof(Vertex, pos), 3, GL_FLOAT, GL_FALSE);
        layout.Add("aNormal", 1, sizeof(Vertex), offsetof(Vertex, normal), 3, GL_FLOAT, GL_FALSE);
        layout.Add("aTexCoord", 2, sizeof(Vertex), offsetof(Vertex, texCoords), 2, GL_FLOAT, GL_FALSE);
        layout.Add("aColor", 3, sizeof(Vertex), offsetof(Vertex, color), 3, GL_FLOAT, GL_FALSE);

        // 2) Upload VBO data (VAO::Init requires GL_ARRAY_BUFFER bound)
        vbo.SetData(static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
            vertices.data(),
            GL_STATIC_DRAW);

        // 3) Init VAO (will glBindVertexArray internally, set attribs, then unbind)
        vao.Init(layout);

        // 4) If indexed: create EBO and ATTACH it to VAO state
        if (!indices.empty()) {
            glGenBuffers(1, &ebo);
            glBindVertexArray(0);

            if (!vao.BindArray()) {
                assert(false && "Failed to bind VAO in MeshGPU::Create");
                return;
			}

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                indices.size() * sizeof(Index),
                indices.data(),
                GL_STATIC_DRAW);

            VAO::UnbindArray();
        }

        VBO::Unbind();
    }

    void Destroy()
    {
        vao.Destroy();
        vbo.Destroy();

        if (ebo) {
            glDeleteBuffers(1, &ebo);
            ebo = 0;
        }

        vertexCount = 0;
        indexCount = 0;
    }
};

struct MOX_API Mesh {
    MeshGPU gpu;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

	Mesh() = default;

    std::vector<Vertex> vertices;
    std::vector<Index>  indices;

    bool keepCpuCopy = false;

    void Create(std::vector<Vertex> vertices_,
        std::vector<Index> indices_ = {})
    {
        gpu.Create(vertices_, indices_);

        if (keepCpuCopy) {
            vertices = std::move(vertices_);
            indices = std::move(indices_);
        }
        else {
            vertices.clear();
            indices.clear();
        }
    }

    void ReleaseCpuCopy()
    {
        if (!keepCpuCopy) {
            vertices.clear(); vertices.shrink_to_fit();
            indices.clear();  indices.shrink_to_fit();
        }
    }

    void Destroy()
    {
        gpu.Destroy();
        vertices.clear();
        indices.clear();
    }
};
