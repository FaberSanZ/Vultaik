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
    public unsafe class CommandBuffer
    {

        internal VkCommandPool commandPool;
        public VkCommandBuffer commandBuffer;

        public CommandBuffer(Device device)
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
                queueFamilyIndex = Device.indices.GraphicsFamily!.Value,
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
            vkResetCommandBuffer(commandBuffer, VkCommandBufferResetFlags.None);
        }


        public void BeginRenderPass(RenderPass framebuffer)
        {
            uint imageIndex = framebuffer.SwapChain.ImageIndex;
            VkExtent2D extent = framebuffer.Extent;

            VkClearValue clearColor = new VkClearValue(0.0f, 0.2f, 0.4f, 1.0f);

            VkRenderPassBeginInfo renderPassInfo = new();
            renderPassInfo.renderPass = framebuffer.renderPass;
            renderPassInfo.framebuffer = framebuffer.swapChainFramebuffers[imageIndex];
            renderPassInfo.renderArea.offset = new VkOffset2D(0, 0);
            renderPassInfo.renderArea.extent = extent;
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VkSubpassContents.Inline);

        }


        public void insertImageMemoryBarrier(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkAccessFlags srcAccessMask,
            VkAccessFlags dstAccessMask,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            VkImageSubresourceRange subresourceRange)
        {
            VkImageMemoryBarrier imageMemoryBarrier = new()
            {
                sType = VkStructureType.ImageMemoryBarrier
            };
            imageMemoryBarrier.srcAccessMask = srcAccessMask;
            imageMemoryBarrier.dstAccessMask = dstAccessMask;
            imageMemoryBarrier.oldLayout = oldImageLayout;
            imageMemoryBarrier.newLayout = newImageLayout;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = subresourceRange;

            vkCmdPipelineBarrier(cmdbuffer, srcStageMask, dstStageMask, 0, 0, null, 0, null, 1, &imageMemoryBarrier);
        }


        public void CmdBeginRendering(Image ColorImage)
        {
            VkImageSubresourceRange range = new()
            {
                aspectMask = VkImageAspectFlags.Color,
                baseArrayLayer = 0,
                baseMipLevel = 0,
                layerCount = 1,
                levelCount = 1,
            };

            VkImage image = ColorImage.image;
            VkImageView view = ColorImage.view;
            int width = ColorImage.width;
            int height = ColorImage.height;

            insertImageMemoryBarrier(commandBuffer, image, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, range);

            VkRenderingAttachmentInfo colorAttachment = new VkRenderingAttachmentInfo();
            colorAttachment.imageView = view;
            colorAttachment.imageLayout = VkImageLayout.ColorAttachmentOptimal;
            colorAttachment.loadOp = VkAttachmentLoadOp.Clear;
            colorAttachment.storeOp = VkAttachmentStoreOp.Store;
            colorAttachment.clearValue.color = new VkClearColorValue(0.0f, 0.2f, 0.4f, 0.0f);

            VkRenderingInfo info = new()
            {
                renderArea = new VkRect2D(0, 0, (uint)width, (uint)height),
                layerCount = 1,
                colorAttachmentCount = 1,
                pColorAttachments = &colorAttachment,
                //pDepthAttachment = null
            };

            vkCmdBeginRendering(commandBuffer, &info);

            vkCmdEndRendering(commandBuffer);

            insertImageMemoryBarrier(commandBuffer, image, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VkPipelineStageFlags.None, range);

        }

        public void CmdEndRendering()
        {
            //vkCmdEndRendering(commandBuffer);

        }


        public void EndRenderPass()
        {
            vkCmdEndRenderPass(commandBuffer);
        }


        public void BeginCommandBuffer()
        {
            VkCommandBufferBeginInfo beginInfo = new();
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = null; // Optional

            vkBeginCommandBuffer(commandBuffer, &beginInfo);
        }


        public void EndCommandBuffer()
        {
            vkEndCommandBuffer(commandBuffer);

        }



    }
}
