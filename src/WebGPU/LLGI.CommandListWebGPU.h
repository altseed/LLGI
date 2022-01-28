#pragma once

#include "LLGI.BaseWebGPU.h"
#include "../LLGI.CommandList.h"

namespace LLGI
{

class CommandListWebGPU : public CommandList
{
    wgpu::CommandBuffer commandBuffer_;
    wgpu::RenderPassEncoder renderPassEncorder_;
    wgpu::ComputePassEncoder computePassEncorder_;
public:
    void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;
};

}