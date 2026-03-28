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


class RenderSystem
{
private:

public:
    RenderSystem()
    {
    }

    uint32_t m_Width{ 1200 }; // Width of the render target
    uint32_t m_Height{ 820 }; // Height of the render target
    Render render{};

    void OnInitialize(entt::registry& registry, HWND hwnd)
    {
        render.Initialize(hwnd, m_Width, m_Height);
    }



    void OnUpdate(entt::registry& registry, GameTime time)
    {

        Update(registry, time);
        Loop(registry);
    }


    void Loop(entt::registry& registry)
    {
        render.Loop();
    }


    void Update(entt::registry& registry, GameTime time)
    {
    }

    void OnShutdown()
    {
        render.Cleanup();

    }

};


