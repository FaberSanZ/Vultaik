// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "GameWindows.h"
#include "GameTime.h"
#include "RenderSystem.h"
#include "entt.hpp"


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

class PhysicsSystem
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
        renderSystem.OnInitialize(registry, gameWindow.GetHandle(), gameWindow.GetClientWidth(), gameWindow.GetClientHeight());


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


// TODO: Implement MyGame class that inherits from GameBase and defines game-specific logic, examples of systems, and game scenes
class MyGame : public GameBase
{
public:
    void OnInitialize(entt::registry& registry) override
    {
    }
    void OnUpdate(entt::registry& registry, GameTime time) override
    {
    }
    void OnShutdown() override
    {
        // Cleanup resources
    }
private:

};


int main()
{
    MyGame myGame;
    myGame.Run();
    return 0;
}