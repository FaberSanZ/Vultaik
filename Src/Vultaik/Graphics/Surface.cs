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
    public unsafe class Surface : IDisposable
    {
        internal Adapter _adapter;
        internal VkSurfaceKHR _surface;

        public Surface(Adapter adapter, Window window)
        {
            _adapter = adapter;
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


            VkSurfaceKHR surface;
            vkCreateWin32SurfaceKHR(_adapter.instance, &windowsSurfaceInfo, null, &surface).CheckResult();
            _surface = surface;
        }



        public void Dispose()
        {
            
            if (_surface != VkSurfaceKHR.Null)
                vkDestroySurfaceKHR(_adapter.instance, _surface);
        }
    }
}
