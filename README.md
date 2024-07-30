

<h1 align="center">
   <img src="Logo/vultaik-logo.png" width=410>

  
  ##               Vultaik is a 2D/3D toy engine implemented in [Vulkan®]([https://www.khronos.org/vulkan/](https://learn.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide))
  
</h1>


  ##              

## Overview
This toy engine is designed for the Vulkan learning process, I mainly use it to experiment with graphical or computing techniques and should not be used as a cerium product as it may have memory leaks and faulty or poorly optimized implementations.

## Low-level rendering backend
The rendering backend focuses entirely on Vulkan, so it reuses Vulkan enums and data structures where appropriate. However, the API greatly simplifies the more painful points of writing straight Vulkan. It's not designed to be the fastest renderer ever made, it's likely a happy middle ground between "perfect" Vulkan/D3D11 CPU overhead.

