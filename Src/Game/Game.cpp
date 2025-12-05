// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "GameWindows.h"
#include <GameTime.h>
#include "RenderSystem.h"
#include <entt.hpp>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

class PlayerControler
{
public:


    uint32_t numInstances = 256 * 256 * 2;
    float dimension = 1.6f;
    void OnInitialize(entt::registry& registry)
    {

        auto entity = registry.create();
        registry.emplace<TransformComponent>(entity, TransformComponent{ 1, 1 ,1 });
		float rotation = 0.0f;
		uint32_t dim = static_cast<uint32_t>(std::cbrt(numInstances)); // using cube root to determine the dimension of the grid
		DirectX::XMFLOAT3 offset = { dimension, dimension, dimension };

        float halfDimOffsetX = (dim * offset.x) / 2.0f;
        float halfDimOffsetY = (dim * offset.y) / 2.0f;
        float halfDimOffsetZ = (dim * offset.z) / 2.0f;

        for (uint32_t x = 0; x < dim; ++x)
        {
            for (uint32_t y = 0; y < dim; ++y)
            {
                for (uint32_t z = 0; z < dim; ++z)
                {
                    uint32_t index = x * dim * dim + y * dim + z;


                    DirectX::XMFLOAT3 position =
                    {
                        -halfDimOffsetX + offset.x / 2.0f + x * offset.x,
                        -halfDimOffsetY + offset.y / 2.0f + y * offset.y,
                        -halfDimOffsetZ + offset.z / 2.0f + z * offset.z
                    };


					auto entity = registry.create();
					registry.emplace<TransformComponent>(entity, TransformComponent{ position.x, position.y ,position.z });

                }
            }
        }

    }
    void OnUpdate(entt::registry& registry, GameTime time)
    {
        //DirectX::XMFLOAT3 offset = { dimension, dimension, dimension };

//float halfDimOffsetX = (dim * offset.x) / 2.0f;
//float halfDimOffsetY = (dim * offset.y) / 2.0f;
//float halfDimOffsetZ = (dim * offset.z) / 2.0f;

//for (uint32_t x = 0; x < dim; ++x)
//{
//    for (uint32_t y = 0; y < dim; ++y)
//    {
//        for (uint32_t z = 0; z < dim; ++z)
//        {
//            uint32_t index = x * dim * dim + y * dim + z;


//            DirectX::XMFLOAT3 position =
//            {
//                -halfDimOffsetX + offset.x / 2.0f + x * offset.x,
//                -halfDimOffsetY + offset.y / 2.0f + y * offset.y,
//                -halfDimOffsetZ + offset.z / 2.0f + z * offset.z
//            };


//            DirectX::XMFLOAT3 cubeRotation =
//            {
//                rotation,
//                rotation,
//                rotation
//            };
//            if (index % 2 == 0)
//            {
//                cubeRotation.x = -rotation;
//                cubeRotation.y = -rotation;
//                cubeRotation.z = -rotation;
//            }
//            DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
//            DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYaw(cubeRotation.x, cubeRotation.y, cubeRotation.z);
//            DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.25f, 0.25f, 0.25f);
//            DirectX::XMMATRIX world = DirectX::XMMatrixTranspose(rot * trans * scale); // Transpose for HLSL
//            dataArray[index] = world;
//        }
//    }
//}



        auto view = registry.view<TransformComponent>();
        for (auto [entity, transform] : view.each())
        {

            transform.rotationX += static_cast<float>(time.GetDeltaTime()) * 0.5f;
            transform.rotationY += static_cast<float>(time.GetDeltaTime()) * 0.5f;
            transform.rotationZ += static_cast<float>(time.GetDeltaTime()) * 0.5f;

        }

    }
};

class MyGame 
{
public:
    MyGame() { }

    void Run()
    {
		Initialize();
		Update();
		Shutdown();
	}

    void Initialize()
    {
		// Initialize games
		gameTime = {};
		gameTime.OnInitialize();

		gameWindow = {};
        gameWindow.OnInitialize();

		// Initialize Game systems
		// PlayerController, Health, etc.
		playerControler = {};
		playerControler.OnInitialize(registry);


		// Initialize systems
        renderSystem = {};
        renderSystem.OnInitialize(gameWindow.GetHandle());
    }



    void Update() 
    {
        while (gameWindow.IsRunning())
        {
            gameWindow.PumpMessages();
            playerControler.OnUpdate(registry, gameTime);

			gameTime.OnUpdate();
			renderSystem.OnUpdate(registry, gameTime);
        }
    }

    void Shutdown() 
    {
        renderSystem.OnShutdown();
		gameWindow.OnShutdown();
    }

private:
	RenderSystem renderSystem;
    GameWindows gameWindow;
	GameTime gameTime;

	PlayerControler playerControler;

    entt::registry registry;

};





int main()
{
	MyGame myGame;
	myGame.Run();
    return 0;
}