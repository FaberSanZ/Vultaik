// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)



using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vultaik;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vultaik.Graphics
{
    public unsafe class Surface
    {
        internal Adapter adapter;
        internal VkSurfaceKHR surface;

        public Surface(Adapter adapter, Window window)
        {
            this.adapter = adapter;
            Window = window;
            createSurface();
        }

        public Window Window { get; }

        private void createSurface()
        {
            VkWin32SurfaceCreateInfoKHR windowsSurfaceInfo = new VkWin32SurfaceCreateInfoKHR
            {
                sType = VkStructureType.Win32SurfaceCreateInfoKHR,
                hwnd = Window.Handle,
                hinstance = Process.GetCurrentProcess().Handle
            };


            VkSurfaceKHR _surface;
            vkCreateWin32SurfaceKHR(adapter.instance, &windowsSurfaceInfo, null, &_surface).CheckResult();
            surface = _surface;
        }

    }
}
