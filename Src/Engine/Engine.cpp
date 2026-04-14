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
            //if (registry.valid(entity))
            //{

            //}
             
            
            // Update particle physics
			particle.velocity.x += particle.acceleraton.x * dt;
			particle.velocity.y += particle.acceleraton.y * dt;

            particle.position.x += particle.velocity.x * dt;
            particle.position.y += particle.velocity.y * dt;




			// Scene3
            if(particle.position.y < -1.8f)
            {
                particle.position.y = -1.8f;
                particle.velocity.y *= -0.8f;
			}

            //if (particle.position.y < -1.8f)
            //{
            //    registry.destroy(entity);
            //}

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
        //Scene2();
        Scene3();

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
        registry.emplace<MeshComponent>(entity, MeshComponent{ ShapeType::Circle, MeshType::Dynamic });
        registry.emplace<ParticleComponent>(entity, particle);
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

	// In this scene we will create three particles with different gravity values, and see how they fall at different speeds.
    void Scene3()
    {
		// first particle with low gravity
        ParticleComponent particle{};
        particle.position = { -1.3f, 1.0f };
        particle.acceleraton = { 0.0f, -0.5f };
        particle.velocity = { 0.0f, 0.0f };
        particle.rotation = 0.0f;
        particle.scale = { 0.5f, 0.5f };
        particle.mass = 1.0f;

        auto entity = registry.create();
        registry.emplace<ParticleComponent>(entity, particle);
        registry.emplace<MeshComponent>(entity, MeshComponent{ ShapeType::Circle, MeshType::Dynamic });


		// second particle with normal gravity
        ParticleComponent particle2{};
        particle2.position = { 0.0f, 1.0f };
        particle2.acceleraton = { 0.0f, -1.f };
        particle2.velocity = { 0.0f, 0.0f };
        particle2.rotation = 0.0f;
        particle2.scale = { 0.4f, 0.4f };
        particle2.mass = 1.0f;

        auto entity2 = registry.create();
        registry.emplace<ParticleComponent>(entity2, particle2);
        registry.emplace<MeshComponent>(entity2, MeshComponent{ ShapeType::Circle, MeshType::Kinematic });


		// third particle with high gravity
        ParticleComponent particle3{};
        particle3.position = { 1.0f, 1.0f };
        particle3.acceleraton = { 0.0f, -1.5f };
        particle3.velocity = { 0.0f, 0.0f };
        particle3.rotation = 0.0f;
        particle3.scale = { 0.3f, 0.3f };
        particle3.mass = 1.0f;

        auto entity3 = registry.create();
        registry.emplace<ParticleComponent>(entity3, particle3);
        registry.emplace<MeshComponent>(entity3, MeshComponent{ ShapeType::Circle, MeshType::Static });
	}


    void Update()
    {
        while (gameWindow.IsRunning())
        {


			physicsSystem.OnUpdate(registry, gameTime);
            renderSystem.OnUpdate(registry, gameTime);


            gameWindow.PumpMessages();
            gameTime.OnUpdate();
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