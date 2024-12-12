// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)



using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Vortice.Vulkan;

namespace Vultaik.Graphics
{
    public class FramebufferAttachment
    {
        public AttachmentType Type { get; set; }
        public Image Image { get; set; }
        public VkFormat Format { get; set; } 
        public int Width { get; set; }
        public int Height { get; set; }
        public VkSampleCountFlags Samples { get; set; }

        
        public FramebufferAttachment(AttachmentType type, VkFormat format, int width, int height, VkSampleCountFlags samples = VkSampleCountFlags.Count1)
        {
            Type = type;
            Format = format;
            Width = width;
            Height = height;
            Samples = samples;
        }



        public static List<FramebufferAttachment> FromSwapChain(SwapChain swapChain)
        {
            List<FramebufferAttachment> attachment = new();


            for (int i = 0; i < swapChain.SwapChainImages.Length; i++)
            {
                attachment.Add(new(AttachmentType.Color, swapChain.swapChainImageFormat, 800, 600)
                {
                    Image = swapChain.SwapChainImages[i],
                    Format = swapChain.swapChainImageFormat,
                });
            }


            return attachment;
        }
    }
}
