#pragma once

// added for later wrote to logs when engine create and destroy
#include <chrono>

class EngineTime
{
private:
    double m_lastFrame = 0.0;
    double m_deltaTime = 0.0;
    double m_totalTime = 0.0;

    // FPS
    int m_frameCount = 0;
    double m_fps = 0.0;
    double m_frameTimeMs = 0.0;
    double m_fpsAccumulator = 0.0;

    // tick system
    double m_tickAccumulator = 0.0;

public:
    EngineTime() = default;

    void Init(double startTime)
    {
        m_lastFrame = startTime;
    }

    void Update(double currentTime)
    {
        m_deltaTime = currentTime - m_lastFrame;
        m_lastFrame = currentTime;

        m_totalTime += m_deltaTime;

        // FPS calc
        m_frameCount++;
        m_fpsAccumulator += m_deltaTime;

        if (m_fpsAccumulator >= 0.5)
        {
            m_fps = m_frameCount / m_fpsAccumulator;
            m_frameTimeMs = 1000.0 / m_fps;

            m_frameCount = 0;
            m_fpsAccumulator = 0.0;
        }

        m_tickAccumulator += m_deltaTime;
    }

    bool ShouldTick(double tickTime)
    {
        if (m_tickAccumulator >= tickTime)
        {
            m_tickAccumulator -= tickTime;
            return true;
        }

        return false;
    }

    double GetDeltaTime() const { return m_deltaTime; }
    double GetTotalTime() const { return m_totalTime; }

    double GetFPS() const { return m_fps; }
    double GetFrameTimeMs() const { return m_frameTimeMs; }
};