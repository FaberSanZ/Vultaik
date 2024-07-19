// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)



using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vultaik;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vultaik.Graphics
{
    public unsafe class CommandList
    {

        internal VkCommandPool commandPool;
        public VkCommandBuffer commandBuffer;

        public CommandList(Device device)
        {
            Device = device;

            CreateCommandPool();
            CreateCommandBuffers();
        }


        public Device Device { get; }




        private void CreateCommandPool()
        {
            //QueueFamilyIndices indices = new QueueFamilyIndices(physicalDevice, surface);

            VkCommandPoolCreateInfo poolInfo = new VkCommandPoolCreateInfo()
            {
                //sType = VkStructureType.CommandPoolCreateInfo,
                queueFamilyIndex = (uint)Device.indices.GraphicsFamily,
                flags = VkCommandPoolCreateFlags.ResetCommandBuffer,
            };

            vkCreateCommandPool(Device.device, &poolInfo, null, out commandPool);
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
            vkAllocateCommandBuffers(Device.device, &allocInfo, &cmd);
            commandBuffer = cmd;

        }

        public void ResetCommandBuffer()
        {
            vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
        }


        public void RecordCommandBuffer(Framebuffer framebuffer, uint imageIndex, VkExtent2D extent)
        {
            VkCommandBufferBeginInfo beginInfo = new();
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = null; // Optional

            vkBeginCommandBuffer(commandBuffer, &beginInfo);


            VkRenderPassBeginInfo renderPassInfo = new();
            renderPassInfo.renderPass = framebuffer.renderPass;
            renderPassInfo.framebuffer = framebuffer.swapChainFramebuffers[imageIndex];
            renderPassInfo.renderArea.offset = new VkOffset2D(0, 0);
            renderPassInfo.renderArea.extent = extent;

            VkClearValue clearColor = new VkClearValue(0.0f, 0.2f, 0.4f, 1.0f);
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;
            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


            vkCmdEndRenderPass(commandBuffer);

            vkEndCommandBuffer(commandBuffer);




        }


    }
}
