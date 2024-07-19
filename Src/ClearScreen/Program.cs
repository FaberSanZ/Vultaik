// See https://aka.ms/new-console-template for more information

using Vortice.Vulkan;
using Vultaik;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vultaik.Graphics;

using var App = new Sample();
App.Run();


public unsafe class Sample : Application, IDisposable
{


    Adapter adapter;
    Surface surface;
    Device device;
    SwapChain swapChain;
    Framebuffer framebuffer;
    CommandList command;





    public override void Initialize()
    {
        adapter = new Adapter();
        surface = new Surface(adapter, Window);
        device = new Device(surface); // new Device(adapter); // compute
        swapChain = new SwapChain(device);

        FramebufferAttachment[] attachment = FramebufferAttachment.FromSwapChain(swapChain);
        framebuffer = new Framebuffer(device, attachment.ToList());

        command = new CommandList(device);

    }




    public override void Update(TimerTick time)
    {
    }


    public override void Draw(TimerTick time)
    {
        device.ResetFences();


        uint imageIndex = swapChain.AcquireNextImage();

        command.ResetCommandBuffer();
        command.RecordCommandBuffer(framebuffer, imageIndex, swapChain.swapChainExtent);


        device.Submit(command);


        swapChain.Present(imageIndex);

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





