// Copyright (c) 2019-2020 Faber Leonardo. All Rights Reserved.

/*=============================================================================
	GraphicsAdapter.cs
=============================================================================*/

using System;
using System.Collections.Generic;
using System.Text;
using Vortice;
using Vortice.DXGI;
using static Vortice.DXGI.DXGI;

namespace Renderer
{
    public class GraphicsAdapter
    {
        public List<string> Description { get; private set; } = new List<string>();

        public List<int> VendorId { get; private set; } = new List<int>();



        internal IDXGIAdapter Adapter;
        internal List<IDXGIAdapter> Adapters = new List<IDXGIAdapter>();
        internal IDXGIFactory4 NativeFactory;

        public GraphicsAdapter()
        {
            InitializeFromImpl();
        }




        public void Recreate()
        {
            InitializeFromImpl();
        }

        internal void InitializeFromImpl()
        {

            if (CreateDXGIFactory2(true, out IDXGIFactory4 tempDXGIFactory4).Failure)
                throw new InvalidOperationException("Cannot create IDXGIFactory4");


            NativeFactory = tempDXGIFactory4;


            NativeFactory.EnumAdapters1(1, out var adapter);
            
                AdapterDescription1 desc = adapter.Description1;



                Adapters.Add(adapter);
                Description.Add(adapter.Description.Description);
                VendorId.Add(adapter.Description.DeviceId);
            

            Adapter = Adapters[0]; // TODO:

        }
    }
}
