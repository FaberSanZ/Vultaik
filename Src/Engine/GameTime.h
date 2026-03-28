#pragma once
#include <chrono>


class GameTime
{
public:
	GameTime()
	{

	}

	void OnInitialize()
	{
		startTime = Clock::now();
		lastFrameTime = startTime;
		totalTime = 0.0;
		deltaTime = 0.0;
	}

	void OnUpdate()
	{
		auto currentTime = Clock::now();

		std::chrono::duration<double> delta = currentTime - lastFrameTime;
		deltaTime = delta.count();

		std::chrono::duration<double> total = currentTime - startTime;
		totalTime = total.count();

		lastFrameTime = currentTime;
	}


	double GetTotalTime() const { return totalTime; }
	double GetDeltaTime() const { return deltaTime; }

private:
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = std::chrono::time_point<Clock>;

	TimePoint startTime;
	TimePoint lastFrameTime;

	double totalTime;
	double deltaTime;
};
