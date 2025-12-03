#pragma once

#include "Device.h"
#include "Adapter.h"

#include <iostream>

namespace Graphics
{
	bool Device::Initialize(const Adapter& adapter)
	{
		UINT flags = 0;
#if defined(_DEBUG)
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};

		D3D_FEATURE_LEVEL selected;

		HRESULT hr = D3D11CreateDevice(
			adapter.GetAdapter(),
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			flags,
			featureLevels,
			_countof(featureLevels),
			D3D11_SDK_VERSION,
			&m_Device,
			&selected,
			&m_Context
		);

		if (FAILED(hr))
		{
			std::cerr << "[Device] Failed to create D3D11 device from adapter.\n";
			return false;
		}

		std::cout << "[Device] Device created using adapter.\n";
		return true;
	}

	void Device::Release()
	{
		if (m_Context)
		{
			m_Context->Release();
			m_Context = nullptr;
		}
		if (m_Device)
		{
			m_Device->Release();
			m_Device = nullptr;
		}
	}

	Device::~Device()
	{
		Release();
	}
}