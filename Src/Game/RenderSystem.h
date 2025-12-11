
#include <windows.h>
#include <fstream>
#include <iostream>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "Adapter.h"
#include "Device.h"
#include "SwapChain.h"
#include "CommandList.h"
#include "Buffer.h"
#include "Texture.h"
#include "Pipeline.h"
#include "GameWindows.h"
#include "GameTime.h"
#include "entt.hpp"
#include "Components.h"


class RenderSystem
{
private:




public:
    RenderSystem()
    {
    }

    uint32_t m_Width{ 1200 }; // Width of the render target
    uint32_t m_Height{ 820 }; // Height of the render target




    void OnInitialize(entt::registry& registry, HWND hwnd)
    {



    }



    void OnUpdate(entt::registry& registry, GameTime time)
    {

        Update(registry, time);
        Loop(registry);
    }


    void Loop(entt::registry& registry)
    {
    }


    void Update(entt::registry& registry, GameTime time)
    {



    }



    void OnShutdown()
    {

    }

};


