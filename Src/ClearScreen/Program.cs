using System.Numerics;
using System.Runtime.CompilerServices;
using Vultaik;
using Vultaik.Graphics;

using var App = new Sample(true);
App.Run();



public unsafe class Sample(bool debug) : Application, IDisposable
{


    public Adapter adapter;
    public Surface surface;
    public Device device;
    public SwapChain swapChain;
    public CommandBuffer command;





    public override void Initialize()
    {
        adapter = new Adapter(debug);
        surface = new Surface(adapter, Window);
        device = new Device(surface); // new Device(adapter); // compute
        swapChain = new SwapChain(device);
        command = new CommandBuffer(device);
    }




    public override void Update(TimerTick time)
    {
    }


    public override void Draw(TimerTick time)
    {
        device.ResetFences();

        command.ResetCommandBuffer();
        command.BeginCommandBuffer();


        command.CmdBeginRendering(swapChain.ColorImage);

        // draw

        command.CmdEndRendering();
        command.EndCommandBuffer();


        device.Submit(command);


        swapChain.Present();

    }

    public void Dispose()
    {
        //vkDeviceWaitIdle(device);



        //vkDestroySemaphore(device, renderFinishedSemaphore, null);
        //vkDestroySemaphore(device, imageAvailableSemaphore, null);
        //vkDestroyFence(device, inFlightFence, null);

        //vkDestroyCommandPool(device, commandPool, null);

        //foreach (VkFramebuffer frame in swapChainFramebuffers)
        //    vkDestroyFramebuffer(device, frame, null);

        //vkDestroyRenderPass(device, renderPass, null);


        //foreach (VkImageView view in swapChainImageViews)
        //    vkDestroyImageView(device, view, null);


        //if (swapChain != VkSwapchainKHR.Null)
        //    vkDestroySwapchainKHR(device, swapChain);

        //if (device != VkDevice.Null)
        //    vkDestroyDevice(device, null);


        //if (debugMessenger != VkDebugUtilsMessengerEXT.Null)
        //    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger);


        //if (surface != VkSurfaceKHR.Null)
        //    vkDestroySurfaceKHR(instance, surface);

        //if (instance != VkInstance.Null)
        //    vkDestroyInstance(instance);


        Window.Dispose();

    }
}





