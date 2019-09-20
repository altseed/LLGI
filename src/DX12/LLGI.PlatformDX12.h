
#pragma once

#include "../LLGI.Platform.h"
#include "LLGI.BaseDX12.h"

#ifdef _WIN32
#include "../Win/LLGI.WindowWin.h"
#endif

namespace LLGI
{

class PlatformDX12 : public Platform
{
private:
	static const int SwapBufferCount = 2;
	WindowWin window;

	ID3D12Device* device = nullptr;
	IDXGIFactory4* dxgiFactory = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12Fence* fence = nullptr;
	HANDLE fenceEvent = nullptr;
	IDXGISwapChain3* swapChain = nullptr;

	ID3D12DescriptorHeap* descriptorHeapRTV = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE handleRTV[SwapBufferCount];
	ID3D12Resource* RenderPass[SwapBufferCount];

	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12GraphicsCommandList* commandListStart = nullptr;
	ID3D12GraphicsCommandList* commandListPresent = nullptr;
	UINT64 fenceValue = 1;

	int32_t frameIndex = 0;

	void Wait();

public:
	PlatformDX12();
	virtual ~PlatformDX12();

	bool Initialize(Vec2I windowSize);

	bool NewFrame() override;
	void Present() override;
	Graphics* CreateGraphics() override;

	ID3D12Device* GetDevice();

	DeviceType GetDeviceType() const override { return DeviceType::DirectX12; }

	ID3D12Resource* GetSwapBuffer(int index);
};

} // namespace LLGI
