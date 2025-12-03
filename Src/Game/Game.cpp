// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "GameWindows.h"
#include <GameTime.h>
#include "RenderSystem.h"


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")


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


		// Initialize systems
        renderSystem = {};
        renderSystem.OnInitialize(gameWindow.GetHandle());
    }



    void Update() 
    {
        while (gameWindow.IsRunning())
        {
            gameWindow.PumpMessages();

			gameTime.OnUpdate();
			renderSystem.OnUpdate(gameTime);
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

};




int main()
{
	MyGame myGame;
	myGame.Run();
    return 0;
}