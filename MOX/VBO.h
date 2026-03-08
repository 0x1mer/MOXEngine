#pragma once

#include "MOX_API.h"

#include <glad/glad.h>
#include <cstddef>
#include <cstdint>
#include <cassert>

class MOX_API VBO
{
private:
    GLuint m_VBO = 0;
    GLsizeiptr m_size = 0;
    GLenum m_usage = GL_STATIC_DRAW;

public:
    VBO() = default;
    ~VBO() { Destroy(); }

    VBO(const VBO&) = delete;
    VBO& operator=(const VBO&) = delete;

    VBO(VBO&& other) noexcept
        : m_VBO(other.m_VBO), m_size(other.m_size), m_usage(other.m_usage)
    {
        other.m_VBO = 0;
        other.m_size = 0;
    }

    VBO& operator=(VBO&& other) noexcept
    {
        if (this != &other) {
            Destroy();
            m_VBO = other.m_VBO;
            m_size = other.m_size;
            m_usage = other.m_usage;
            other.m_VBO = 0;
            other.m_size = 0;
        }
        return *this;
    }

    [[nodiscard]] bool IsValid() const noexcept { return m_VBO != 0; }
    [[nodiscard]] GLuint GetId() const noexcept { return m_VBO; }
    [[nodiscard]] GLsizeiptr GetSize() const noexcept { return m_size; }

    void Create()
    {
        if (m_VBO == 0) {
            glGenBuffers(1, &m_VBO);
            assert(m_VBO != 0 && "glGenBuffers failed");
        }
    }

    void Destroy()
    {
        if (m_VBO != 0) {
            glDeleteBuffers(1, &m_VBO);
            m_VBO = 0;
            m_size = 0;
        }
    }

    void Bind() const
    {
        assert(m_VBO != 0 && "VBO::Bind called on invalid buffer");
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    }

    static void Unbind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Allocate + upload data (also works with nullptr data for just allocating)
    void SetData(GLsizeiptr sizeBytes, const void* data, GLenum usage = GL_STATIC_DRAW)
    {
        assert(sizeBytes >= 0);
        Create();
        Bind();
        m_size = sizeBytes;
        m_usage = usage;
        glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, usage);
    }

    // Update a sub-range (buffer must be allocated already)
    void UpdateSubData(GLintptr offsetBytes, GLsizeiptr sizeBytes, const void* data)
    {
        assert(m_VBO != 0 && "VBO::UpdateSubData on invalid buffer");
        assert(data != nullptr && "UpdateSubData data is null");
        assert(offsetBytes >= 0 && sizeBytes >= 0);
        assert(offsetBytes + sizeBytes <= m_size && "UpdateSubData out of bounds");

        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, offsetBytes, sizeBytes, data);
    }
};