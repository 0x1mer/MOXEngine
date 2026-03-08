#pragma once

#include "MOX_API.h"

#include "Renderer.h"

class MOX_API Engine
{
private:
	Renderer m_renderer;

	bool m_isInit = false;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	float fps = 0.0f;
	float frameTimeMs = 0.0f;

	int frameCount = 0;
	float timeAccumulator = 0.0f;

	float totalTime = 0.0f;

public:
	Engine() = default;
	~Engine() {
		Shutdown();
	};

	 Engine(const Engine&) = delete;
	 Engine& operator=(const Engine&) = delete;
	 Engine(Engine&&) = delete;
	 Engine& operator=(Engine&&) = delete;

	 bool ShouldClose() const { return m_renderer.ShouldClose(); }

	 int Init();
	 void Shutdown();
	 void Frame(const Scene& scene);
};