using System;
using ClearScreen;
using Desktop;

namespace Samples
{
    class Program
    {
        static void Main(string[] args)
        {
            using (var App = new Game())
                App.Run();
        }
    }
}
