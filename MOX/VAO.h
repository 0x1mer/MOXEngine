#pragma once

#include "MOX_API.h"

#include <glad/glad.h>
#include <vector>
#include <cstddef>   // size_t
#include <cstdint>   // uint8_t, int8_t
#include <cassert>   // assert

[[nodiscard]] constexpr std::size_t GLTypeSize(GLenum t) noexcept {
	switch (t) {
	case GL_FLOAT:          return sizeof(float);
	case GL_UNSIGNED_INT:   return sizeof(std::uint32_t);
	case GL_INT:            return sizeof(std::int32_t);
	case GL_UNSIGNED_BYTE:  return sizeof(std::uint8_t);
	case GL_BYTE:           return sizeof(std::int8_t);
	case GL_SHORT:          return sizeof(std::int16_t);
	case GL_UNSIGNED_SHORT: return sizeof(std::uint16_t);
	default:
		assert(false && "Unknown GL type in GLTypeSize");
		return 0;
	}
}

struct BufferLayer {
	const char* name = nullptr;
	GLuint index = 0;
	GLint size = 3;
	GLenum type = GL_FLOAT;
	GLboolean normalized = GL_FALSE;
	GLsizei stride = 0;
	std::size_t offsetBytes = 0;
};

struct LayersArray {
	std::vector<BufferLayer> layers;
	std::size_t currentOffset = 0;
	GLsizei stride = 0;

	void Add(const char* name, GLuint index, GLsizei stride_,
		std::size_t offsetBytes_,
		GLint size = 3, GLenum type = GL_FLOAT, GLboolean normalized = GL_FALSE)
	{
		if (stride == 0) stride = stride_;
		assert(stride == stride_ && "All layers must have the same stride");
		assert(stride_ > 0 && "stride must be > 0");
		assert(size >= 1 && size <= 4 && "attribute size must be 1..4");
		assert(GLTypeSize(type) != 0 && "unknown attribute type");
		assert(offsetBytes_ < static_cast<std::size_t>(stride_) && "offset must be < stride");

		BufferLayer l{};
		l.name = name;
		l.index = index;
		l.size = size;
		l.type = type;
		l.normalized = normalized;
		l.stride = stride;
		l.offsetBytes = offsetBytes_;

		layers.push_back(l);
	}
};

class MOX_API VAO
{
private:
	GLuint m_VAO = 0;

public:
	VAO() = default;
	~VAO() 
	{
		Destroy();
	}
	VAO(const VAO&) = delete;
	VAO& operator=(const VAO&) = delete;

	VAO(VAO&& other) noexcept : m_VAO(other.m_VAO) { other.m_VAO = 0; }
	VAO& operator=(VAO&& other) noexcept {
		if (this != &other) {
			Destroy();
			m_VAO = other.m_VAO;
			other.m_VAO = 0;
		}
		return *this;
	}

	[[nodiscard]] bool BindArray() const;
	static void UnbindArray();

	void Init(const LayersArray& layersArray);
	void Destroy();
};