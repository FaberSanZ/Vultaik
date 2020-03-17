using System;
using Zeckoxe.Desktop;

namespace _01__CreatingWindow
{
    public class D3D12Graphics : IDisposable
    {
        public Window Window { get; set; }

        public D3D12Graphics()
        {
            // create the window
            InitializeWindow();

            // initialize direct3d
            InitD3D();

            // start the main loop
            Mainloop();

            // we want to wait for the gpu to finish executing the command list before we start releasing everything
            WaitForPreviousFrame();



            // clean up everything
            Cleanup();
        }


        public void InitializeWindow()
        {
            Window = new Window(string.Empty, 1000, 720);
            Window?.Show();
        }

        public void InitD3D()
        {

        }

        private void WaitForPreviousFrame()
        {
        }

        private void Mainloop()
        {
            Window.RenderLoop(() =>
            {


            });
        }

        private void Cleanup()
        {
        }
        public void Dispose()
        {
            Cleanup();
        }


    }
}
