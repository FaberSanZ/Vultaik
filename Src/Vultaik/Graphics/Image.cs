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
    public unsafe class Image
    {
        internal VkImage _image;
        internal VkImageView _view;
        internal int _width;
        internal int _height;

        public Image()
        {
            
        }
        public VkFormat Format { get; set; }
    }
}
