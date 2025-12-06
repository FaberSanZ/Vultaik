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


    uint32_t numInstances = 128 * 128;
    float dimension = 1.0f;
    void OnInitialize(entt::registry& registry)
    {

		InstanceComponent instanceComp;

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

                    instanceComp.words.push_back(Transform
                    {
                        position,
                        DirectX::XMFLOAT3{0.0f, 0.0f, 0.0f},
                        DirectX::XMFLOAT3{0.25f, 0.25f, 0.25f}
                    });
                }
            }
        }

  //      MeshComponent meshComp;
  //      meshComp.shapeType = ShapeType::Cube;
		//meshComp.meshType = MeshType::Static;

  //      auto entity = registry.create();
		//registry.emplace<MeshComponent>(entity, meshComp);
  //      registry.emplace<InstanceComponent>(entity, instanceComp);
		//registry.emplace<TagComponent>(entity, TagComponent{ "First Cube" });

           
		// Create a simple square mesh for the second entity
        MeshComponent triangleMesh;
        triangleMesh.shapeType = ShapeType::Null;
		triangleMesh.meshType = MeshType::Dynamic;

		triangleMesh.Indices = 
        {
            // front face
            0, 1, 2, // first triangle

        };
		triangleMesh.Vertices = 
        {
            // Front face
            {{-0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        };

        auto entity2 = registry.create();
        registry.emplace<MeshComponent>(entity2, triangleMesh);
        registry.emplace<InstanceComponent>(entity2, instanceComp);
		registry.emplace<TagComponent>(entity2, TagComponent{ "triangle" });



    }
    void OnUpdate(entt::registry& registry, GameTime time)
    {

        auto view_inst = registry.view<InstanceComponent>();
        for (auto [entity,inst] : view_inst.each())
        {

        }

        if (GameInput::IsKeyPressed(GameInput::KeyCode::A))
        {
            MeshComponent meshComp;
            meshComp.shapeType = ShapeType::Cube;


            auto entity = registry.create();
            registry.emplace<MeshComponent>(entity, meshComp);
            //registry.emplace<InstanceComponent>(entity, InstanceComponent{ instancePositions });
            registry.emplace<TagComponent>(entity, TagComponent{ "First Cube" });
			std::cout << "Created new cube entity!" << std::endl;
        }

        auto view = registry.view<TagComponent, InstanceComponent, MeshComponent>();
        for (auto [entity, tag, inst, mesh] : view.each())
        {
            // 1. Lógica de Malla Dinámica (Solo una vez por entidad)
            if (tag.Tag == "triangle")
            {

                float timeFactor = static_cast<float>(time.GetTotalTime()) * 5;

                mesh.Vertices[0].Position.x = 0.5f + 0.5f * sinf(timeFactor);
                mesh.Vertices[1].Position.x = 0.5f + 0.5f * cosf(timeFactor);
                mesh.Vertices[2].Position.y = -0.5f + 0.5f * sinf(timeFactor + 1.0f);
            }

            for (auto& instance : inst.words)
            {
                //instance.rotation.y += 0.5f * time.GetDeltaTime();
            }
        }
    }


};

class TerrainSystem
{
    public:
    void OnInitialize(entt::registry& registry)
    {


    }
    void OnUpdate(entt::registry& registry, GameTime time)
    {
        // Update terrain system
	}
private:
	uint16_t terrainWidth = 512;
	uint16_t terrainHeight = 512;
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

		terrainSystem = {};
		terrainSystem.OnInitialize(registry);


		// Initialize systems
        renderSystem = {};
        renderSystem.OnInitialize(registry, gameWindow.GetHandle());
    }



    void Update() 
    {
        while (gameWindow.IsRunning())
        {
			gameTime.OnUpdate();
            gameWindow.PumpMessages();

            playerControler.OnUpdate(registry, gameTime);
			terrainSystem.OnUpdate(registry, gameTime);

			renderSystem.OnUpdate(registry, gameTime);
        }
    }

    void Shutdown() 
    {
        renderSystem.OnShutdown();
		gameWindow.OnShutdown();
    }

private:
    GameWindows gameWindow;
	GameTime gameTime;

	RenderSystem renderSystem;
	PlayerControler playerControler;
	TerrainSystem terrainSystem;

    entt::registry registry;

};





int main()
{
	MyGame myGame;
	myGame.Run();
    return 0;
}