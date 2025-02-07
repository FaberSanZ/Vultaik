// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)



using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vultaik;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using System.Xml.Linq;


namespace Vultaik.Graphics
{
    public unsafe class RenderPass
    {
        public Device Device { get; }
        public List<FramebufferAttachment> ColorAttachments { get; set; }
        public SwapChain SwapChain { get; }
        public VkExtent2D Extent { get; set; }
    }
}
