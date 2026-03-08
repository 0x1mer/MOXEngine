#pragma once

#include <vector>
#include "Shader.h"

#include "MOX_API.h"

class MOX_API ShaderManager
{
private:
	std::vector<Shader> m_shaders;

public:
	ShaderManager() = default;

	ShaderManager(const ShaderManager&) = delete;
	ShaderManager& operator=(const ShaderManager&) = delete;
	ShaderManager(ShaderManager&&) = delete;
	ShaderManager& operator=(ShaderManager&&) = delete;

	void LoadShader(const char* vertexPath, const char* fragmentPath)
	{
		m_shaders.emplace_back(vertexPath, fragmentPath);
		std::cout << "Loaded shader: " << vertexPath << " + " << fragmentPath << "\n";
	}

	Shader& GetShader(size_t index) {
		if (index >= m_shaders.size()) {
			throw std::out_of_range("Shader index out of range");
		}
		return m_shaders[index];
	}

	const Shader& GetShaderConst(size_t index) const
	{
		if (index >= m_shaders.size()) {
			throw std::out_of_range("Shader index out of range");
		}
		return m_shaders[index];
	}
};