using Silk.NET.GLFW;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Vultaik
{
    public unsafe class Window : IDisposable
    {

        private readonly WindowHandle* _handle;
        private Glfw native_api;

        public Window(int width, int height, string title)
        {
            Width = width;
            Height = height;
            Title = title;

            native_api = Glfw.GetApi();


            native_api.Init();
            native_api.WindowHint(WindowHintClientApi.ClientApi, ClientApi.NoApi);
            native_api.WindowHint(WindowHintBool.Visible, false);
            _handle = native_api.CreateWindow(width, height, title, null, null);

            native_api.MakeContextCurrent(_handle);

            GlfwNativeWindow glfw_native = new(native_api, _handle);
            Handle = glfw_native.Win32!.Value.Hwnd;


        }





        public int Width { get; set; }
        public int Height { get; set; }
        public string Title { get; set; }
        public IntPtr Handle { get; set; }


        public void Show()
        {
            native_api.ShowWindow(_handle);
        }


        public void RenderLoop(Action loop)
        {
            while (!native_api.WindowShouldClose(_handle))
            {
                loop();
                native_api.PollEvents();
            }
        }


        public void Dispose()
        {
            native_api.DestroyWindow(_handle);
        }
    }
}
