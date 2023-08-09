#pragma once

#include "../LLGI.Platform.h"
#include "LLGI.BaseWebGPU.h"

namespace LLGI
{

class PlatformWebGPU : public Platform
{
private:
	wgpu::SwapChain swapchain_;
	wgpu::Device device_;

public:
	int GetCurrentFrameIndex() const override;
	int GetMaxFrameCount() const override;

	bool NewFrame() override;
	void Present() override;
	Graphics* CreateGraphics() override;
};

} // namespace LLGI