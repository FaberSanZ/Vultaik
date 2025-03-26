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
    public class Pipeline
    {
        private VkPipeline _pipeline;
        private VkPipelineLayout _pipelineLayout;


        public Pipeline(int data)
        {
            byte[] code1 = new byte[128];   
            byte[] code2 = new byte[128];

            string vs = "VS"; 
            string ps = "PS";
            string datar = code1 + vs + ps;
        }
    }
}
