#pragma once

#include <dxgi.h>
#include <string>

#pragma comment(lib, "dxgi.lib")

namespace Graphics
{
	class Adapter
	{
	public:
		Adapter() = default;
		~Adapter();

		bool Initialize(uint32_t gpuIndex);
		void Release();

		const std::wstring& GetGpuName() const { return m_GpuName; }
		uint64_t GetDedicatedVideoMemory() const { return m_DedicatedVideoMemory; }
		uint64_t GetDedicatedSystemMemory() const { return m_DedicatedSystemMemory; }
		uint64_t GetSharedSystemMemory() const { return m_SharedSystemMemory; }
		IDXGIAdapter* GetAdapter() const { return m_Adapter; }
	private:
		std::wstring m_GpuName = L"Unknown GPU";
		uint64_t m_DedicatedVideoMemory = 0;
		uint64_t m_DedicatedSystemMemory = 0;
		uint64_t m_SharedSystemMemory = 0;

		IDXGIAdapter* m_Adapter = nullptr;
		IDXGIFactory* m_Factory = nullptr;
	};
}