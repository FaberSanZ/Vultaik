// Game.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <WindowsApp.h>

using namespace Desktop;

int main()
{
	WindowsApp app(1280, 720, L"Game Engine [Vultaik] {DirectX 12}!");
	if (!app.Initialize(GetModuleHandle(nullptr)))
	{
		
		std::cerr << "Failed to initialize the application.\n";
		return -1;
	}
	app.SetOnUpdate([]() 
	{
		// Update logic here
	});
	app.SetOnRender([]() 
	{
		// Render logic here
	});
	app.SetOnResize([](UINT width, UINT height) 
	{
		// Handle window resize
		std::cout << "Window resized to: " << width << "x" << height << "\n";
	});

	int result = app.Run();

	if (result < 0)
	{
		std::cerr << "Application exited with error code: " << result << "\n";
	}
	return result;
}
