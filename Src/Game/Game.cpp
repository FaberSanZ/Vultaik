// Game.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <WindowsApp.h>
#include <chrono>
#include <vector>
#include <thread>   // std::this_thread::sleep_for
#include <future>
#include <unordered_map>
#include <typeindex>

#include <windows.h>
#include <string>
#include <d3d11.h>
#include <wrl/client.h>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")



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


class ServiceProvider
{
public:
	template <typename T>
	void AddService(std::shared_ptr<T> service)
	{
		services[typeid(T)] = service;
	}

	template <typename T>
	std::shared_ptr<T> GetService()
	{
		auto it = services.find(typeid(T));
		if (it != services.end())
			return std::static_pointer_cast<T>(it->second);
		return nullptr;
	}

	template <typename T>
	std::shared_ptr<T> GetRequiredService()
	{
		auto service = GetService<T>();
		if (!service)
			throw std::runtime_error("Service not registered!");
		return service;
	}

private:
	std::unordered_map<std::type_index, std::shared_ptr<void>> services;
};

class IContentManager
{
public:
	virtual ~IContentManager() = default;
	virtual void Load(const std::string& assetName) = 0;
};


class IGame : public IGameSystem
{
public:
	virtual ~IGame() = default;

	virtual ServiceProvider* GetServices() = 0;
	virtual IContentManager* GetContent() = 0;
	virtual std::vector<std::unique_ptr<IGameSystem>>& GetGameSystems() = 0;
	virtual const GameTime& GetTime() const = 0;
	virtual bool IsRunning() const = 0;

	virtual void Run() = 0;
	virtual void Exit() = 0;
	virtual void Tick() = 0;
	virtual void Initialize() = 0;

	virtual std::future<void> LoadContentAsync() = 0;

	virtual void BeginRun() = 0;
	virtual void EndRun() = 0;
};

class GameWindow
{
public:
	GameWindow(int width = 1280, int height = 720, const std::wstring& title = L"My Game Window")
		: width(width), height(height), title(title), hWnd(nullptr)
	{
	}

	bool Initialize()
	{
		WNDCLASS wc = {};
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.lpszClassName = L"MyGameWindowClass";
		RegisterClass(&wc);

		hWnd = CreateWindowEx(
			0,
			wc.lpszClassName,
			title.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height,
			nullptr, nullptr, wc.hInstance, nullptr);

		if (!hWnd) return false;

		ShowWindow(hWnd, SW_SHOW);
		return true;
	}

	// Nuevo: solo procesa mensajes UNA VEZ por frame
	void PumpMessages()
	{
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				PostQuitMessage(0);
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Exit()
	{
		PostQuitMessage(0);
	}

	HWND GetHWND() const { return hWnd; }
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

private:
	HWND hWnd;
	int width, height;
	std::wstring title;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_DESTROY)
		{
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
};


class GameBase : public IGame
{
public:
	GameBase(std::shared_ptr<ServiceProvider> services)
		: services(services), running(false)
	{
	}

	virtual ~GameBase() = default;

	ServiceProvider* GetServices() override { return services.get(); }
	IContentManager* GetContent() override { return nullptr; } // simplificado
	std::vector<std::unique_ptr<IGameSystem>>& GetGameSystems() override { return gameSystems; }
	const GameTime& GetTime() const override { return gameTime; }
	bool IsRunning() const override { return running; }

	virtual void Run() override;
	virtual void Exit() override { running = false; }
	virtual void Tick() override;
	virtual void Initialize() override {}
	virtual std::future<void> LoadContentAsync() override { return std::async([] {}); }
	virtual void BeginRun() override {}
	virtual void EndRun() override {}

	// --- De IGameSystem
	void Update(const GameTime& gameTime) override;
	void BeginDraw() override;
	void Draw(const GameTime& gameTime) override;
	void EndDraw() override;

protected:
	std::shared_ptr<ServiceProvider> services;
	GameTime gameTime;
	std::vector<std::unique_ptr<IGameSystem>> gameSystems;
	bool running;
};


void GameBase::Run()
{
	Initialize();
	BeginRun();

	running = true;

	while (running)
	{
		if (auto window = services->GetService<GameWindow>())
			window->PumpMessages();

		gameTime.Update();
		Tick();

		std::this_thread::sleep_for(std::chrono::milliseconds(16)); // 60fps
	}

	EndRun();
}

void GameBase::Tick()
{
	Update(gameTime);

	BeginDraw();
	Draw(gameTime);
	EndDraw();
}

void GameBase::Update(const GameTime& gt)
{
	for (auto& sys : gameSystems)
		sys->Update(gt);
}

void GameBase::BeginDraw()
{
	for (auto& sys : gameSystems)
		sys->BeginDraw();
}

void GameBase::Draw(const GameTime& gt)
{
	for (auto& sys : gameSystems)
		sys->Draw(gt);
}

void GameBase::EndDraw()
{
	for (auto& sys : gameSystems)
		sys->EndDraw();
}

// dummies de los sistemas
class GraphicsDevice
{
public:
	bool Initialize(HWND hwnd, int width, int height)
	{
		DXGI_SWAP_CHAIN_DESC scd = {};
		scd.BufferCount = 1;
		scd.BufferDesc.Width = width;
		scd.BufferDesc.Height = height;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = hwnd;
		scd.SampleDesc.Count = 1;
		scd.Windowed = TRUE;

		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&scd,
			&swapChain,
			&device,
			nullptr,
			&context
		);

		if (FAILED(hr)) return false;

		// Crear RTV del backbuffer
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv);

		return true;
	}

	void Clear(float r, float g, float b, float a)
	{
		float color[4] = { r, g, b, a };
		context->ClearRenderTargetView(rtv.Get(), color);
	}

	void Present()
	{
		swapChain->Present(1, 0);
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
};

class InputManager : public IGameSystem
{
public:
	void Update(const GameTime&) override {  }
	void BeginDraw() override {}
	void Draw(const GameTime&) override {}
	void EndDraw() override {}
};

class SceneSystem : public IGameSystem
{
public:
	void Update(const GameTime&) override {  }
	void BeginDraw() override {}
	void Draw(const GameTime&) override {}
	void EndDraw() override {}
};


class RenderSystem : public IGameSystem
{
public:
	RenderSystem(std::shared_ptr<GraphicsDevice> gfx) : gfx(gfx) {}

	void Update(const GameTime&) override {}

	void BeginDraw() override
	{
		gfx->Clear(0.1f, 0.2f, 0.4f, 1.0f); // azulito
	}


	void BeginDraw2()
	{
		gfx->Clear(1.1f, 1.2f, 1.4f, 1.0f); // azulito
	}



	void Draw(const GameTime&) override
	{
	}

	void EndDraw() override
	{
		gfx->Present();
	}

public:
	std::shared_ptr<GraphicsDevice> gfx;
};


class AsyncScript
{
public:
	AsyncScript(ServiceProvider* services = nullptr)
		: services(services)
	{
	}

	virtual ~AsyncScript() = default;

	virtual void Initialize() {}
	virtual void Update(float deltaTime) {}

	void Run()
	{
		running = true;
		worker = std::async(std::launch::async, [this]()
			{
				Initialize();
				auto last = std::chrono::high_resolution_clock::now();

				while (running)
				{
					auto now = std::chrono::high_resolution_clock::now();
					std::chrono::duration<float> dt = now - last;
					last = now;

					Update(dt.count());
					std::this_thread::sleep_for(std::chrono::milliseconds(16));
				}
			});
	}

	void Stop() { running = false; }
	bool IsRunning() const { return running; }

protected:
	ServiceProvider* services = nullptr;

private:
	std::atomic<bool> running = false;
	std::future<void> worker;
};



class MyScript : public AsyncScript
{
public:
	using AsyncScript::AsyncScript; 

	void Initialize() override
	{
		std::cout << "MyScript inicializado!\n";
		counter = 0;
		renderSystem = services->GetService<RenderSystem>();
	}

	void Update(float deltaTime) override
	{
		counter++;
		std::cout << "Update " << counter << " dt=" << deltaTime << "s\n";

		if (counter % 2)
		{
			//Stop();

			if (renderSystem)
				renderSystem->gfx->Clear(0.8f, 0.8f, 0.8f, 1.0f); // rojo
		}
	}

private:
	int counter = 0;
	std::shared_ptr<RenderSystem> renderSystem;
};


class ScriptSystem : public IGameSystem
{
public:
	void AddScript(std::shared_ptr<AsyncScript> script)
	{
		scripts.push_back(script);
		script->Run(); // corre el script en un hilo aparte
	}

	void Update(const GameTime&) override
	{
		scripts.erase(
			std::remove_if(scripts.begin(), scripts.end(),
				[](auto& s) { return !s->IsRunning(); }),
			scripts.end()
		);

	}

	void BeginDraw() override {}
	void Draw(const GameTime&) override {}
	void EndDraw() override {}

	void Shutdown()
	{
		for (auto& s : scripts)
			s->Stop();
		scripts.clear();
	}

private:
	std::vector<std::shared_ptr<AsyncScript>> scripts;
};



class Game : public GameBase
{
public:
	Game(std::shared_ptr<ServiceProvider> context) : GameBase(context)
	{
		// Recuperar servicios del "ServiceProvider"
		graphicsDevice = services->GetService<GraphicsDevice>();
		window = services->GetService<GameWindow>();
		input = services->GetRequiredService<InputManager>();
		script = services->GetRequiredService<ScriptSystem>();
		sceneSystem = services->GetRequiredService<SceneSystem>();

		// Agregar sistemas al motor (composición del juego)
		if (input)       gameSystems.push_back(std::make_unique<InputManager>(*input));
		if (script)      gameSystems.push_back(std::make_unique<ScriptSystem>(*script));
		if (sceneSystem) gameSystems.push_back(std::make_unique<SceneSystem>(*sceneSystem));
		if (graphicsDevice) gameSystems.push_back(std::make_unique<RenderSystem>(graphicsDevice));
	}

	// Inicializa el juego
	void Initialize() override
	{
		std::cout << "[Game] Initialize\n";
		GameBase::Initialize();


		auto scriptSystem = services->GetRequiredService<ScriptSystem>();



		scriptSystem->AddScript(std::make_shared<MyScript>(services.get()));

	}


	void BeginRun() override
	{
		std::cout << "[Game] BeginRun\n";
		if (window && window->Initialize())
		{
			// Inicializar DirectX
			if (graphicsDevice)
			{
				if (!graphicsDevice->Initialize(window->GetHWND(), window->GetWidth(), window->GetHeight()))
				{
					std::cerr << "[Game] Error: GraphicsDevice no se pudo inicializar.\n";
					return;
				}
			}
		}
	}

	// Comienza el frame de render
	void BeginDraw() override
	{
		GameBase::BeginDraw(); // aquí RenderSystem hará el Clear()
	}

	// Termina el frame de render
	void EndDraw() override
	{
		GameBase::EndDraw(); // RenderSystem ya dibujó
	}

	// Termina el loop principal
	void EndRun() override
	{
		if (window) window->Exit();
	}

private:
	std::shared_ptr<GraphicsDevice> graphicsDevice;
	std::shared_ptr<GameWindow> window;
	std::shared_ptr<InputManager> input;
	std::shared_ptr<ScriptSystem> script;
	std::shared_ptr<SceneSystem> sceneSystem;
};



int main()
{


	auto services = std::make_shared<ServiceProvider>();
	services->AddService(std::make_shared<GraphicsDevice>());
	services->AddService(std::make_shared<RenderSystem>());
	services->AddService(std::make_shared<GameWindow>());
	services->AddService(std::make_shared<InputManager>());
	services->AddService(std::make_shared<ScriptSystem>());
	services->AddService(std::make_shared<SceneSystem>());

	Game game(services);
	game.Run();

	return 0;

	return 0;
}
