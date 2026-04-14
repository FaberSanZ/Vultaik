#include <windows.h>
#include <fstream>
#include <iostream>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "GameWindows.h"
#include "GameTime.h"
#include "entt.hpp"
#include "Components.h"
#include "RenderingDevice.h"
#include "EasyUI.h"


class RenderSystem
{
private:

public:
    RenderSystem()
    {
    }

    uint32_t m_Width{ }; // Width of the render target
    uint32_t m_Height{ }; // Height of the render target
    Render render{};
	Mesh triangle = {};
	Mesh cuad = {};
	Mesh pentagon = {};
	Mesh hexagon = {};
	Mesh circle = {};
	UIMesh uiMesh = {};
    UIManager uiManager; 


    void OnInitialize(entt::registry& registry, HWND hwnd,uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;

        render.Initialize(hwnd, m_Width, m_Height);




        GameInput::AddMouseMoveCallback([&](int x, int y)
            {
                uiManager.onMouseMove((float)x, (float)y);
            });

        GameInput::AddMouseCallback([&](int x, int y, GameInput::MouseButton btn, GameInput::KeyState state)
            {
                if (btn == GameInput::MouseButton::Left)
                {
                    if (state == GameInput::KeyState::Pressed)
                        uiManager.onMouseDown((float)x, (float)y, 0);

                    if (state == GameInput::KeyState::Released)
                        uiManager.onMouseUp((float)x, (float)y, 0);
                }
            });


		triangle = GeneratePolygonMesh(0.5f, 3);
		cuad = GeneratePolygonMesh(0.5f, 4);
		pentagon = GeneratePolygonMesh(0.5f, 5);
		hexagon = GeneratePolygonMesh(0.5f, 6);
		circle = GeneratePolygonMesh(0.5f, 12);


        uiManager.setScreenSize(m_Width, m_Height);

        if (!root)
        {
            // Root: ocupa toda la pantalla (stretch)
            root = new UIElement();
            root->setHorizontalAlignment(HorizontalAlignment::Stretch);
            root->setVerticalAlignment(VerticalAlignment::Stretch);
            root->setMargin(Thickness(0));
            root->setPadding(Thickness(0));   // sin padding interno
            root->setColor(0x80FFFFFF);

            // Botón 1: posición absoluta (20,20) se logra con margin left/top
            auto* playBtn = new UIButton();
            playBtn->setSize(100, 75);
            playBtn->setMargin(Thickness(20, 20, 0, 0));
            playBtn->setHorizontalAlignment(HorizontalAlignment::Center);
            playBtn->setVerticalAlignment(VerticalAlignment::Center);
            playBtn->setColor(0xFF00C8FF);
            playBtn->setBorder(2, 0xFFFFFFFF);
            playBtn->setOnClick([]() { std::cout << "Button 1 clicked!\n"; });

            // Botón 2: posición (140,20) -> margin left=140, top=20
            auto* playBtn2 = new UIButton();
            playBtn2->setSize(100, 75);
            playBtn2->setMargin(Thickness(140, 20, 0, 0));
            playBtn2->setHorizontalAlignment(HorizontalAlignment::Left);
            playBtn2->setVerticalAlignment(VerticalAlignment::Top);
            playBtn2->setColor(0xFF9B59B6);
            playBtn2->setBorder(2, 0xFFFFFFFF);
            playBtn2->setOnClick([]() { std::cout << "Button 2 clicked!\n"; });

            // Slider: ancho 300, posición (50,200) -> margin left=50, top=200
            auto* slider = new UISlider();
            slider->setSize(300, 20);
            slider->setMargin(Thickness(50, 200, 0, 0));
            slider->setColor(0xFF3A3A3A);          // fondo del carril
            slider->setHandleSize(24);              // handle más grande
            slider->setHandleColor(0xFF00C8FF);     // cian brillante
            slider->setHandleBorder(2, 0xFFFFFFFF); // borde blanco
            slider->setOnChange([](float v) { std::cout << v << std::endl; });

            // Checkbox: posición (100,100)
            auto* check = new UICheckBox();
            check->setSize(30, 30);
            check->setMargin(Thickness(100, 100, 0, 0));
            check->setHorizontalAlignment(HorizontalAlignment::Left);
            check->setVerticalAlignment(VerticalAlignment::Top);
            check->setBorder(2, 0xFFFFFFFF);
            check->setColor(0x00FFFFFF);
            check->setOnChange([](bool v) { std::cout << "Vsync: " << v << std::endl; });

            // Dropdown: posición (500,100)
            auto* dropdown = new UIDropdown();
            dropdown->setSize(200, 30);
            dropdown->setMargin(Thickness(500, 100, 0, 0));
            dropdown->setHorizontalAlignment(HorizontalAlignment::Left);
            dropdown->setVerticalAlignment(VerticalAlignment::Top);
            dropdown->setColor(0xFF3A3A3A);
            dropdown->setBorder(1, 0xFFFFFFFF);
            dropdown->addOption("Opción 1");
            dropdown->addOption("Opción 2");
            dropdown->addOption("Opción 3");
            dropdown->addOption("Opción 4");
            dropdown->setOnChange([](int idx, const std::string& text) {
                std::cout << "Seleccionado: " << idx << " - " << text << std::endl;
                });

            // ProgressBar: posición (50,300)
            progress = new UIProgressBar();
            progress->setSize(300, 20);
            progress->setMargin(Thickness(50, 300, 0, 0));
            progress->setHorizontalAlignment(HorizontalAlignment::Left);
            progress->setVerticalAlignment(VerticalAlignment::Top);
            progress->setColor(0xFF3A3A3A);
            progress->setBorder(1, 0xFFFFFFFF);
            progress->setFillColor(0xFF00E5FF);
            progress->setValue(0.01f);


            // En OnInitialize
            auto fontAtlas = new FontAtlas();
            fontAtlas->LoadFromFile("../../../Assets/Fonts/Roboto-Regular.ttf", 24.0f);

            // Luego, creas un UIText
            auto* text = new UIText();
            text->setFontAtlas(fontAtlas);
            text->setText("Hello World!");
            text->setFontSize(24);
            text->setColor(0xFFFFFFFF);
            text->setMargin(Thickness(10, 10, 0, 0));
            text->setHorizontalAlignment(HorizontalAlignment::Left);
            text->setVerticalAlignment(VerticalAlignment::Top);
            root->addChild(text);

            // Añadir todos al root
            root->addChild(playBtn);
            root->addChild(playBtn2);
            root->addChild(slider);
            root->addChild(check);
            root->addChild(dropdown);
            root->addChild(progress);

            uiManager.setRoot(root);
        }
        std::vector<UIVertex> vertices;
        std::vector<uint32_t> indices;

        vertices.clear();
        indices.clear();

        uiManager.updateLayout();
        uiManager.render(vertices, indices);

        uiMesh = render.CreateUIMesh(reinterpret_cast<uiVertex*>(vertices.data()), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size());



        
    }



    UIProgressBar* progress = nullptr;  // Guardamos la referencia
    UIElement* root = nullptr;


    Mesh GeneratePolygonMesh(float radius, uint32_t segmentCount)
    {
        std::vector<Vertex> vertices;
        vertices.reserve(segmentCount + 1);

        for (uint32_t i = 0; i <= segmentCount; ++i)
        {
            float angle = (2.0f * 3.14159265f * i) / segmentCount;
            float x = radius * cos(angle);
            float y = radius * sin(angle);

            vertices.push_back({ x, y, 0.0f, 1.0f});
        }

        std::vector<uint32_t> indices;
        indices.reserve(segmentCount * 3);

        for (uint32_t i = 0; i < segmentCount; ++i)
        {
            indices.push_back(0);
            indices.push_back(i + 2);
            indices.push_back(i + 1);
        }

        return render.CreateMesh(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size());
        
    }



	float speed = 0.0f;
	float progressValue = 0.0f;

    void OnUpdate(entt::registry& registry, GameTime time)
    {
		Update(registry, time);
        Loop(registry);
    }

    void Loop(entt::registry& registry)
    {
		render.Reset(); // Reset the command allocator and command list for the current frame
		render.Clear(); // Clear the render target and depth/stencil buffer, and set them for rendering
		render.BeginFrame(); // Set the viewport, scissor rect, and pipeline state for the current frame

        render.BeginUI();
        uiMesh.Draw(render.commandList); // Draw the UI mesh using the command list


		render.BeginGame();
		triangle.Draw(render.commandList); // Draw the mesh using the command list 
		cuad.Draw(render.commandList); // Draw the mesh using the command list
		pentagon.Draw(render.commandList); // Draw the mesh using the command list
		hexagon.Draw(render.commandList); // Draw the mesh using the command list
		circle.Draw(render.commandList); // Draw the mesh using the command list



        render.Loop();
    }


	float gettrasparencyfromvalue(float value)
	{
		return (sin(value) + 1.0f) / 2.0f; // Oscila entre 0 y 1
	}

    void Update(entt::registry& registry, GameTime time)
    {
		speed += 0.01f;
		progress->setValue((sin(speed) + 1.0f) / 2.0f);
		root->setColor(0x80FF0000 + (uint32_t)((sin(speed) + 1.0f) / 2.0f * 0x7F) * 0x10000); // Cambia el rojo del fondo con el tiempo


        std::vector<UIDrawCommand> commands;
        uiManager.render(commands);

        std::vector<UIVertex> allVerts;
        std::vector<uint32_t> allIndices;
        uint32_t base = 0;
        for (auto& cmd : commands) {
            allVerts.insert(allVerts.end(), cmd.vertices.begin(), cmd.vertices.end());
            for (uint32_t idx : cmd.indices)
                allIndices.push_back(base + idx);
            base += (uint32_t)cmd.vertices.size();
        }

        render.UpdateUIGeometry(uiMesh, reinterpret_cast<uiVertex*>(allVerts.data()), (uint32_t)allVerts.size(), allIndices.data(), (uint32_t)allIndices.size());
        


        std::vector<InstanceData> triangleInstancing;
        std::vector<InstanceData> cuadInstancing;
        std::vector<InstanceData> pentagonInstancing;
		std::vector<InstanceData> hexagonInstancing;
        std::vector<InstanceData> circleInstancing;
        std::vector<InstanceData> polygonInstancing;

        DirectX::XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

        auto view_mesh = registry.view<MeshComponent>();

        for (auto [entity, mesh] : view_mesh.each())
        {

            if (mesh.meshType == MeshType::Static)
                color = { 0.4f, 0.7f, 0.3f, 1.0f };

            else if (mesh.meshType == MeshType::Dynamic)
                color = { 0.25f, 0.45f, 0.75f, 1.0f };

            else if (mesh.meshType == MeshType::Kinematic)
                color = { 0.9f, 0.6f, 0.2f, 1.0f };


            if (registry.all_of<TransformComponent>(entity))
            {
                auto& trasform = registry.get<TransformComponent>(entity);

                if (mesh.shapeType == ShapeType::Triangle)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    triangleInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Cuad)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    cuadInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Pentagon)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    pentagonInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Hexagon)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    hexagonInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }

                if (mesh.shapeType == ShapeType::Circle)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    circleInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }
            }
            else if (registry.all_of<ParticleComponent>(entity))
            {
                auto& trasform = registry.get<ParticleComponent>(entity);

                if (mesh.shapeType == ShapeType::Triangle)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    triangleInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Cuad)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    cuadInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Pentagon)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    pentagonInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }


                if (mesh.shapeType == ShapeType::Hexagon)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    hexagonInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }

                if (mesh.shapeType == ShapeType::Circle)
                {
                    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(trasform.position.x, trasform.position.y, 0.0f);
                    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, trasform.rotation);
                    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(trasform.scale.x, trasform.scale.y, 0.0f);
                    circleInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
                }
            }
        }


        if(triangleInstancing.size() > 0)
			render.UpdateInstanceBuffer(triangle, triangleInstancing.data(), triangleInstancing.size());

        if (cuadInstancing.size() > 0)
            render.UpdateInstanceBuffer(cuad, cuadInstancing.data(), cuadInstancing.size());

		if (pentagonInstancing.size() > 0)
			render.UpdateInstanceBuffer(pentagon, pentagonInstancing.data(), pentagonInstancing.size());

        if(hexagonInstancing.size() > 0)
			render.UpdateInstanceBuffer(hexagon, hexagonInstancing.data(), hexagonInstancing.size());

        if (circleInstancing.size() > 0)
            render.UpdateInstanceBuffer(circle, circleInstancing.data(), circleInstancing.size());

		std::vector<InstanceData> uiInstancing;

        DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0, 0, 0.0f);
        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
        DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1.0f, 1.0f, 0.0f);

        uiInstancing.push_back({ DirectX::XMMatrixTranspose(scale * rotation * translation), color });
		render.UpdateUIInstanceBuffer(uiMesh, uiInstancing.data(), uiInstancing.size());




    }

    void OnShutdown()
    {
        render.Cleanup();
    }

};


