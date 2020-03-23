using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using Vortice.Direct3D;
using Vortice.Direct3D12;
using Vortice.Direct3D12.Debug;
using Vortice.Dxc;
using Vortice.DXGI;
using Zeckoxe.Desktop;
using static Vortice.Direct3D12.D3D12;
using static Vortice.DXGI.DXGI;

namespace _02__InitializingDirectX12
{
    public static class ShaderCompiler
    {
        public static ShaderBytecode Compile(DxcShaderStage shaderStage, string source, string entryPoint, string sourceName = "")
        {
            return Compile(shaderStage, File.ReadAllText(source), entryPoint, sourceName, new DxcShaderModel(6, 1));
        }

        public static byte[] Compile(DxcShaderStage shaderStage, string source, string entryPoint, string sourceName, DxcShaderModel shaderModel)
        {
            return Compile(shaderStage, source, entryPoint, sourceName, new DxcCompilerOptions
            {
                ShaderModel = shaderModel,
                //PackMatrixInRowMajor = true,
                //GenerateSPIRV = true,
                //DisableOptimizations = true,
            });
        }

        public static byte[] Compile(DxcShaderStage shaderStage, string source, string entryPoint, string sourceName, DxcCompilerOptions options)
        {
            return Dxc.GetBytesFromBlob(DxcCompiler.Compile(shaderStage, source, entryPoint, sourceName, options).GetResult());
        }
    }
    public class D3D12Graphics : IDisposable
    {
        public Window Window { get; set; }



        // direct3d stuff
        internal const int frameBufferCount = 3; // number of buffers we want, 2 for double buffering, 3 for tripple buffering

        internal ID3D12Device device; // direct3d device

        internal IDXGISwapChain3 swapChain; // swapchain used to switch between render targets

        internal ID3D12CommandQueue commandQueue; // container for command lists

        internal ID3D12DescriptorHeap rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets

        internal ID3D12Resource[] renderTargets = new ID3D12Resource[frameBufferCount]; // number of render targets equal to buffer count

        internal ID3D12CommandAllocator[] commandAllocator = new ID3D12CommandAllocator[frameBufferCount]; // we want enough allocators for each buffer * number of threads (we only have one thread)

        internal ID3D12GraphicsCommandList commandList; // a command list we can record commands into, then execute them to render the frame

        internal ID3D12Fence[] fence = new ID3D12Fence[frameBufferCount];    // an object that is locked while our command list is being executed by the gpu. We need as many 
                                                                             //as we have allocators (more if we want to know when the gpu is finished with an asset)

        internal AutoResetEvent fenceEvent; // a handle to an event when our fence is unlocked by the gpu

        internal int[] fenceValue = new int[frameBufferCount]; // this value is incremented each frame. each fence will have its own value

        internal int frameIndex; // current rtv we are on

        internal int rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)



        public D3D12Graphics()
        {
            // create the window
            InitializeWindow();

            // initialize direct3d
            InitD3D();

            // start the main loop
            Window.RenderLoop(Mainloop);


            // we want to wait for the gpu to finish executing the command list before we start releasing everything
            WaitForPreviousFrame();



            // clean up everything
            Cleanup();
        }


        public void InitializeWindow()
        {
            Window = new Window(string.Empty, 800, 600);
            Window?.Show();
        }

        public void InitD3D()
        {
            List<IDXGIAdapter> Adapters = new List<IDXGIAdapter>();
            IDXGIFactory4 Factory;
            D3D12GetDebugInterface(out ID3D12Debug D3D12Debug);

            D3D12Debug.EnableDebugLayer();


            if (CreateDXGIFactory2(true, out IDXGIFactory4 tempDXGIFactory4).Failure)
            {
                throw new InvalidOperationException("Cannot create IDXGIFactory4");
            }

            Factory = tempDXGIFactory4;
            foreach (IDXGIAdapter1 adapter in tempDXGIFactory4.Adapters1)
            {
                AdapterDescription1 desc = adapter.Description1;

                // Don't select the Basic Render Driver adapter.
                if ((desc.Flags & AdapterFlags.Software) != AdapterFlags.None)
                {
                    continue;
                }

                Adapters.Add(adapter);
            }



            foreach (IDXGIAdapter Adapter in Adapters)
            {
                if (D3D12CreateDevice(Adapter, FeatureLevel.Level_11_1, out ID3D12Device dev).Success)
                {
                    device = dev;
                }
            }

            CommandQueueDescription cqDesc = new CommandQueueDescription()
            {
                Type = CommandListType.Direct,
                NodeMask = 0,
                Priority = 0,
                Flags = CommandQueueFlags.None
            };

            commandQueue = device.CreateCommandQueue(cqDesc);





            /* Swapchain */
            SwapChainDescription1 SwapchainDesc = new SwapChainDescription1
            {
                BufferCount = frameBufferCount,
                Width = Window.Width,
                Height = Window.Height,
                Format = Format.R8G8B8A8_UNorm,
                Usage = Usage.RenderTargetOutput,
                SwapEffect = SwapEffect.FlipDiscard,
                SampleDescription = new SampleDescription(1, 0)
            };

            IDXGISwapChain1 SwapchainTemp = Factory.CreateSwapChainForHwnd(commandQueue, Window.Win32Handle, SwapchainDesc);


            swapChain = SwapchainTemp.QueryInterface<IDXGISwapChain3>();

            frameIndex = swapChain.GetCurrentBackBufferIndex();



            // -- Create the Back Buffers (render target views) Descriptor Heap -- //


            DescriptorHeapDescription rtvHeapDesc = new DescriptorHeapDescription()
            {
                Flags = DescriptorHeapFlags.None,
                DescriptorCount = frameBufferCount,
                Type = DescriptorHeapType.RenderTargetView,
                NodeMask = 0,
            };

            rtvDescriptorHeap = device.CreateDescriptorHeap(rtvHeapDesc);


            // get the size of a descriptor in this heap (this is a rtv heap, so only rtv descriptors should be stored in it.
            // descriptor sizes may vary from device to device, which is why there is no set size and we must ask the 
            // device to give us the size. we will use this size to increment a descriptor handle offset
            rtvDescriptorSize = device.GetDescriptorHandleIncrementSize(DescriptorHeapType.RenderTargetView);



            CpuDescriptorHandle rtvHandle = rtvDescriptorHeap.GetCPUDescriptorHandleForHeapStart();



            for (int i = 0; i < frameBufferCount; i++)
            {
                // first we get the n'th buffer in the swap chain and store it in the n'th
                // position of our ID3D12Resource array
                renderTargets[i] = swapChain.GetBuffer<ID3D12Resource>(i);


                // the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
                device.CreateRenderTargetView(renderTargets[i], null, rtvHandle);

                // we increment the rtv handle by the rtv descriptor size we got above
                rtvHandle.Ptr += /*FrameIndex **/ rtvDescriptorSize;
            }



            // -- Create the Command Allocators -- //

            for (int i = 0; i < frameBufferCount; i++)
            {
                commandAllocator[i] = device.CreateCommandAllocator(CommandListType.Direct);
            }



            // create the command list with the first allocator
            ID3D12PipelineState InitialState = null;
            commandList = device.CreateCommandList(CommandListType.Direct, commandAllocator[0], InitialState);


            // command lists are created in the recording state. our main loop will set it up for recording again so close it now
            commandList.Close();

            // -- Create a Fence & Fence Event -- //

            // create the fences
            for (int i = 0; i < frameBufferCount; i++)
            {
                fence[i] = device.CreateFence(0, FenceFlags.None);

                fenceValue[i] = 0; // set the initial fence value to 0
            }


            fenceEvent = new AutoResetEvent(false);
        }


        public void UpdatePipeline()
        {
            // We have to wait for the gpu to finish with the command allocator before we reset it
            WaitForPreviousFrame();

            // we can only reset an allocator once the gpu is done with it
            // resetting an allocator frees the memory that the command list was stored in
            commandAllocator[frameIndex].Reset();


            // reset the command list. by resetting the command list we are putting it into
            // a recording state so we can start recording commands into the command allocator.
            // the command allocator that we reference here may have multiple command lists
            // associated with it, but only one can be recording at any time. Make sure
            // that any other command lists associated to this command allocator are in
            // the closed state (not recording).
            // Here you will pass an initial pipeline state object as the second parameter,
            // but in this tutorial we are only clearing the rtv, and do not actually need
            // anything but an initial default pipeline, which is what we get by setting
            // the second parameter to NULL
            commandList.Reset(commandAllocator[frameIndex]);

            // here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

            // transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here

            commandList.ResourceBarrier(new ResourceBarrier(new ResourceTransitionBarrier(renderTargets[frameIndex], ResourceStates.Present, ResourceStates.RenderTarget)));

            // here we again get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
            CpuDescriptorHandle rtvHandle = rtvDescriptorHeap.GetCPUDescriptorHandleForHeapStart();
            rtvHandle.Ptr += frameIndex * device.GetDescriptorHandleIncrementSize(DescriptorHeapType.RenderTargetView);

            // set the render target for the output merger stage (the output of the pipeline)
            commandList.OMSetRenderTargets(rtvHandle);

            // Clear the render target by using the ClearRenderTargetView command
            commandList.ClearRenderTargetView(rtvHandle, System.Drawing.Color.DarkRed);

            // transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a
            // warning if present is called on the render target when it's not in the present state
            commandList.ResourceBarrier(new ResourceBarrier(new ResourceTransitionBarrier(renderTargets[frameIndex], ResourceStates.RenderTarget, ResourceStates.Present)));

            commandList.Close();

        }

        private void WaitForPreviousFrame()
        {


            // swap the current rtv buffer index so we draw on the correct buffer
            frameIndex = swapChain.GetCurrentBackBufferIndex();

            // if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
            // the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
            if (fence[frameIndex].CompletedValue < fenceValue[frameIndex])
            {
                // we have the fence create an event which is signaled once the fence's current value is "fenceValue"
                fence[frameIndex].SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);


                // We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
                // has reached "fenceValue", we know the command queue has finished executing
                fenceEvent.WaitOne();
            }

            // increment fenceValue for next frame
            fenceValue[frameIndex]++;
        }

        private void Mainloop()
        {
            UpdatePipeline(); // update the pipeline by sending commands to the commandqueue

            // create an array of command lists (only one command list here)
            ID3D12CommandList[] ppCommandLists = { commandList };

            // execute the array of command lists
            commandQueue.ExecuteCommandLists(ppCommandLists);

            // this command goes in at the end of our command queue. we will know when our command queue 
            // has finished because the fence value will be set to "fenceValue" from the GPU since the command
            // queue is being executed on the GPU
            commandQueue.Signal(fence[frameIndex], fenceValue[frameIndex]);


            // present the current backbuffer
            swapChain.Present(0, 0);
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
