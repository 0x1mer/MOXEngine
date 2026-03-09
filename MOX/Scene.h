#pragma once

#include <unordered_map>
#include <cstdint>

#include "Model.h"
#include "MOX_API.h"

class Scene
{
public:
    using nodeId = uint64_t;

private:
    std::unordered_map<nodeId, Model> m_models;
    nodeId m_nextId = 0;

public:
    Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;

    nodeId AddModel(Model&& model)
    {
        nodeId id = m_nextId++;
        m_models.emplace(id, std::move(model));
        return id;
    }

    void RemoveModel(nodeId id)
    {
        m_models.erase(id);
    }

    Model* GetModel(nodeId id)
    {
        auto it = m_models.find(id);
        if (it == m_models.end())
            return nullptr;

        return &it->second;
    }

    const std::unordered_map<nodeId, Model>& GetModels() const
    {
        return m_models;
    }
};