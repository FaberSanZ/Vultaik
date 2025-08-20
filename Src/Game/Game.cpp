// Game.cpp : This file contains the 'main' function. Program execution begins and ends there.

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
#include <d3dcompiler.h>
#include <wrl/client.h>

#define GAMEINPUT_API_VERSION 2
#include <GameInput.h>


#include <unordered_set>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <any>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "GameInput.lib")
#pragma comment(lib, "d3dcompiler.lib")



using namespace Desktop;
using namespace Microsoft::WRL;
using namespace GameInput::v2;
using Microsoft::WRL::ComPtr;

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
	virtual std::vector<std::shared_ptr<IGameSystem>>& GetGameSystems() = 0;
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
	IContentManager* GetContent() override { return nullptr; }
	std::vector<std::shared_ptr<IGameSystem>>& GetGameSystems() override { return gameSystems; } // QUITADO el = 0
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
	std::vector<std::shared_ptr<IGameSystem>> gameSystems;
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

		//std::this_thread::sleep_for(std::chrono::milliseconds(16)); // 60fps
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

struct int2 { int x, y; };

class Sprite
{
public:
	Sprite() = default;

	Sprite(int x, int y, int w, int h, int texX = 0, int texY = 0, float s = 0)
		: screenPos { x,y }, size { w,h }, atlasPos { texX, texY }, scale(s)
	{
	}

	void SetAtlas(int texX, int texY)
	{
		atlasPos = { texX, texY };
	}

	void SetScreenPos(int x, int y)
	{
		screenPos = { x, y };
	}

	int2 screenPos;
	int2 size;
	int2 atlasPos;
	float scale = 1.0f;
};

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

		ID3D11Texture2D* framebufferTexture;
		swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&framebufferTexture));
		device->CreateRenderTargetView(framebufferTexture, nullptr, &framebufferRTV);



		int texWidth, texHeight, texChannels;
		unsigned char* pixels = stbi_load("../../../../Assets/Textures/rollingBall_sheet.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			//return;
		}
		std::cout << "Texture loaded: " << texWidth << "x" << texHeight << " with " << texChannels << " channels." << std::endl;

		float constantData[4] = { 2.0f / width, -2.0f / height, 1.0f / texWidth, 1.0f / texHeight }; // one-time calc here to make it simpler for the shader later (float2 rn_screensize, r_atlassize)


		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.ByteWidth = sizeof(constantData) + 0xf & 0xfffffff0; // ensure constant buffer size is multiple of 16 bytes
		constantBufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // constant buffer doesn't need updating in this example
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		D3D11_SUBRESOURCE_DATA constantSRD = { constantData };


		device->CreateBuffer(&constantBufferDesc, &constantSRD, &constantBuffer);





		D3D11_TEXTURE2D_DESC atlasTextureDesc = {};
		atlasTextureDesc.Width = texWidth;
		atlasTextureDesc.Height = texHeight;
		atlasTextureDesc.MipLevels = 1;
		atlasTextureDesc.ArraySize = 1;
		atlasTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		atlasTextureDesc.SampleDesc.Count = 1;
		atlasTextureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		atlasTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = pixels;
		initData.SysMemPitch = texWidth * 4; // 4 bytes (RGBA)

		device->CreateTexture2D(&atlasTextureDesc, &initData, &atlasTexture);
		device->CreateShaderResourceView(atlasTexture, nullptr, &atlasSRV);




		D3D11_SAMPLER_DESC pointsamplerDesc = {};
		pointsamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		pointsamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		pointsamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		pointsamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		pointsamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

		device->CreateSamplerState(&pointsamplerDesc, &sampler);



		ID3DBlob* vsBlob;
		D3DCompileFromFile(L"../../../../Assets/Shaders/PixelShader.hlsl", nullptr, nullptr, "vs", "vs_5_0", 0, 0, &vsBlob, nullptr);
		device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);


		ID3DBlob* psBlob;
		D3DCompileFromFile(L"../../../../Assets/Shaders/PixelShader.hlsl", nullptr, nullptr, "ps", "ps_5_0", 0, 0, &psBlob, nullptr);
		device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);


		spriteBatch = reinterpret_cast<Sprite*>(HeapAlloc(GetProcessHeap(), 0, 4096 * sizeof(Sprite)));



		D3D11_BUFFER_DESC spriteBufferDesc = {};
		spriteBufferDesc.ByteWidth = 4096 * sizeof(Sprite);
		spriteBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		spriteBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		spriteBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		spriteBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		spriteBufferDesc.StructureByteStride = sizeof(Sprite);

		device->CreateBuffer(&spriteBufferDesc, nullptr, &spriteBuffer);



		device->CreateBuffer(&spriteBufferDesc, nullptr, &spriteBuffer);

		D3D11_SHADER_RESOURCE_VIEW_DESC spriteSRVDesc = {};
		spriteSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		spriteSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		spriteSRVDesc.Buffer.NumElements = 4096;


		device->CreateShaderResourceView(spriteBuffer, &spriteSRVDesc, &textureAtlas);
		viewport = { 0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1 };



		return true;
	}

	ID3D11Buffer* constantBuffer;
	ID3D11Texture2D* atlasTexture;
	ID3D11ShaderResourceView* atlasSRV;
	ID3D11SamplerState* sampler;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11Buffer* spriteBuffer;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	ID3D11ShaderResourceView* textureAtlas;
	D3D11_VIEWPORT viewport;

	Sprite* spriteBatch;

	void SetSprites(Sprite* sprites, uint16_t count)
	{
		spriteCount = count;
		D3D11_MAPPED_SUBRESOURCE spriteBufferMSR;
		context->Map(spriteBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &spriteBufferMSR);
		memcpy(spriteBufferMSR.pData, sprites, count * sizeof(Sprite));
		context->Unmap(spriteBuffer, 0);
	}

	void Clear(float r, float g, float b, float a)
	{


		context->OMSetRenderTargets(1, &framebufferRTV, nullptr);


		float color[4] = { r, g, b, a };
		context->ClearRenderTargetView(framebufferRTV, color);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); // to render sprite quad using 4 vertices

		context->VSSetShader(vertexShader, nullptr, 0);
		context->VSSetShaderResources(0, 1, &textureAtlas);
		context->VSSetConstantBuffers(0, 1, &constantBuffer);

		context->RSSetViewports(1, &viewport);

		context->PSSetShader(pixelShader, nullptr, 0);
		context->PSSetShaderResources(1, 1, &atlasSRV);
		context->PSSetSamplers(0, 1, &sampler);


		context->DrawInstanced(4, spriteCount, 0, 0);
	}

	void Present()
	{
		swapChain->Present(1, 0);
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	ID3D11RenderTargetView* framebufferRTV;
	uint16_t spriteCount = 5;

};

class InputManager : public IGameSystem
{
public:

	void Initialize(HWND hwnd)
	{
		this->hwnd = hwnd;
	
	}

	void Update(const GameTime&) override
	{
		std::lock_guard<std::mutex> lock(mutex);

		// --- Teclado ---
		previousKeys = currentKeys;
		currentKeys.clear();

		for (int vk = 0; vk < 256; ++vk)
		{
			if (GetAsyncKeyState(vk) & 0x8000)
				currentKeys.insert(static_cast<uint8_t>(vk));
		}

		// --- Mouse ---
		POINT p;
		if (GetCursorPos(&p))
		{
			ScreenToClient(hwnd, &p); // coordenadas relativas a la ventana
			mouseX = p.x;
			mouseY = p.y;
		}

		mouseButtons = 0;
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) mouseButtons |= 1;
		if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) mouseButtons |= 2;
		if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) mouseButtons |= 4;
	}

	void BeginDraw() override {}
	void Draw(const GameTime&) override {}
	void EndDraw() override {}

	// --- API ---
	bool IsKeyDown(uint8_t vk) const
	{
		std::lock_guard<std::mutex> lock(mutex);
		return currentKeys.find(vk) != currentKeys.end();
	}

	bool IsKeyPressed(uint8_t vk) const
	{
		std::lock_guard<std::mutex> lock(mutex);
		return currentKeys.find(vk) != currentKeys.end() &&
			previousKeys.find(vk) == previousKeys.end();
	}

	bool IsMouseButtonDown(uint32_t button) const
	{
		std::lock_guard<std::mutex> lock(mutex);
		return (mouseButtons & button) != 0;
	}

	int GetMouseX() const { std::lock_guard<std::mutex> lock(mutex); return mouseX; }
	int GetMouseY() const { std::lock_guard<std::mutex> lock(mutex); return mouseY; }

private:
	HWND hwnd;
	mutable std::mutex mutex;
	std::unordered_set<uint8_t> currentKeys;
	std::unordered_set<uint8_t> previousKeys;
	int mouseX = 0, mouseY = 0;
	uint32_t mouseButtons = 0;
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

class SyncScript
{
public:
	SyncScript(ServiceProvider* services = nullptr)
		: services(services)
	{
	}

	virtual ~SyncScript() = default;

	virtual void Initialize() {}
	virtual void Update(const GameTime& gameTime) {}

protected:
	ServiceProvider* services = nullptr;
};

class SyncScriptSystem : public IGameSystem
{
public:
	void AddScript(std::shared_ptr<SyncScript> script)
	{
		scripts.push_back(script);
		script->Initialize();
	}

	void Update(const GameTime& gameTime) override
	{
		for (auto& s : scripts)
			s->Update(gameTime);
	}

	void BeginDraw() override {}
	void Draw(const GameTime&) override {}
	void EndDraw() override {}

private:
	std::vector<std::shared_ptr<SyncScript>> scripts;
};

struct IComponent
{
	virtual ~IComponent() = default;
};



struct TransformComponent : public IComponent
{
	int2 position;
	TransformComponent() = default;
	TransformComponent(const int2& pos) : position(pos) {}
};

struct SpriteComponent : public IComponent
{
	Sprite value;

	SpriteComponent() = default;
	SpriteComponent(const Sprite& s) : value(s) {}
};


using Entity = uint32_t;
constexpr Entity INVALID_ENTITY = 0;


class ComponentManager
{
public:
	template<typename T>
	void Add(Entity e, T component)
	{
		auto& arr = GetComponentArray<T>();
		arr[e] = component;
	}

	template<typename T>
	T* Get(Entity e)
	{
		auto& arr = GetComponentArray<T>();
		auto it = arr.find(e);
		return it != arr.end() ? &it->second : nullptr;
	}

	template<typename T>
	std::unordered_map<Entity, T>& GetAll()
	{
		return GetComponentArray<T>();
	}

private:
	// Aquí va GetComponentArray
	template<typename T>
	std::unordered_map<Entity, T>& GetComponentArray()
	{
		const std::type_index idx = typeid(T);
		if (components.find(idx) == components.end())
		{
			components[idx] = std::make_any<std::unordered_map<Entity, T>>();
		}
		return std::any_cast<std::unordered_map<Entity, T>&>(components[idx]);
	}

	std::unordered_map<std::type_index, std::any> components;
};

class ECS
{
public:
	Entity CreateEntity() { return ++nextEntity; }

	template<typename T>
	void AddComponent(Entity e, T component)
	{
		cm.Add(e, component);
	}

	template<typename T>
	T* GetComponent(Entity e)
	{
		return cm.Get<T>(e);
	}

	template<typename T>
	std::unordered_map<Entity, T>& GetAllComponents()
	{
		return cm.GetAll<T>();
	}

private:
	Entity nextEntity = 0;
	ComponentManager cm;
};

class RenderSystem : public IGameSystem
{
public:
	RenderSystem(std::shared_ptr<GraphicsDevice> gfx)
		: gfx(gfx)
	{
		clearColor[0] = 0.1f;
		clearColor[1] = 0.2f;
		clearColor[2] = 0.4f;
		clearColor[3] = 1.0f;
	}

	void Update(const GameTime&) override
	{
		sprites.clear();
		if (!ecs) return;

		for (auto& [entity, spriteComp] : ecs->GetAllComponents<SpriteComponent>())
		{
			if (auto* transform = ecs->GetComponent<TransformComponent>(entity))
			{
				auto* realSprite = ecs->GetComponent<SpriteComponent>(entity);
				realSprite->value.screenPos = transform->position;
				sprites.push_back(realSprite->value);
			}
		}
	}

	void BeginDraw() override
	{
		gfx->Clear(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);


		// Copiar sprites al buffer GPU
		if (!sprites.empty())
			gfx->SetSprites(sprites.data(), static_cast<uint16_t>(sprites.size()));
	}

	void SetClearColor(float r, float g, float b, float a)
	{
		clearColor[0] = r;
		clearColor[1] = g;
		clearColor[2] = b;
		clearColor[3] = a;
	}


	void AddSprite(const Sprite& s)
	{
		sprites.push_back(s);
	}

	void SetECS(std::shared_ptr<ECS> ecs_) { ecs = ecs_; }


	void Draw(const GameTime&) override {}
	void EndDraw() override { gfx->Present(); }

public:
	std::shared_ptr<GraphicsDevice> gfx;
	std::vector<Sprite> sprites;
	std::shared_ptr<ECS> ecs;


	float clearColor[4];
};

class SceneSystem : public IGameSystem
{
public:
	SceneSystem(std::shared_ptr<ECS> ecs) : ecs(ecs) {}

	Entity CreateEntity()
	{
		Entity e = ecs->CreateEntity();
		entities.push_back(e);
		return e;
	}

	template<typename T>
	void AddComponent(Entity e, T component)
	{
		ecs->AddComponent<T>(e, component);
	}

	template<typename T>
	T* GetComponent(Entity e)
	{
		return ecs->GetComponent<T>(e);
	}

	void Update(const GameTime&) override {}
	void BeginDraw() override {}
	void Draw(const GameTime&) override {}
	void EndDraw() override {}

private:
	std::shared_ptr<ECS> ecs;
	std::vector<Entity> entities;
};

class MySyncScript : public SyncScript
{
public:
	using SyncScript::SyncScript;

	void Initialize() override
	{
		scene = services->GetRequiredService<SceneSystem>();
		input = services->GetRequiredService<InputManager>();

		leftPaddle = scene->CreateEntity();
		scene->AddComponent(leftPaddle, TransformComponent({ 50, 300 }));
		scene->AddComponent(leftPaddle, SpriteComponent(Sprite(50, 300, 32, 128, 0, 128, 1)));

		rightPaddle = scene->CreateEntity();
		scene->AddComponent(rightPaddle, TransformComponent({ 1180, 300 }));
		scene->AddComponent(rightPaddle, SpriteComponent(Sprite(1180, 300, 32, 128, 0, 128, 1)));

		ball = scene->CreateEntity();
		scene->AddComponent(ball, TransformComponent({ 640, 360 }));
		scene->AddComponent(ball, SpriteComponent(Sprite(640, 360, 32, 32, 64, 0, 1)));

		ballVel = { 200.0f, 150.0f };
	}

	void Update(const GameTime& gt) override
	{
		float dt = static_cast<float>(gt.GetDeltaTime());

		auto* leftT = scene->GetComponent<TransformComponent>(leftPaddle);
		auto* rightT = scene->GetComponent<TransformComponent>(rightPaddle);

		if (input->IsKeyDown('W')) leftT->position.y -= int(400 * dt);
		if (input->IsKeyDown('S')) leftT->position.y += int(400 * dt);
		if (input->IsKeyDown(VK_UP)) rightT->position.y -= int(400 * dt);
		if (input->IsKeyDown(VK_DOWN)) rightT->position.y += int(400 * dt);

		leftT->position.y = std::clamp(leftT->position.y, 0, 720 - 128);
		rightT->position.y = std::clamp(rightT->position.y, 0, 720 - 128);

		auto* ballT = scene->GetComponent<TransformComponent>(ball);
		ballT->position.x += int(ballVel.x * dt);
		ballT->position.y += int(ballVel.y * dt);

		if (ballT->position.y <= 0 || ballT->position.y >= 720 - 32) ballVel.y *= -1;

		if (CheckCollision(ballT->position, { 32,32 }, leftT->position, { 32,128 }) ||
			CheckCollision(ballT->position, { 32,32 }, rightT->position, { 32,128 }))
			ballVel.x *= -1;

		if (ballT->position.x <= 0 || ballT->position.x >= 1280 - 32)
		{
			ballT->position = { 640,360 };
			ballVel = { 200.0f * ((ballVel.x < 0) ? 1 : -1),150.0f };
		}
	}

private:
	std::shared_ptr<SceneSystem> scene;
	std::shared_ptr<InputManager> input;

	Entity leftPaddle = INVALID_ENTITY;
	Entity rightPaddle = INVALID_ENTITY;
	Entity ball = INVALID_ENTITY;

	struct { float x, y; } ballVel;

	bool CheckCollision(int2 posA, int2 sizeA, int2 posB, int2 sizeB)
	{
		return posA.x < posB.x + sizeB.x && posA.x + sizeA.x > posB.x &&
			posA.y < posB.y + sizeB.y && posA.y + sizeA.y > posB.y;
	}

};

class MyScript : public AsyncScript
{
public:
	using AsyncScript::AsyncScript; 

	void Initialize() override
	{
		std::cout << "My AsyncScript Initialize!\n";
		counter = 0;
		renderSystem = services->GetService<RenderSystem>();
		input = services->GetRequiredService<InputManager>();
	}

	void Update(float deltaTime) override
	{


	}

private:
	Sprite playerSprite;
	int counter = 0;
	std::shared_ptr<RenderSystem> renderSystem;
	std::shared_ptr<InputManager> input;
};

class ScriptSystem : public IGameSystem
{
public:
	void AddScript(std::shared_ptr<AsyncScript> script)
	{
		scripts.push_back(script);
		script->Run(); 
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
	Game(std::shared_ptr<ServiceProvider> services) : GameBase(services)
	{
		// Tomamos los sistemas ya creados
		window = services->GetRequiredService<GameWindow>();
		input = services->GetRequiredService<InputManager>();
		scriptSystem = services->GetRequiredService<ScriptSystem>();
		syncScripts = services->GetRequiredService<SyncScriptSystem>();
		sceneSystem = services->GetRequiredService<SceneSystem>();
		graphicsDevice = services->GetRequiredService<GraphicsDevice>();  // <--- CORRECTO
		renderSystem = services->GetRequiredService<RenderSystem>();

		// Agregamos a gameSystems para actualizar y dibujar
		gameSystems.push_back(input);
		gameSystems.push_back(scriptSystem);
		gameSystems.push_back(syncScripts);
		gameSystems.push_back(sceneSystem);
		gameSystems.push_back(renderSystem);

	}

	void BeginRun() override
	{
		std::cout << "[Game] BeginRun\n";
		if (window && window->Initialize())
		{
			input->Initialize(window->GetHWND());

			if (graphicsDevice)
			{
				if (!graphicsDevice->Initialize(window->GetHWND(), window->GetWidth(), window->GetHeight()))
				{
					std::cerr << "[Game] Error initializing graphics device.\n";
					return;
				}
			}
		}


		auto myAsyncScript = std::make_shared<MyScript>(services.get());
		scriptSystem->AddScript(myAsyncScript);

		auto mySyncScript = std::make_shared<MySyncScript>(services.get());
		syncScripts->AddScript(mySyncScript);
	}

private:
	std::shared_ptr<GraphicsDevice> graphicsDevice;
	std::shared_ptr<RenderSystem> renderSystem;
	std::shared_ptr<GameWindow> window;
	std::shared_ptr<InputManager> input;
	std::shared_ptr<ScriptSystem> scriptSystem;
	std::shared_ptr<SyncScriptSystem> syncScripts;
	std::shared_ptr<SceneSystem> sceneSystem;
};

int main()
{
	// Crear servicios compartidos
	auto services = std::make_shared<ServiceProvider>();

	auto graphicsDevice = std::make_shared<GraphicsDevice>();
	services->AddService(graphicsDevice);

	auto window = std::make_shared<GameWindow>();
	services->AddService(window);

	auto input = std::make_shared<InputManager>();
	services->AddService(input);

	auto ecs = std::make_shared<ECS>();
	services->AddService(ecs);

	auto sceneSystem = std::make_shared<SceneSystem>(ecs);
	services->AddService(sceneSystem);

	auto syncScripts = std::make_shared<SyncScriptSystem>();
	services->AddService(syncScripts);

	auto asyncScripts = std::make_shared<ScriptSystem>();
	services->AddService(asyncScripts);

	auto renderSystem = std::make_shared<RenderSystem>(graphicsDevice);
	renderSystem->SetECS(ecs);
	services->AddService(renderSystem);

	// Crear y ejecutar el juego
	Game game(services);
	game.Run();

	return 0;
}