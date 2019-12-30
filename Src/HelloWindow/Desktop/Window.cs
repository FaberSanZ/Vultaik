// Copyright (c) 2019-2020 Faber Leonardo. All Rights Reserved.

/*=============================================================================
	Window.cs
=============================================================================*/



using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;


namespace Zeckoxe.Desktop
{
    public class Window
    {
        Form form;
        public Window(string title, int width, int height)
        {
            form = new Form();
            form.Text = title;
            form.Width = width;
            form.Height = height;
            
        }



        public void RenderLoop(Action render)
        {
            if (this is null)
                throw new ArgumentNullException("Windows");

            if (render is null)
                throw new ArgumentNullException("renderCallback");

            form.Show();

            using var renderLoop = new RenderLoop(form)
            {
                UseApplicationDoEvents = false,
                //AllowWindowssKeys = true
            };

            while (renderLoop.NextFrame())
                render();

        }
    }



}
