
#pragma once

#include "../LLGI.G3.Platform.h"
#include "LLGI.G3.BaseDX12.h"

namespace LLGI
{
namespace G3
{
	
class PlatformDX12
	: public Platform
{
private:
	static const int SwapBufferCount = 2;

	HWND hwnd;
	ID3D12Device* device = nullptr;
	IDXGIFactory4* dxgiFactory = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12Fence* fence = nullptr;
	HANDLE fenceEvent = nullptr;
	IDXGISwapChain3* swapChain = nullptr;

	ID3D12DescriptorHeap* descriptorHeapRTV = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE handleRTV[SwapBufferCount];
	ID3D12Resource* renderTarget[SwapBufferCount];

	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12GraphicsCommandList* commandListStart = nullptr;
	ID3D12GraphicsCommandList* commandListPresent = nullptr;
	UINT64 fenceValue = 0;

	int32_t frameIndex = 0;

	void Wait();

public:
	PlatformDX12();
	virtual ~PlatformDX12();

	bool Initialize(Vec2I windowSize);

	void NewFrame() override;
	void Present() override;
	Graphics* CreateGraphics() override;

	ID3D12Device* GetDevice();
};

}
}