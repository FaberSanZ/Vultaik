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
        float dt = static_cast<float>(time.GetDeltaTime());

		// particle system 
        auto view = registry.view<ParticleComponent>();

        for (auto [entity, particle] : view.each())
        {
            particle.position.x += particle.velocity.x * dt;
            particle.position.y += particle.velocity.y * dt;


            if (particle.position.x > 3.1f) 
            {
				particle.velocity.x = -particle.velocity.x;
                auto& mesh = registry.get<MeshComponent>(entity); 
				mesh.meshType = MeshType::Static;

            }
            else if (particle.position.x < -3.1f) 
            {
                particle.velocity.x = -particle.velocity.x;
                auto& mesh = registry.get<MeshComponent>(entity);
                mesh.meshType = MeshType::Dynamic;
            }
		}



    }

	// TODO: move to game math, and add more math functions like sin, cos, tan, etc.
    float radians(float degrees)
    {
        return degrees * (GameMath::PI / 180.0f);
    }
private:

};


class MyGame
{
public:



    void Run()
    {
        // Initialize games
        gameTime = {};
        gameTime.OnInitialize();

        gameWindow = {};
        gameWindow.OnInitialize();



        //Scene1();
        Scene2();

		physicsSystem = {};
		physicsSystem.OnInitialize(registry);

        renderSystem = {};
        renderSystem.OnInitialize(registry, gameWindow.GetHandle(), gameWindow.GetClientWidth(), gameWindow.GetClientHeight());


        // Main loop
        Update();
    }



    void Scene1()
    {
        ParticleComponent particle{};
        particle.position = { 0.0f, -1.0f };
        particle.acceleraton = { 0.0f, 0.0f };
        particle.velocity = { 1.0f, 0.0f };
        particle.rotation = 0.0f;
        particle.scale = { 0.3f, 0.3f };
        particle.mass = 1.0f;

        auto entity = registry.create();
        registry.emplace<ParticleComponent>(entity, particle);
        registry.emplace<MeshComponent>(entity, MeshComponent{ ShapeType::Circle, MeshType::Dynamic });
    }


    void Scene2()
    {
        ParticleComponent particle{};
        particle.position = { 0.0f, -1.0f };
        particle.acceleraton = { 0.0f, 0.0f };
        particle.velocity = { 3.0f, 0.0f };
		particle.rotation = 0.0f;
		particle.scale = { 0.3f, 0.3f };
		particle.mass = 1.0f;

        auto entity = registry.create();
        registry.emplace<ParticleComponent>(entity, particle);
        registry.emplace<MeshComponent>(entity, MeshComponent{ ShapeType::Circle, MeshType::Dynamic });


        ParticleComponent particle2{};
        particle2.position = { 0.0f, 0.0f };
        particle2.acceleraton = { 0.0f, 0.0f };
        particle2.velocity = { 2.0f, 0.0f };
        particle2.rotation = 0.0f;
        particle2.scale = { 0.3f, 0.3f };
        particle2.mass = 1.0f;

        auto entity2 = registry.create();
        registry.emplace<ParticleComponent>(entity2, particle2);
        registry.emplace<MeshComponent>(entity2, MeshComponent{ ShapeType::Circle, MeshType::Dynamic });


        ParticleComponent particle3{};
        particle3.position = { 0.0f, 1.0f };
        particle3.acceleraton = { 0.0f, 0.0f };
        particle3.velocity = { 1.0f, 0.0f };
        particle3.rotation = 0.0f;
        particle3.scale = { 0.3f, 0.3f };
        particle3.mass = 1.0f;

        auto entity3 = registry.create();
        registry.emplace<ParticleComponent>(entity3, particle3);
        registry.emplace<MeshComponent>(entity3, MeshComponent{ ShapeType::Circle, MeshType::Dynamic });

	}


    void Update()
    {
        while (gameWindow.IsRunning())
        {
            gameWindow.PumpMessages();
            gameTime.OnUpdate();

			physicsSystem.OnUpdate(registry, gameTime);
            renderSystem.OnUpdate(registry, gameTime);
        }
    }

    GameWindows gameWindow;
    GameTime gameTime;

    RenderSystem renderSystem;
	PhysicsSystem physicsSystem;

    entt::registry registry;
};



int main()
{
    MyGame myGame;
    myGame.Run();
    return 0;
}