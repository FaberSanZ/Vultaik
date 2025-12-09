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


    uint32_t numInstances = 256 * 256 * 4;
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


                    Vector3 position =
                    {
                        -halfDimOffsetX + offset.x / 2.0f + x * offset.x,
                        -halfDimOffsetY + offset.y / 2.0f + y * offset.y,
                        -halfDimOffsetZ + offset.z / 2.0f + z * offset.z
                    };

                    instanceComp.words.push_back(Transform
                    {
                        index,
                        "Cube",
                        position,
                        Vector3 {0.0f, 0.0f, 0.0f},
                        Vector3 {0.25f, 0.25f, 0.25f}
                    });
                }
            }
        }

        MeshComponent meshComp;
        meshComp.shapeType = ShapeType::Cube;
		meshComp.meshType = MeshType::Static;

        auto entity = registry.create();
		registry.emplace<MeshComponent>(entity, meshComp);
        registry.emplace<InstanceComponent>(entity, instanceComp);
		registry.emplace<TagComponent>(entity, TagComponent{ "First Cube" });

          



    }
    void OnUpdate(entt::registry& registry, GameTime time)
    {

        auto view_inst = registry.view<InstanceComponent>();
        for (auto [entity,inst] : view_inst.each())
        {

        }
		// Add a new entity with a triangle mesh when the 'A' key is pressed
        if (GameInput::IsKeyPressed(GameInput::KeyCode::A))
        {
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
            InstanceComponent instanceComp;
            instanceComp.words.push_back(Transform
            {
                0,
				"triangle_instance_1", // parent name
                Vector3 {0.0f, 0.0f, 0.0f},
                Vector3 {0.0f, 0.0f, 0.0f},
                Vector3 {2.8f, 2.8f, 2.8f}
            });


            auto entity2 = registry.create();
            registry.emplace<MeshComponent>(entity2, triangleMesh);
            registry.emplace<InstanceComponent>(entity2, instanceComp);
            registry.emplace<TagComponent>(entity2, TagComponent{ "triangle" });
        }


        // Add a new entity with a cube mesh when the 'D' key is pressed
        if (GameInput::IsKeyPressed(GameInput::KeyCode::B))
        {
            // Create a simple square mesh for the second entity
            MeshComponent cubeMesh;
            cubeMesh.shapeType = ShapeType::Cube;
            cubeMesh.meshType = MeshType::Static;

            InstanceComponent instanceComp;
            instanceComp.words.push_back(Transform
            {
                0,
				"cube_instance_1",
                Vector3 {0.0f, 0.0f, 0.0f},
                Vector3 {0.0f, 0.0f, 0.0f},
                Vector3 {1.8f, 1.8f, 1.8f}
            });


            auto entity = registry.create();
            registry.emplace<MeshComponent>(entity, cubeMesh);
            registry.emplace<InstanceComponent>(entity, instanceComp);
            registry.emplace<TagComponent>(entity, TagComponent{ "cube" });
        }


        auto view = registry.view<TagComponent, InstanceComponent, MeshComponent>();
        for (auto [entity, tag, inst, mesh] : view.each())
        {

            if (tag.Tag == "First Cube")
            {
                for (auto& instance : inst.words)
                {
                    if (instance.id % 2 == 0) 
                    {
                        instance.rotation.x += 1.0f * time.GetDeltaTime();
                        instance.rotation.x += 0.5f * time.GetDeltaTime();
                        instance.rotation.z += 0.8f * time.GetDeltaTime();
                    }

                }
            }
            if (tag.Tag == "cube")
            {
                for (auto& instance : inst.words)
                {
                    instance.scale.x += 0.1f * time.GetDeltaTime();
                    instance.scale.y += 0.1f * time.GetDeltaTime();
                    instance.scale.z += 0.1f * time.GetDeltaTime();


                    instance.rotation.x += 1.0f * time.GetDeltaTime();
                    instance.rotation.x += 0.5f * time.GetDeltaTime();
                    instance.rotation.z += 0.8f * time.GetDeltaTime();
                    if(instance.scale.x >= 3.0f)
                    {
                        instance.scale.x = 1.0f;
                        instance.scale.y = 1.0f;
                        instance.scale.z = 1.0f;
					}
                }
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

    // Acceso al registry
    entt::registry& GetRegistry() { return registry; }

    // Cargar escena desde YAML
    bool LoadFromYAML(const std::string& filepath);

    // Guardar escena a YAML
    bool SaveToYAML(const std::string& filepath);

    // Actualizar escena, si es necesario
    void Update(float deltaTime);

private:
    entt::registry registry;

    // Datos internos para sincronización, cache, etc
};

int main()
{
	MyGame myGame;
	myGame.Run();
    return 0;
}