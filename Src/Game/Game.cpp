// Game.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <WindowsApp.h>
#include <chrono>
#include <vector>
#include <thread>   // std::this_thread::sleep_for


using namespace Desktop;

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

	// Tiempo entre el frame anterior y este (delta time)
	double GetDeltaTime() const { return deltaTime; }

private:
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = std::chrono::time_point<Clock>;

	TimePoint startTime;
	TimePoint lastFrameTime;

	double totalTime; 
	double deltaTime;
};

class IGameSystem
{
public:
	virtual ~IGameSystem() = default;

	virtual void Update(const GameTime& gameTime) = 0;
	virtual void BeginDraw() = 0;
	virtual void Draw(const GameTime& gameTime) = 0;
	virtual void EndDraw() = 0;
};


class RenderSystem : public IGameSystem
{
public:
	RenderSystem() : position(0.0) {}

	void Update(const GameTime& gameTime) override
	{
		// Mover objeto a la derecha 1 unidad por segundo
		position += 1.0 * gameTime.GetDeltaTime();
	}

	void BeginDraw() override
	{
		std::cout << "[RenderSystem] Begin drawing...\n";
	}

	void Draw(const GameTime& gameTime) override
	{
		std::cout << "[RenderSystem] Drawing object at position: "
			<< position
			<< " (time=" << gameTime.GetTotalTime() << "s)\n";
	}

	void EndDraw() override
	{
		std::cout << "[RenderSystem] End drawing.\n\n";
	}

private:
	double position; // posición simulada de un objeto
};


int main()
{
	//WindowsApp app(1280, 720, L"Game Engine [Vultaik] {DirectX 12}!");
	//if (!app.Initialize(GetModuleHandle(nullptr)))
	//{
	//	
	//	std::cerr << "Failed to initialize the application.\n";
	//	return -1;
	//}
	//app.SetOnUpdate([]() 
	//{
	//	// Update logic here
	//});
	//app.SetOnRender([]() 
	//{
	//	// Render logic here
	//});
	//app.SetOnResize([](UINT width, UINT height) 
	//{
	//	// Handle window resize
	//	std::cout << "Window resized to: " << width << "x" << height << "\n";
	//});

	//int result = app.Run();

	//if (result < 0)
	//{
	//	std::cerr << "Application exited with error code: " << result << "\n";
	//}


	GameTime gameTime;

	// Lista de sistemas
	std::vector<std::unique_ptr<IGameSystem>> systems;
	systems.push_back(std::make_unique<RenderSystem>());

	while (gameTime.GetTotalTime() < 5.0) // corre el loop 5 segundos
	{
		gameTime.Update();

		// Update
		for (auto& sys : systems)
			sys->Update(gameTime);

		// Draw
		for (auto& sys : systems)
		{
			sys->BeginDraw();
			sys->Draw(gameTime);
			sys->EndDraw();
		}

		// Simular 60 FPS (16 ms)
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}


	return 1;
}
