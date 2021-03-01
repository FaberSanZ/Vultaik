using System;
using System.Collections.Generic;
using System.Text;

namespace Renderer
{
    public class PipelineStateDescription
    {
        public ShaderByteCode VertexShader { get; set; }
        public ShaderByteCode PixelShader { get; set; } 
        public ShaderByteCode HullShader { get; set; }
        public ShaderByteCode GeometryShader { get; set; } 
        public ShaderByteCode DomainShader { get; set; }
    }
}
