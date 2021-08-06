// Copyright (c) 2019-2020 Faber Leonardo. All Rights Reserved.

/*=============================================================================
	GraphicsDevice.cs
=============================================================================*/


using System;
using System.Collections.Generic;
using System.Text;
using Vortice.Direct3D12;
using Vortice.Direct3D12.Debug;
using Vortice.Direct3D;
using Vortice.DXGI;
using static Vortice.Direct3D12.D3D12;
using static Vortice.DXGI.DXGI;

namespace Renderer
{
    public sealed unsafe class GraphicsDevice : IDisposable
    {
        public CommandQueue NativeDirectCommandQueue { get; private set; }

        public GraphicsAdapter NativeAdapter { get; }

        public RenderDescriptor NativeParameters { get; internal set; }

        //public SwapChain NativeSwapChain { get; }

        public DescriptorAllocator DepthStencilViewAllocator { get; private set; }

        public DescriptorAllocator RenderTargetViewAllocator { get; private set; }
        public DescriptorAllocator ShaderResourceViewAllocator { get; internal set; }

        internal ID3D12Device5 NativeDevice;

        public GraphicsDevice()
        {
            NativeAdapter = new GraphicsAdapter();
            InitializeFromImpl();
        }

        public GraphicsDevice(GraphicsAdapter graphicsAdapter, RenderDescriptor presentation)
        {
            NativeAdapter = graphicsAdapter;
            NativeParameters = presentation;
            InitializeFromImpl();
        }


        public void InitializeFromImpl()
        {
            InitializePlatformDevice();
        }




        private void InitializePlatformDevice()
        {

            NativeDevice = CreateDevice(NativeAdapter);


            SupportedFeature();

            



            // Create the NativeCommandQueue
            NativeDirectCommandQueue = CreateCommandQueueDirect();

            CreateDescriptorAllocators();

        }

        public bool SupportedFeature()
        {
            FeatureDataD3D12Options1 Options1 = NativeDevice.CheckFeatureSupport<FeatureDataD3D12Options1>(Vortice.Direct3D12.Feature.Options1);
            FeatureDataD3D12Options2 Options2 = NativeDevice.CheckFeatureSupport<FeatureDataD3D12Options2>(Vortice.Direct3D12.Feature.Options2);
            FeatureDataD3D12Options3 Options3 = NativeDevice.CheckFeatureSupport<FeatureDataD3D12Options3>(Vortice.Direct3D12.Feature.Options3);
            FeatureDataD3D12Options4 Options4 = NativeDevice.CheckFeatureSupport<FeatureDataD3D12Options4>(Vortice.Direct3D12.Feature.Options4);
            FeatureDataD3D12Options5 Options5 = NativeDevice.CheckFeatureSupport<FeatureDataD3D12Options5>(Vortice.Direct3D12.Feature.Options5);
            FeatureDataD3D12Options6 Options6 = NativeDevice.CheckFeatureSupport<FeatureDataD3D12Options6>(Vortice.Direct3D12.Feature.Options6);
            FeatureDataD3D12Options7 Options7 = NativeDevice.CheckFeatureSupport<FeatureDataD3D12Options7>(Vortice.Direct3D12.Feature.Options7);
            FeatureDataD3D12Options8 Options8 = NativeDevice.CheckFeatureSupport<FeatureDataD3D12Options8>(Vortice.Direct3D12.Feature.D3D12Options8);
            FeatureDataD3D12Options9 Options9 = NativeDevice.CheckFeatureSupport<FeatureDataD3D12Options9>(Vortice.Direct3D12.Feature.D3D12Options9);
            FeatureDataD3D12Options10 Options10 = NativeDevice.CheckFeatureSupport<FeatureDataD3D12Options10>(Vortice.Direct3D12.Feature.D3D12Options10);


            Console.WriteLine($"Int64ShaderOps {Options1.Int64ShaderOps}");
            Console.WriteLine($"ProgrammableSamplePositionsTier {Options2.ProgrammableSamplePositionsTier}");
            Console.WriteLine($"DepthBoundsTestSupported {Options2.DepthBoundsTestSupported}");
            Console.WriteLine($"SamplerFeedbackTier {Options7.SamplerFeedbackTier}");
            Console.WriteLine($"MeshShaderTier {Options7.MeshShaderTier}");
            Console.WriteLine($"UnalignedBlockTexturesSupported {Options8.UnalignedBlockTexturesSupported}");
            Console.WriteLine($"MeshShaderPipelineStatsSupported {Options9.MeshShaderPipelineStatsSupported}");
            Console.WriteLine($"ViewInstancingTier {Options3.ViewInstancingTier}");
            Console.WriteLine($"VariableShadingRateTier {Options6.VariableShadingRateTier}");
            Console.WriteLine($"MSAA64KBAlignedTextureSupported {Options4.MSAA64KBAlignedTextureSupported}");
            Console.WriteLine($"Native16BitShaderOpsSupported {Options4.Native16BitShaderOpsSupported}");
            Console.WriteLine($"Ray {Options5.RaytracingTier}");
            Console.WriteLine($"SRVOnlyTiledResourceTier3 {Options5.SRVOnlyTiledResourceTier3}");
            Console.WriteLine($"VariableRateShadingSumCombinerSupported {Options10.VariableRateShadingSumCombinerSupported}");
            return true;


        }

        internal ID3D12Device5 CreateDevice(GraphicsAdapter factory4)
        {


            foreach (var Adapters in factory4.Adapters)
                if (D3D12CreateDevice<ID3D12Device5>(Adapters, FeatureLevel.Level_12_1, out var device).Success)
                    return device.QueryInterface<ID3D12Device5>();


            return null;
        }

        public void CreateDescriptorAllocators()
        {
            RenderTargetViewAllocator = new DescriptorAllocator(this, DescriptorHeapType.RenderTargetView);

            DepthStencilViewAllocator = new DescriptorAllocator(this, DescriptorHeapType.DepthStencilView);

            ShaderResourceViewAllocator = new DescriptorAllocator(this, DescriptorHeapType.ConstantBufferViewShaderResourceViewUnorderedAccessView);
        }

        public CommandQueue CreateCommandQueueDirect()
        {
            // Describe and create the command queue.
            CommandQueue Queue = new CommandQueue(this)
            {
                Type = CommandListType.Direct
            };

            return Queue;
        }


        public CommandQueue CreateCommandQueueCopy()
        {
            // Describe and create the command queue.
            CommandQueue Queue = new CommandQueue(this)
            {
                Type = CommandListType.Copy
            };

            return Queue;
        }


        public CommandQueue CreateCommandQueueBundle()
        {
            // Describe and create the command queue.
            CommandQueue Queue = new CommandQueue(this)
            {
                Type = CommandListType.Bundle
            };

            return Queue;
        }


        public CommandQueue CreateCommandQueueCompute()
        {
            // Describe and create the command queue.
            CommandQueue Queue = new CommandQueue(this)
            {
                Type = CommandListType.Compute
            };

            return Queue;
        }


        public CommandQueue CreateCommandQueueVideoDecode()
        {
            // Describe and create the command queue.
            CommandQueue Queue = new CommandQueue(this)
            {
                Type = CommandListType.VideoDecode
            };

            return Queue;
        }

        public CommandQueue CreateCommandQueueVideoEncode()
        {
            // Describe and create the command queue.
            CommandQueue Queue = new CommandQueue(this)
            {
                Type = CommandListType.VideoEncode
            };

            return Queue;
        }


        public CommandQueue CreateCommandQueueVideoProcess()
        {
            // Describe and create the command queue.
            CommandQueue Queue = new CommandQueue(this)
            {
                Type = CommandListType.VideoProcess
            };

            return Queue;
        }


        public void Dispose()
        {

        }
    }
}
