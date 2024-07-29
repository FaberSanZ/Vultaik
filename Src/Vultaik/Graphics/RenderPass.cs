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
    public unsafe class RenderPass
    {



        public VkRenderPass renderPass;
        public VkFramebuffer[] swapChainFramebuffers;


        public RenderPass(Device device, List<FramebufferAttachment> attachments)
        {
            Device = device;
            ColorAttachments = attachments;
            CreateRenderPass();
            createFramebuffers();
        }

        public Device Device { get; }
        public List<FramebufferAttachment> ColorAttachments { get; set; }

        private void CreateRenderPass()
        {
            var Attachment = new FramebufferAttachment(AttachmentType.Color, VkFormat.B8G8R8A8Unorm, 1920, 1080);

            foreach (var attachment in ColorAttachments)
            {

            }


            VkAttachmentDescription colorAttachment = new VkAttachmentDescription()
            {
                format = Attachment.Format,
                samples = VkSampleCountFlags.Count1,
                loadOp = VkAttachmentLoadOp.Clear,
                storeOp = VkAttachmentStoreOp.Store,
                stencilLoadOp = VkAttachmentLoadOp.DontCare,
                stencilStoreOp = VkAttachmentStoreOp.DontCare,
                initialLayout = VkImageLayout.Undefined,
                finalLayout = VkImageLayout.PresentSrcKHR,
            };

            VkAttachmentReference colorAttachmentRef = new VkAttachmentReference()
            {
                attachment = 0,
                layout = VkImageLayout.ColorAttachmentOptimal,
            };

            VkSubpassDescription subpass = new VkSubpassDescription()
            {
                pipelineBindPoint = VkPipelineBindPoint.Graphics,
                colorAttachmentCount = 1,
                pColorAttachments = &colorAttachmentRef,
            };

            VkSubpassDependency dependency = new VkSubpassDependency()
            {
                srcSubpass = VK_SUBPASS_EXTERNAL,
                srcStageMask = VkPipelineStageFlags.ColorAttachmentOutput,
                srcAccessMask = 0,

                dstSubpass = 0,
                dstStageMask = VkPipelineStageFlags.ColorAttachmentOutput,
                dstAccessMask = VkAccessFlags.ColorAttachmentWrite,
            };

            VkRenderPassCreateInfo renderPassInfo = new VkRenderPassCreateInfo()
            {
                //sType = VkStructureType.RenderPassCreateInfo,
                attachmentCount = 1,
                pAttachments = &colorAttachment,

                subpassCount = 1,
                pSubpasses = &subpass,

                dependencyCount = 1,
                pDependencies = &dependency,
            };

            vkCreateRenderPass(Device.device, &renderPassInfo, null, out VkRenderPass newRenderPass);
            renderPass = newRenderPass;
        }



        private void createFramebuffers()
        {
            swapChainFramebuffers = new VkFramebuffer[ColorAttachments.Count];

            for (int i = 0; i < ColorAttachments.Count; i++)
            {
                VkImageView* attachments = stackalloc VkImageView[] { ColorAttachments[i].Image.view };

                VkFramebufferCreateInfo frameBufferInfo = new VkFramebufferCreateInfo()
                {
                    //sType = VkStructureType.FramebufferCreateInfo,
                    renderPass = renderPass,
                    attachmentCount = 1,
                    pAttachments = attachments,
                    width = (uint)ColorAttachments[i].Width,
                    height = (uint)ColorAttachments[i].Height,
                    layers = 1,
                };

                vkCreateFramebuffer(Device.device, &frameBufferInfo, null, out swapChainFramebuffers[i]).CheckResult();
            }
        }


    }
}
