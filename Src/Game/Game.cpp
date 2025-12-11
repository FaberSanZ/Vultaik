// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "GameWindows.h"
#include <GameTime.h>
#include "RenderSystem.h"
#include <entt.hpp>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

class PlayerControlerSystem
{
public:

    void OnInitialize(entt::registry& registry)
    {
    }
    void OnUpdate(entt::registry& registry, GameTime time)
    {
    }
private:

};

class TerrainSystem
{
public:
    void OnInitialize(entt::registry& registry)
    {
    }
    void OnUpdate(entt::registry& registry, GameTime time)
    {
	}
private:

};






class GameBase
{
public:

    virtual void OnInitialize(entt::registry& registry) = 0;
    virtual void OnUpdate(entt::registry& registry, GameTime time) = 0;

    virtual void OnShutdown() = 0;

    void Run()
    {
        // Initialize games
        gameTime = {};
        gameTime.OnInitialize();

        gameWindow = {};
        gameWindow.OnInitialize();


        OnInitialize(registry);


        // Initialize systems
		// TODO: Physics system, AI system, etc.
        renderSystem = {};
        renderSystem.OnInitialize(registry, gameWindow.GetHandle());


		// Main loop
		Update();
    }



    void Update()
    {
        while (gameWindow.IsRunning())
        {
            gameWindow.PumpMessages();
            gameTime.OnUpdate();


			renderSystem.OnUpdate(registry, gameTime);
            OnUpdate(registry, gameTime);
        }
    }

    GameWindows gameWindow;
    GameTime gameTime;

    RenderSystem renderSystem;

    entt::registry registry;
};

class MyGame : public GameBase
{
public:
    void OnInitialize(entt::registry& registry) override
    {
        playerControler = {};
        playerControler.OnInitialize(registry);

        terrainSystem = {};
        terrainSystem.OnInitialize(registry);
    }
    void OnUpdate(entt::registry& registry, GameTime time) override
    {
        playerControler.OnUpdate(registry, time);
        terrainSystem.OnUpdate(registry, time);
    }
    void OnShutdown() override
    {
        // Cleanup resources
    }
private:

    PlayerControlerSystem playerControler;
    TerrainSystem terrainSystem;
};

// TODO: Implement GameScene class for scene management
class GameScene 
{
public:
    GameScene() = default;

    entt::registry& GetRegistry() { return registry; }
    bool LoadFromYAML(const std::string& filepath);
    bool SaveToYAML(const std::string& filepath);
    void Update(float deltaTime);

private:
    entt::registry registry;

};

int main()
{
	MyGame myGame;
	myGame.Run();
    return 0;
}