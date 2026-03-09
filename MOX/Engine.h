#pragma once

#include "MOX_API.h"

#include "Paths.h"
#include "Renderer.h"
#include "EngineTime.h"

class MOX_API Engine
{
private:
	Renderer m_renderer;
	EngineTime m_time;

	bool m_isInit = false;

public:
	Engine() = default;
	~Engine() {
		Shutdown();
	};

	double GetTime() const { return m_renderer.GetTime(); }

	// TODO: remove this when it ill be possible
	Renderer& GetRenderer() { return m_renderer; }

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(Engine&&) = delete;

	bool ShouldClose() const { return m_renderer.ShouldClose(); }

	Camera& GetCamera() { return m_renderer.GetCamera(); }
	bool ShouldTick(double tickTime) { return m_time.ShouldTick(tickTime); }

	int Init();
	void Shutdown();
	void Frame(const Scene& scene);
};