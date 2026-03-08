#pragma once

#include <vector>

#include "Model.h"
#include "MOX_API.h"

class Scene
{
private:
	std::vector<Model> m_models;

public:
	Scene() = default;

	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	Scene(Scene&&) noexcept = default;
	Scene& operator=(Scene&&) noexcept = default;

	void AddModel(Model&& model)
	{
		m_models.push_back(std::move(model));
	}

	const std::vector<Model>& GetModels() const { return m_models; }
};