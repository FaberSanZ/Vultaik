#pragma once

#include <d3d11.h>
#include <cstdint>

#pragma comment(lib, "d3d11.lib")

namespace Graphics
{
	class Adapter; // Forward declaration
	class Device
	{
	public:
		Device() = default;
		~Device();

		bool Initialize(const Adapter& adapter);
		void Release();

		ID3D11Device* GetDevice() const { return m_Device; }
		ID3D11DeviceContext* GetContext() const { return m_Context; }

	private:
		ID3D11Device* m_Device = nullptr;
		ID3D11DeviceContext* m_Context = nullptr;
	};
}
