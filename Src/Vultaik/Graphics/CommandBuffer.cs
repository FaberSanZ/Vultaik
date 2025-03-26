// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)



using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vultaik;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using Silk.NET.GLFW;
using System.Net.Mail;

namespace Vultaik.Graphics
{
    public enum CommandBufferType
    {
        Graphics,

        Compute,

        Transfer,
    }
    public unsafe class CommandBuffer : IDisposable
    {

        internal VkCommandPool _commandPool;
        internal VkCommandBuffer _commandBuffer;

        public CommandBuffer(Device device, CommandBufferType type)
        {
            Device = device;
            Type = type;
            CreateCommandPool();
            CreateCommandBuffers();
        }


        public Device Device { get; }
        public CommandBufferType Type { get; }

        private void CreateCommandPool()
        {
            //QueueFamilyIndices indices = new QueueFamilyIndices(physicalDevice, surface);

            VkCommandPoolCreateInfo cmd_pool_info = new VkCommandPoolCreateInfo()
            {
                sType = VkStructureType.CommandPoolCreateInfo,
                pNext = null,
                flags = VkCommandPoolCreateFlags.ResetCommandBuffer,
            };

            switch (Type)
            {
                case CommandBufferType.Graphics:
                    cmd_pool_info.queueFamilyIndex = Device.QueueGraphicsFamily;
                    break;
                case CommandBufferType.Compute:
                    cmd_pool_info.queueFamilyIndex = Device.QueueComputeFamily;
                    break;
                case CommandBufferType.Transfer:
                    cmd_pool_info.queueFamilyIndex = Device.QueueTransferFamily;
                    break;
                default:
                    cmd_pool_info.queueFamilyIndex = Device.QueueGraphicsFamily;
                    break;
            }

            vkCreateCommandPool(Device._device, &cmd_pool_info, null, out _commandPool);
        }

        private void CreateCommandBuffers()
        {
            VkCommandBufferAllocateInfo allocInfo = new VkCommandBufferAllocateInfo()
            {
                sType = VkStructureType.CommandBufferAllocateInfo,
                commandPool = _commandPool,
                level = VkCommandBufferLevel.Primary,
                commandBufferCount = 1,
            };

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(Device._device, &allocInfo, &commandBuffer);
            _commandBuffer = commandBuffer;

        }

        public void ResetCommandBuffer()
        {
            vkResetCommandBuffer(_commandBuffer, VkCommandBufferResetFlags.None);
        }



   







        public void BeginRendering(Image ColorImage, Image? depth = null)
        {
            VkImageSubresourceRange imageSubresourceRange = new()
            {
                aspectMask = VkImageAspectFlags.Color,
                baseArrayLayer = 0,
                baseMipLevel = 0,
                layerCount = 1,
                levelCount = 1,
            };

            VkImage image = ColorImage._image;
            VkImageView view = ColorImage._view;
            int width = ColorImage._width;
            int height = ColorImage._height;

            //TODO: sync 
            //insertImageMemoryBarrier(cmd, image, 0, VkAccessFlags.ColorAttachmentWrite, VkImageLayout.Undefined, VkImageLayout.AttachmentOptimal, VkPipelineStageFlags.ColorAttachmentOutput, VkPipelineStageFlags.ColorAttachmentOutput, range);

            VkRenderingAttachmentInfo  colorAttachment = new VkRenderingAttachmentInfo()
            {
                imageView = view,
                imageLayout = VkImageLayout.ColorAttachmentOptimal,
                loadOp = VkAttachmentLoadOp.Clear,
                storeOp = VkAttachmentStoreOp.Store,
            };
            colorAttachment.clearValue.color = new VkClearColorValue(0.0f, 0.2f, 0.4f, 0.0f);

            VkRenderingInfo rendringInfo = new()
            {
                renderArea = new VkRect2D(0, 0, (uint)width, (uint)height),
                layerCount = 1,
                colorAttachmentCount = 1,
                pColorAttachments = &colorAttachment,
                //pDepthAttachment = null
            };


            if (depth is not null)
            {
                //rendring_info.pDepthAttachment
            }

            vkCmdBeginRendering(_commandBuffer, &rendringInfo);





        }

        public void EndRendering()
        {
            //TODO: sync 
            //insertImageMemoryBarrier(cmd, image, VkAccessFlags.ColorAttachmentWrite, 0, VkImageLayout.AttachmentOptimal, VkImageLayout.PresentSrcKHR, VkPipelineStageFlags.ColorAttachmentOutput, VkPipelineStageFlags.None, range);
            vkCmdEndRendering(_commandBuffer);
        }



        public void BeginCommandBuffer()
        {
            VkCommandBufferBeginInfo commandBufferBeginInfo = new()
            {
                sType = VkStructureType.CommandBufferBeginInfo,
                pNext = null,
                flags = VkCommandBufferUsageFlags.None,
                pInheritanceInfo = null
            };

            vkBeginCommandBuffer(_commandBuffer, &commandBufferBeginInfo);
        }


        public void EndCommandBuffer()
        {
            vkEndCommandBuffer(_commandBuffer);
        }






        internal void insertImageMemoryBarrier(VkCommandBuffer cmd_buffer, VkImage image, VkAccessFlags src_access, VkAccessFlags dst_access, VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage, VkImageSubresourceRange range)
        {
            VkImageMemoryBarrier imageMemoryBarrier = new()
            {
                sType = VkStructureType.ImageMemoryBarrier,
                srcAccessMask = src_access,
                dstAccessMask = dst_access,
                oldLayout = old_image_layout,
                newLayout = new_image_layout,
                image = image,
                subresourceRange = range,
            };

            vkCmdPipelineBarrier(cmd_buffer, src_stage, dst_stage, 0, 0, null, 0, null, 1, &imageMemoryBarrier);
        }

        public void Dispose()
        {
            vkDestroyCommandPool(Device._device, _commandPool, null);
        }
    }
}
