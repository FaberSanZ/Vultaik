#pragma once

#include <chrono>
#include <cstdint>
#include <algorithm>


#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

class GameTime final
{
public:
    using Clock = std::chrono::steady_clock;

    struct Config
    {
        double fixedDeltaTime = 1.0 / 60.0;
        double maxDeltaTime = 0.25;
        uint32_t maxPhysicsStepsPerFrame = 8;
        double timeScale = 1.0;
        bool clearPhysicsAccumulatorOnReset = true;
    };

public:
    GameTime()
    {
        Reset();
    }

    explicit GameTime(const Config& config)
        : m_config(config)
    {
        SanitizeConfig();
        Reset();
    }

    void Reset()
    {
        const auto now = Clock::now();

        m_lastTime = now;

        m_deltaTime = 0.0;
        m_unscaledDeltaTime = 0.0;

        m_totalTime = 0.0;
        m_unscaledTotalTime = 0.0;

        m_fixedTime = 0.0;
        m_accumulator = 0.0;

        m_frameCount = 0;
        m_physicsTickCount = 0;
        m_physicsStepsThisFrame = 0;

        m_fps = 0.0;
        m_fpsTimer = 0.0;
        m_fpsCounter = 0;

        m_wasClampedThisFrame = false;
        m_hitPhysicsStepLimitThisFrame = false;
    }


    void ResetFrameTime()
    {
        m_lastTime = Clock::now();

        m_deltaTime = 0.0;
        m_unscaledDeltaTime = 0.0;

        m_physicsStepsThisFrame = 0;
        m_wasClampedThisFrame = false;
        m_hitPhysicsStepLimitThisFrame = false;

        if (m_config.clearPhysicsAccumulatorOnReset)
        {
            m_accumulator = 0.0;
        }
    }

    void OnUpdate()
    {
        const auto now = Clock::now();

        double frameTime = std::chrono::duration<double>(now - m_lastTime).count();
        m_lastTime = now;

        m_wasClampedThisFrame = false;
        m_hitPhysicsStepLimitThisFrame = false;

        if (frameTime < 0.0)
        {
            frameTime = 0.0;
        }

        m_unscaledDeltaTime = frameTime;

        if (m_config.maxDeltaTime > 0.0 && frameTime > m_config.maxDeltaTime)
        {
            frameTime = m_config.maxDeltaTime;
            m_wasClampedThisFrame = true;
        }

        if (m_paused)
        {
            m_deltaTime = 0.0;
        }
        else
        {
            m_deltaTime = frameTime * m_config.timeScale;
        }

        m_unscaledTotalTime += m_unscaledDeltaTime;
        m_totalTime += m_deltaTime;

        if (!m_paused)
        {
            m_accumulator += m_deltaTime;
        }

        m_physicsStepsThisFrame = 0;

        ++m_frameCount;

        UpdateFPS(m_unscaledDeltaTime);
    }

    bool UpdatePhysics()
    {
        if (m_paused)
            return false;

        const double fixedDt = m_config.fixedDeltaTime;

        if (fixedDt <= 0.0)
            return false;

        if (m_accumulator < fixedDt)
            return false;

        if (m_config.maxPhysicsStepsPerFrame > 0 &&
            m_physicsStepsThisFrame >= m_config.maxPhysicsStepsPerFrame)
        {
            m_hitPhysicsStepLimitThisFrame = true;

            m_accumulator = 0.0;

            return false;
        }

        m_accumulator -= fixedDt;

        m_fixedTime += fixedDt;

        ++m_physicsTickCount;
        ++m_physicsStepsThisFrame;

        return true;
    }

    bool UpdatePhycs()
    {
        return UpdatePhysics();
    }

    double InterpolationAlpha() const
    {
        const double fixedDt = m_config.fixedDeltaTime;

        if (fixedDt <= 0.0)
            return 0.0;

        return std::clamp(m_accumulator / fixedDt, 0.0, 1.0);
    }

public:


    double DeltaTime() const
    {
        return m_deltaTime;
    }

    double UnscaledDeltaTime() const
    {
        return m_unscaledDeltaTime;
    }

    double FixedDeltaTime() const
    {
        return m_config.fixedDeltaTime;
    }

    double TotalTime() const
    {
        return m_totalTime;
    }

    double UnscaledTotalTime() const
    {
        return m_unscaledTotalTime;
    }

    double FixedTime() const
    {
        return m_fixedTime;
    }

    double FPS() const
    {
        return m_fps;
    }

    uint64_t FrameCount() const
    {
        return m_frameCount;
    }

    uint64_t PhysicsTickCount() const
    {
        return m_physicsTickCount;
    }

    uint32_t PhysicsStepsThisFrame() const
    {
        return m_physicsStepsThisFrame;
    }

    bool IsPaused() const
    {
        return m_paused;
    }

    bool WasClampedThisFrame() const
    {
        return m_wasClampedThisFrame;
    }

    bool HitPhysicsStepLimitThisFrame() const
    {
        return m_hitPhysicsStepLimitThisFrame;
    }

    const Config& GetConfig() const
    {
        return m_config;
    }

public:


    void SetConfig(const Config& config)
    {
        m_config = config;
        SanitizeConfig();
    }

    void SetFixedDeltaTime(double fixedDeltaTime)
    {
        m_config.fixedDeltaTime = std::max(0.000001, fixedDeltaTime);
    }

    void SetPhysicsHz(double hz)
    {
        hz = std::max(1.0, hz);
        m_config.fixedDeltaTime = 1.0 / hz;
    }

    void SetMaxDeltaTime(double maxDeltaTime)
    {
        m_config.maxDeltaTime = std::max(0.0, maxDeltaTime);
    }

    void SetMaxPhysicsStepsPerFrame(uint32_t maxSteps)
    {
        m_config.maxPhysicsStepsPerFrame = maxSteps;
    }

    void SetTimeScale(double timeScale)
    {
        m_config.timeScale = std::max(0.0, timeScale);
    }

    void SetPaused(bool paused)
    {
        m_paused = paused;
    }

    void Pause()
    {
        m_paused = true;
    }

    void Resume()
    {
        m_paused = false;
        ResetFrameTime();
    }

private:
    void SanitizeConfig()
    {
        m_config.fixedDeltaTime = std::max(0.000001, m_config.fixedDeltaTime);
        m_config.maxDeltaTime = std::max(0.0, m_config.maxDeltaTime);
        m_config.timeScale = std::max(0.0, m_config.timeScale);
    }

    void UpdateFPS(double realFrameTime)
    {
        m_fpsTimer += realFrameTime;
        ++m_fpsCounter;

        if (m_fpsTimer >= 1.0)
        {
            m_fps = static_cast<double>(m_fpsCounter) / m_fpsTimer;

            m_fpsTimer = 0.0;
            m_fpsCounter = 0;
        }
    }

private:
    Config m_config{};

    Clock::time_point m_lastTime{};

    double m_deltaTime = 0.0;
    double m_unscaledDeltaTime = 0.0;

    double m_totalTime = 0.0;
    double m_unscaledTotalTime = 0.0;
    double m_fixedTime = 0.0;

    double m_accumulator = 0.0;

    double m_fps = 0.0;
    double m_fpsTimer = 0.0;

    uint64_t m_frameCount = 0;
    uint64_t m_physicsTickCount = 0;

    uint32_t m_fpsCounter = 0;
    uint32_t m_physicsStepsThisFrame = 0;

    bool m_paused = false;
    bool m_wasClampedThisFrame = false;
    bool m_hitPhysicsStepLimitThisFrame = false;
};