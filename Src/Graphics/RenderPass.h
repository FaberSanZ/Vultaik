#pragma once

#include <d3d11.h>
#include <cstdint>
#include <stdexcept>

#include "Device.h"
#include "EngineData.h"
#include "Texture.h"

namespace Graphics
{
	class Texture;

	class RenderPass
	{
	public:
		const Texture& GetColorTexture() const { return m_Color; }
		const Texture& GetDepthTexture() const { return m_Depth; }

		//private:
		Texture m_Color;
		Texture m_Depth;
	};

} // namespace Graphics




