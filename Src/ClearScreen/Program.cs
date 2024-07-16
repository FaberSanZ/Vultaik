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



    internal VkCommandPool commandPool;
    internal VkCommandBuffer commandBuffer;


    public override void Initialize()
    {
        adapter = new Adapter();
        surface = new Surface(adapter, Window);
        device = new Device(surface); // new Device(adapter); // compute
        swapChain = new SwapChain(device);
        FramebufferAttachment[] attachment = FramebufferAttachment.FromSwapChain(swapChain);


        framebuffer = new Framebuffer(device, attachment.ToList());

        CreateCommandPool();
        CreateCommandBuffers();

        createSyncObjects();
    }


    private void CreateCommandPool()
    {
        //QueueFamilyIndices indices = new QueueFamilyIndices(physicalDevice, surface);

        VkCommandPoolCreateInfo poolInfo = new VkCommandPoolCreateInfo()
        {
            //sType = VkStructureType.CommandPoolCreateInfo,
            queueFamilyIndex = (uint)device.indices.GraphicsFamily,
            flags = VkCommandPoolCreateFlags.ResetCommandBuffer,
        };

        vkCreateCommandPool(device.device, &poolInfo, null, out commandPool);
    }

    private void CreateCommandBuffers()
    {
        VkCommandBufferAllocateInfo allocInfo = new VkCommandBufferAllocateInfo()
        {
            //sType = VkStructureType.CommandBufferAllocateInfo,
            commandPool = commandPool,
            level = VkCommandBufferLevel.Primary,
            commandBufferCount = 1,
        };
        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(device.device, &allocInfo, &cmd);
        commandBuffer = cmd;

    }


    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo = new();
        //beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = null; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            //throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo = new();
        //renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = framebuffer.renderPass;
        renderPassInfo.framebuffer = framebuffer.swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = new VkOffset2D(0, 0);
        renderPassInfo.renderArea.extent = swapChain.swapChainExtent;

        VkClearValue clearColor = new VkClearValue(0.0f, 0.2f, 0.4f, 1.0f);
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            //throw std::runtime_error("failed to record command buffer!");
        }



    }



    private void createSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo = new VkSemaphoreCreateInfo() { };
        //semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = new VkFenceCreateInfo() { };
        //fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(device.device, &semaphoreInfo, null, out imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device.device, &semaphoreInfo, null, out renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device.device, &fenceInfo, null, out inFlightFence) != VK_SUCCESS)
        {
            //throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }


    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    private void drawFrame()
    {
        vkWaitForFences(device.device, inFlightFence, true, uint.MaxValue);
        vkResetFences(device.device, inFlightFence);


        uint imageIndex;
        vkAcquireNextImageKHR(device.device, swapChain.swapChain, uint.MaxValue, imageAvailableSemaphore, VkFence.Null, &imageIndex);

        vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffer, imageIndex);

        VkSubmitInfo submitInfo = new VkSubmitInfo() { };
        //submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore* waitSemaphores = stackalloc[] { imageAvailableSemaphore };
        VkPipelineStageFlags* waitStages = stackalloc[] { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        VkCommandBuffer* cmd = stackalloc[] { commandBuffer };


        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = cmd;

        VkSemaphore* signalSemaphores = stackalloc[] { renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(device.graphics_queue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
        {
            //throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo = new VkPresentInfoKHR() { };
        //presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR* swapChains = stackalloc[] { swapChain.swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(device.present_queue, &presentInfo);
    }



    public override void Update(TimerTick time)
    {
    }


    public override void Draw(TimerTick time)
    {
        drawFrame();

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


public class QueueData
{
    public QueueData(int family, int index)
    {
        Family = family;
        Index = index;
    }


    public int Family { get; set; }
    public int Index { get; set; }
}



