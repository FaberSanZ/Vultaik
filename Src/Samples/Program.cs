using System;
using Desktop;

namespace Samples
{
    class Program
    {
        static void Main(string[] args)
        {
            Window window = new Window("", 800, 600);

            window.RenderLoop(() => { });
        }
    }
}
