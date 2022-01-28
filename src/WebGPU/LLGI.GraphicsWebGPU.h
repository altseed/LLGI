#pragma once

#include "../LLGI.Base.h"
#include "../LLGI.Graphics.h"
#include "LLGI.BaseWebGPU.h"

namespace LLGI
{

class GraphicsWebGPU : public Graphics
{
private:
	wgpu::Device device_;
	wgpu::Queue queue_;

public:
	wgpu::Device& GetDevice() { return device_; }
	wgpu::Queue& GetQueue() { return queue_; }
};

} // namespace LLGI