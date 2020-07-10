
#pragma once

#include "../LLGI.Base.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <system_error>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace LLGI
{

#define SHOW_DX12_ERROR(x)                                                                                                                 \
	if (FAILED(x))                                                                                                                         \
	{                                                                                                                                      \
		auto msg = (std::string("Error : ") + std::string(__FILE__) + " : " + std::to_string(__LINE__) + std::string(" : ") +              \
					std::system_category().message(hr));                                                                                   \
		::LLGI::Log(::LLGI::LogType::Error, msg.c_str());                                                                                  \
	}

class GraphicsDX12;
class RenderPassDX12;
class SingleFrameMemoryPoolDX12;

ID3D12Resource* CreateResourceBuffer(ID3D12Device* device,
									 D3D12_HEAP_TYPE heapType,
									 DXGI_FORMAT format,
									 D3D12_RESOURCE_DIMENSION resourceDimention,
									 D3D12_RESOURCE_STATES resourceState,
									 D3D12_RESOURCE_FLAGS flags,
									 Vec2I size,
									 int32_t samplingCount);

DXGI_FORMAT ConvertFormat(TextureFormatType format);

TextureFormatType ConvertFormat(DXGI_FORMAT format);

namespace DirectX12
{

DXGI_FORMAT GetGeneratedFormat(DXGI_FORMAT format);

DXGI_FORMAT GetShaderResourceViewFormat(DXGI_FORMAT format);

int32_t GetNodeMask();

void SetNodeMask(int nodeMask);

} // namespace DirectX12

} // namespace LLGI
