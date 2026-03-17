#include "pch.h"
#include "Engine.h"
#include "Logger.h"

#include <iostream>

int Engine::Init()
{
    InitPaths();
    LOGGER_SET_FOLDER(paths::logsDir);

	int result = m_renderer.Init();
	if (result != 0) {
		LOG_ERROR("Renderer initialization failed with code: " + std::to_string(result));
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
		LOG_ERROR("Engine::Frame() called before successful initialization");
        return;
    }

    if (m_renderer.ShouldClose()) {
		LOG_CRITICAL("Renderer requested to close. Stopping the engine.");
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