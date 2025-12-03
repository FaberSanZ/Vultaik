#pragma once
#include <chrono>


class GameTime
{
public:
	GameTime()
	{
		startTime = Clock::now();
		lastFrameTime = startTime;
		totalTime = 0.0;
		deltaTime = 0.0;
	}

	void Update()
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


//class GameTime
//{
//public:
//	GameTime() {}
//
//
//	void OnInitialize()
//	{
//		LARGE_INTEGER freq;
//		QueryPerformanceFrequency(&freq);
//		invFrequency = 1.0 / static_cast<double>(freq.QuadPart);
//		QueryPerformanceCounter(&last);
//		start = last;
//	}
//
//	void Update()
//	{
//		LARGE_INTEGER now;
//		QueryPerformanceCounter(&now);
//
//		LONGLONG deltaTicks = now.QuadPart - last.QuadPart;
//		LONGLONG totalTicks = now.QuadPart - start.QuadPart;
//
//		deltaTime = deltaTicks * invFrequency;
//		totalTime = totalTicks * invFrequency;
//
//		last = now;
//	}
//
//	double GetDeltaTime() const { return deltaTime; }
//	double GetTotalTime() const { return totalTime; }
//
//private:
//	LARGE_INTEGER start, last;
//	double invFrequency;
//	double deltaTime = 0.0;
//	double totalTime = 0.0;
//};
