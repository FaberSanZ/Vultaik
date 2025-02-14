﻿// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
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
    public unsafe class CommandBuffer
    {

        internal VkCommandPool command_pool;
        internal VkCommandBuffer cmd;

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
                    cmd_pool_info.queueFamilyIndex = Device.QueueGraphicsFamily!.Value;
                    break;
                case CommandBufferType.Compute:
                    //cmd_pool_info.queueFamilyIndex = Device.indices.Compute!.Value;
                    break;
                case CommandBufferType.Transfer:
                    //cmd_pool_info.queueFamilyIndex = Device.indices.Transfer!.Value;
                    break;
                default:
                    cmd_pool_info.queueFamilyIndex = Device.QueueGraphicsFamily!.Value;
                    break;
            }
            vkCreateCommandPool(Device.device, &cmd_pool_info, null, out command_pool);
        }

        private void CreateCommandBuffers()
        {
            VkCommandBufferAllocateInfo allocInfo = new VkCommandBufferAllocateInfo()
            {
                //sType = VkStructureType.CommandBufferAllocateInfo,
                commandPool = command_pool,
                level = VkCommandBufferLevel.Primary,
                commandBufferCount = 1,
            };
            VkCommandBuffer cmd;
            vkAllocateCommandBuffers(Device.device, &allocInfo, &cmd);
            this.cmd = cmd;

        }

        public void ResetCommandBuffer()
        {
            vkResetCommandBuffer(cmd, VkCommandBufferResetFlags.None);
        }



   







        public void BeginRendering(Image ColorImage, Image? depth = null)
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

            //TODO: sync 
            //insert_image_memory_Barrier(cmd, image, 0, VkAccessFlags.ColorAttachmentWrite, VkImageLayout.Undefined, VkImageLayout.AttachmentOptimal, VkPipelineStageFlags.ColorAttachmentOutput, VkPipelineStageFlags.ColorAttachmentOutput, range);

            VkRenderingAttachmentInfo color_attachment = new VkRenderingAttachmentInfo()
            {
                imageView = view,
                imageLayout = VkImageLayout.ColorAttachmentOptimal,
                loadOp = VkAttachmentLoadOp.Clear,
                storeOp = VkAttachmentStoreOp.Store,
            };
            color_attachment.clearValue.color = new VkClearColorValue(0.0f, 0.2f, 0.4f, 0.0f);

            VkRenderingInfo rendring_info = new()
            {
                renderArea = new VkRect2D(0, 0, (uint)width, (uint)height),
                layerCount = 1,
                colorAttachmentCount = 1,
                pColorAttachments = &color_attachment,
                //pDepthAttachment = null
            };


            if (depth is not null)
            {
                //rendring_info.pDepthAttachment
            }

            vkCmdBeginRendering(cmd, &rendring_info);



            //TODO: sync 
            //insert_image_memory_Barrier(cmd, image, VkAccessFlags.ColorAttachmentWrite, 0, VkImageLayout.AttachmentOptimal, VkImageLayout.PresentSrcKHR, VkPipelineStageFlags.ColorAttachmentOutput, VkPipelineStageFlags.None, range);

        }

        public void EndRendering()
        {
            vkCmdEndRendering(cmd);
        }



        public void BeginCommandBuffer()
        {
            VkCommandBufferBeginInfo beginInfo = new();
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = null; // Optional

            vkBeginCommandBuffer(cmd, &beginInfo);
        }


        public void EndCommandBuffer()
        {
            vkEndCommandBuffer(cmd);

        }






        internal void insert_image_memory_Barrier(VkCommandBuffer cmd_buffer, VkImage image, VkAccessFlags src_access, VkAccessFlags dst_access, VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage, VkImageSubresourceRange range)
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

    }
}
