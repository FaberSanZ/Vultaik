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
        TransformComponent trasform{};
		trasform.position = { -1.5f, 0.0f, 0.0f };
		trasform.rotation = { 0.0f, 0.0f, 0.0f };
		trasform.scale = { 0.8f, 0.8f, 0.8f };

        auto entity = registry.create();
        registry.emplace<TransformComponent>(entity, trasform);
        registry.emplace<MeshComponent>(entity, MeshComponent { ShapeType::Triangle, MeshType::Static });



        trasform.position = { 0.0f, 0.0f, 0.0f };
        trasform.rotation = { 0.0f, 0.0f, 0.0f };
        trasform.scale = { 0.8f, 0.8f, 0.8f };

        auto entity2 = registry.create();
        registry.emplace<TransformComponent>(entity2, trasform);
        registry.emplace<MeshComponent>(entity2, MeshComponent { ShapeType::Cuad, MeshType::Dynamic });



        trasform.position = { 1.5f, 0.0f, 0.0f };
        trasform.rotation = { 0.0f, 0.0f, 0.0f };
        trasform.scale = { 0.8f, 0.8f, 0.8f };

        auto entity3 = registry.create();
        registry.emplace<TransformComponent>(entity3, trasform);
        registry.emplace<MeshComponent>(entity3, MeshComponent{ ShapeType::Pentagon, MeshType::Kinematic });



        trasform.position = { 1.5f, 1.0f, 0.0f };
        trasform.rotation = { 0.0f, 0.0f, 0.0f };
        trasform.scale = { 0.8f, 0.8f, 0.8f };

        auto entity4 = registry.create();
        registry.emplace<TransformComponent>(entity4, trasform);
        registry.emplace<MeshComponent>(entity4, MeshComponent{ ShapeType::Hexagon, MeshType::Static });



        trasform.position = { 1.5f, -1.0f, 0.0f };
        trasform.rotation = { 0.0f, 0.0f, 0.0f };
        trasform.scale = { 0.8f, 0.8f, 0.8f };

        auto entity5 = registry.create();
        registry.emplace<TransformComponent>(entity5, trasform);
        registry.emplace<MeshComponent>(entity5, MeshComponent{ ShapeType::Circle, MeshType::Dynamic });


    }
    void OnUpdate(entt::registry& registry, GameTime time)
    {
        // Simple physics update: Move the object in a circular path
        float speed = 1.0f; // Radians per second
        float angle = static_cast<float>(time.GetTotalTime()) * speed;

        auto view = registry.view<TransformComponent>();
        for (auto [entity, trasform] : view.each())
        {


			trasform.rotation = {  0.0f, 0.0f, angle }; // Rotate around Z-axis
            registry.replace<TransformComponent>(entity, trasform);
		}
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

		physicsSystem = {};
		physicsSystem.OnInitialize(registry);

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

			physicsSystem.OnUpdate(registry, gameTime);
            renderSystem.OnUpdate(registry, gameTime);
            OnUpdate(registry, gameTime);
        }
    }

    GameWindows gameWindow;
    GameTime gameTime;

    RenderSystem renderSystem;
	PhysicsSystem physicsSystem;

    entt::registry registry;
};


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

    }
private:

};


int main()
{
    MyGame myGame;
    myGame.Run();
    return 0;
}