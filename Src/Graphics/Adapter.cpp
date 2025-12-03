#include "Adapter.h"
#include <iostream>

namespace Graphics
{
	bool Adapter::Initialize(uint32_t gpuIndex)
	{
		HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&m_Factory);
		if (FAILED(hr) || !m_Factory)
		{
			std::wcerr << L"[Adapter] Failed to create DXGI Factory.\n";
			return false;
		}

		hr = m_Factory->EnumAdapters(gpuIndex, &m_Adapter);
		if (FAILED(hr) || !m_Adapter)
		{
			std::wcerr << L"[Adapter] Failed to enumerate adapter 0.\n";
			return false;
		}

		DXGI_ADAPTER_DESC desc;
		hr = m_Adapter->GetDesc(&desc);
		if (FAILED(hr))
		{
			std::wcerr << L"[Adapter] Failed to get adapter description.\n";
			return false;
		}

		m_GpuName = desc.Description;
		m_DedicatedVideoMemory = desc.DedicatedVideoMemory;
		m_DedicatedSystemMemory = desc.DedicatedSystemMemory;
		m_SharedSystemMemory = desc.SharedSystemMemory;

		return true;
	}

	void Adapter::Release()
	{
		if (m_Adapter)
		{
			m_Adapter->Release();
			m_Adapter = nullptr;
		}

		if (m_Factory)
		{
			m_Factory->Release();
			m_Factory = nullptr;
		}
	}

	Adapter::~Adapter()
	{
		Release();
	}
}