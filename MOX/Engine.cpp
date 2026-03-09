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

	m_time.Init(m_renderer.GetTime());

	m_isInit = true;
}

void Engine::Shutdown()
{
	m_renderer.Shutdown();
}

void Engine::Frame(const Scene& scene)
{
    if (!m_isInit)
    {
        std::cout << "Engine::Frame() called before successful initialization\n";
        return;
    }

    if (m_renderer.ShouldClose()) {
		std::cout << "Stoped" << std::endl;
        return;
    }

    double currentTime = m_renderer.GetTime();

    m_time.Update(currentTime);

    m_renderer.BeginFrame(m_time.GetDeltaTime());

    m_renderer.Render(
        scene,
        m_time.GetFPS(),
        m_time.GetFrameTimeMs(),
        m_time.GetDeltaTime(),
        m_time.GetTotalTime()
    );

    m_renderer.EndFrame();
}