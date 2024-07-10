// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)


using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Vultaik
{
    public class Application
    {


        public Application()
        {

            Window = new(800, 600, "Vultaik");

            Time = new();
            Time.Reset();
        }






        public Window Window { get; set; }
        public TimerTick Time { get; }



        public void Run()
        {

            Initialize();
            LoadContentAsync();

            Time.Tick(); // update
            Window?.Show();
            //Window!.Resize += ExampleBase_Resize;
            Window.RenderLoop(() =>
            {
                Update(Time);
                Draw(Time);
                Time.Tick();
            });
        }


        public virtual void Initialize()
        {
        }


        public virtual Task LoadContentAsync()
        {
            return Task.CompletedTask;
        }

        public virtual void Draw(TimerTick time)
        {
        }

        public virtual void Update(TimerTick time)
        {
        }

        public virtual void Resize(int width, int height)
        {

        }

    }
}
