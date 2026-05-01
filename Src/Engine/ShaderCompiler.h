#pragma once

#include <Windows.h>
#include <wrl/client.h>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <dxcapi.h>

using Microsoft::WRL::ComPtr;
namespace Core
{
	static void AppendShaderLog(const std::string& line)
	{
		const std::filesystem::path logPath = std::filesystem::temp_directory_path() / "vultaik_dxr.log";
		std::ofstream out(logPath, std::ios::app);
		if (out.is_open())
			out << line << '\n';
	}

	class ShaderCompiler
	{
	public:
		ShaderCompiler()
		{
			DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
			DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
			DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		}

		IDxcBlob* Compile(const std::wstring& shaderPath, const std::wstring& entryPoint, const std::wstring& targetProfile)
		{
			std::wstring resolvedPath = ResolveShaderPath(shaderPath);
			if (resolvedPath.empty())
			{
				OutputDebugStringW((L"Shader not found: " + shaderPath + L"\n").c_str());
				AppendShaderLog("Shader not found: " + std::filesystem::path(shaderPath).string());
				return nullptr;
			}

			// Cargar archivo
			IDxcBlobEncoding* source = nullptr;
			HRESULT loadHr = utils->LoadFile(resolvedPath.c_str(), nullptr, &source);
			if (FAILED(loadHr) || !source)
			{
				OutputDebugStringW((L"Failed to load shader: " + resolvedPath + L"\n").c_str());
				AppendShaderLog("Failed to load shader: " + std::filesystem::path(resolvedPath).string());
				return nullptr;
			}

			// Crear include handler
			IDxcIncludeHandler* includeHandler = nullptr;
			utils->CreateDefaultIncludeHandler(&includeHandler);

			// Argumentos b?sicos
			LPCWSTR args[] =
			{
				resolvedPath.c_str(),
				L"-E", entryPoint.c_str(),
				L"-T", targetProfile.c_str(),
				L"-Zi", L"-Qembed_debug", // Debug info embebida
				L"-Od" // Sin optimizaci?n
			};


			// Compilar
			ComPtr<IDxcOperationResult> result;
			HRESULT hr = compiler->Compile(
				source,
				resolvedPath.c_str(),
				entryPoint.c_str(),
				targetProfile.c_str(),
				args, _countof(args),
				nullptr, 0,
				includeHandler,
				&result
			);



			HRESULT status;
			result->GetStatus(&status);
			if (FAILED(status))
			{
				IDxcBlobEncoding* errors = nullptr;
				result->GetErrorBuffer(&errors);
				if (errors)
				{
					OutputDebugStringA((char*)errors->GetBufferPointer());
					AppendShaderLog(std::string("DXC error: ") + (char*)errors->GetBufferPointer());
				}
				else
				{
					AppendShaderLog("DXC error: unknown");
				}
				return nullptr;
			}

			IDxcBlob* shaderBlob = nullptr;
			result->GetResult((IDxcBlob**)&shaderBlob);
			return shaderBlob;
		}

		private:
		std::wstring ResolveShaderPath(const std::wstring& shaderPath)
		{
			std::filesystem::path input(shaderPath);
			if (std::filesystem::exists(input))
				return input.wstring();

			wchar_t modulePath[MAX_PATH]{};
			GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
			std::filesystem::path base = std::filesystem::path(modulePath).parent_path();

			std::filesystem::path candidate = (base / input).lexically_normal();
			if (std::filesystem::exists(candidate))
				return candidate.wstring();

			std::filesystem::path parent = base;
			for (int i = 0; i < 4; ++i)
			{
				parent = parent.parent_path();
				candidate = (parent / input).lexically_normal();
				if (std::filesystem::exists(candidate))
					return candidate.wstring();
			}

			return {};
		}

		IDxcCompiler* compiler = nullptr;
		IDxcLibrary* library = nullptr;
		IDxcUtils* utils = nullptr;
	};


}
