#include "pch.h"
#include "Engine.h"

#include <iostream>

int Engine::Init()
{
	int result = m_renderer.Init();
	if (result != 0) {
		std::cout << "Renderer initialization failed with code: " << result << "\n";
		return result;
	}

	m_isInit = true;
}

void Engine::Shutdown()
{
	m_renderer.Shutdown();
}

void Engine::Frame(const Scene& scene)
{
	if (!m_isInit) {
		std::cout << "Engine::Frame() called before successful initialization\n";
		return;
	}

	if (m_renderer.ShouldClose()) {
		std::cout << "Renderer signaled to close. Exiting main loop.\n";
		return;
	}

	const float currentFrame = m_renderer.GetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// FPS calc
	frameCount++;
	timeAccumulator += deltaTime;
	if (timeAccumulator >= 0.5f)
	{
		fps = frameCount / timeAccumulator;
		frameTimeMs = 1000.0f / fps;
		frameCount = 0;
		timeAccumulator = 0.0f;
	}
	totalTime += deltaTime;

	m_renderer.BeginFrame(deltaTime);
	m_renderer.Render(scene, fps, frameTimeMs, deltaTime, totalTime);
	m_renderer.EndFrame();
}
