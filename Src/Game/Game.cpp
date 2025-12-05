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


    uint32_t numInstances = 8 * 8;
    float dimension = 1.6f;
    std::vector<DirectX::XMFLOAT3> instancePositions;
    void OnInitialize(entt::registry& registry)
    {



		uint32_t dim = static_cast<uint32_t>(std::cbrt(numInstances)); // using cube root to determine the dimension of the grid
		DirectX::XMFLOAT3 offset = { dimension, dimension, dimension };


        float halfDimOffsetX = (dim * offset.x) / 2.0f;
        float halfDimOffsetY = (dim * offset.y) / 2.0f;
        float halfDimOffsetZ = (dim * offset.z) / 2.0f;




     //   for (uint32_t x = 0; x < dim; ++x)
     //   {
     //       for (uint32_t y = 0; y < dim; ++y)
     //       {
     //           for (uint32_t z = 0; z < dim; ++z)
     //           {
     //               uint32_t index = x * dim * dim + y * dim + z;


     //               DirectX::XMFLOAT3 position =
     //               {
     //                   -halfDimOffsetX + offset.x / 2.0f + x * offset.x,
     //                   -halfDimOffsetY + offset.y / 2.0f + y * offset.y,
     //                   -halfDimOffsetZ + offset.z / 2.0f + z * offset.z
     //               };

					//instancePositions.push_back(position);
     //           }
     //       }
     //   }

        MeshComponent meshComp;
        meshComp.shapeType = ShapeType::Cube;


  //      auto entity = registry.create();
		//registry.emplace<MeshComponent>(entity, meshComp);
  //      registry.emplace<TransformComponent>(entity, TransformComponent{ 1, 1 , 1, 0, 0, 0, 0.25f });
  //      registry.emplace<InstanceComponent>(entity, InstanceComponent{ instancePositions });
		//registry.emplace<TagComponent>(entity, TagComponent{ "First Cube" });

           
		// Create a simple square mesh for the second entity
  //      MeshComponent triangleMesh;
  //      triangleMesh.shapeType = ShapeType::Null;
		//triangleMesh.Indices = { 0, 1, 2 };
		//triangleMesh.Vertices = 
  //      {
  //          {{-0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
  //          {{ 0.5f, -0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
  //          {{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
  //      };

  //      auto entity2 = registry.create();
  //      registry.emplace<MeshComponent>(entity2, triangleMesh);
  //      registry.emplace<TransformComponent>(entity2, TransformComponent{ 1, 1 , 1, 0, 0, 0, 2 });
  //      registry.emplace<InstanceComponent>(entity2, InstanceComponent{ instancePositions });
		//registry.emplace<TagComponent>(entity2, TagComponent{ "Second Cube" });



    }
    void OnUpdate(entt::registry& registry, GameTime time)
    {

        auto view_inst = registry.view<InstanceComponent>();
        for (auto [entity,inst] : view_inst.each())
        {

        }

        auto view = registry.view<TransformComponent, TagComponent, InstanceComponent>();
        for (auto [entity, transform, tag, inst] : view.each())
        {
            if(tag.Tag == "Second Cube")
            {
                transform.rotationX += static_cast<float>(time.GetDeltaTime()) * 0.7f;
                transform.rotationY += static_cast<float>(time.GetDeltaTime()) * 0.5f;
                transform.rotationZ += static_cast<float>(time.GetDeltaTime()) * -0.7f;
			}
            else
            {
                transform.rotationX += static_cast<float>(time.GetDeltaTime()) * -0.7f;
                transform.rotationY += static_cast<float>(time.GetDeltaTime()) * -0.5f;
                transform.rotationZ += static_cast<float>(time.GetDeltaTime()) * 0.7f;
			}


            if (GameInput::IsKeyPressed(GameInput::KeyCode::C))
            {
				transform.x += 0.5f;
            }

            if (GameInput::IsKeyPressed(GameInput::KeyCode::W))
            {
                DirectX::XMFLOAT3 position =
                {
                    offset++, 0, 0
                };

                inst.instancePositions.push_back(position);
				std::cout << "Added Instance Position: (" << position.x << ", " << position.y << ", " << position.z << ")\n";
            }

            for (const auto& instancePos : inst.instancePositions)
            {
				std::cout << "Instance Position: (" << instancePos.x << ", " << instancePos.y << ", " << instancePos.z << ")\n";
            }



        }

        if(GameInput::IsKeyPressed(GameInput::KeyCode::A))
        {
            MeshComponent meshComp;
            meshComp.shapeType = ShapeType::Cube;


            auto entity = registry.create();
            registry.emplace<MeshComponent>(entity, meshComp);
            registry.emplace<TransformComponent>(entity, TransformComponent{ 1.0f, 1.0f , 1.0f, 1, 1, 1, 1.5f });
            registry.emplace<InstanceComponent>(entity, InstanceComponent{ instancePositions });
            registry.emplace<TagComponent>(entity, TagComponent{ "First Cube" });
		}



        //offset += 0.5f;

    }

	float offset = -1.0f;
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
		playerControler = {};
		playerControler.OnInitialize(registry);


		// Initialize systems
        renderSystem = {};
        renderSystem.OnInitialize(registry, gameWindow.GetHandle());
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