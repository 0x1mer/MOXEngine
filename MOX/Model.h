#pragma once

#include "Mesh.h"
#include "Transform.h"
#include "Shader.h"

#include "MOX_API.h"
#include <vector>

class MOX_API Model
{
	struct GPURenderable
	{
		const MeshGPU* mesh = nullptr;
		Transform transform;
		Shader* shader = nullptr;
	};

	std::vector<GPURenderable> m_renderables;

public:
	Model() = default;
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;

	void AddMesh(const MeshGPU& mesh, const Transform& transform, Shader* shader)
	{
		if (!mesh.IsValid()) {
			assert(false && "Attempting to add invalid mesh to model");
		}
		if (shader == nullptr) {
			assert(false && "Attempting to add mesh with null shader to model");
		}
		m_renderables.push_back({ &mesh, transform, shader });
	}

	const std::vector<GPURenderable>& GetRenderables() const { return m_renderables; }
};