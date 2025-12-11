#pragma once
#include <cmath>
#include <DirectXMath.h>


namespace Games
{

	class GameMath
	{
	public:
		static inline float DegreesToRadians(float degrees)
		{
			return degrees * (PI / 180.0f);
		}

		static constexpr float PI = 3.141592654f;
		static constexpr float PI2 = 6.283185307f;
		static constexpr float PIDIV2 = 1.570796327f;
		static constexpr float PIDIV4 = 0.785398163f;


		static constexpr uint32_t Select0 = DirectX::XM_SELECT_0;
		static constexpr uint32_t Select1 = DirectX::XM_SELECT_1;
	};

	struct Float3
	{
		float x;
		float y;
		float z;
		constexpr Float3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}

	};

}