#include "pch.h"
#include "VAO.h"

#include <glad/glad.h>

bool VAO::BindArray() const
{
	if (m_VAO != 0) {
		glBindVertexArray(m_VAO);
		return true;
	}
	return false;
}

void VAO::UnbindArray()
{
	glBindVertexArray(0);
}

void VAO::Init(const LayersArray& layersArray)
{
	Destroy();
#ifdef _DEBUG
	GLint bound = 0;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &bound);
	assert(bound != 0 && "GL_ARRAY_BUFFER must be bound before VAO::Init()");
#endif
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	for (const auto& layer : layersArray.layers) {
		const bool isIntegral =
			layer.type == GL_INT || layer.type == GL_UNSIGNED_INT ||
			layer.type == GL_BYTE || layer.type == GL_UNSIGNED_BYTE ||
			layer.type == GL_SHORT || layer.type == GL_UNSIGNED_SHORT;

		const void* ptr = reinterpret_cast<const void*>(static_cast<uintptr_t>(layer.offsetBytes));

		if (isIntegral && layer.normalized == GL_FALSE) {
			glVertexAttribIPointer(layer.index, layer.size, layer.type, layer.stride, ptr);
		}
		else {
			glVertexAttribPointer(layer.index, layer.size, layer.type, layer.normalized, layer.stride, ptr);
		}
		glEnableVertexAttribArray(layer.index);
	}

	UnbindArray();
}

void VAO::Destroy() {
	if (m_VAO != 0) {
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}
}