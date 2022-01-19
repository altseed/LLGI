#pragma once

#include "../LLGI.CommandList.h"
#include "LLGI.BaseWebGPU.h"

namespace LLGI
{

class CommandListWebGPU : public CommandList
{
	wgpu::Device device_;
	wgpu::CommandBuffer commandBuffer_;
	wgpu::CommandEncoder commandEncorder_;
	wgpu::RenderPassEncoder renderPassEncorder_;
	wgpu::ComputePassEncoder computePassEncorder_;
	wgpu::Sampler samplers_[2][2];

public:
	CommandListWebGPU(wgpu::Device device);

	void Begin() override;

	void End() override;

	void BeginRenderPass(RenderPass* renderPass) override;

	void EndRenderPass() override;

	void Draw(int32_t primitiveCount, int32_t instanceCount) override;

	void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

	void CopyTexture(Texture* src, Texture* dst) override;

	void CopyTexture(
		Texture* src, Texture* dst, const Vec3I& srcPos, const Vec3I& dstPos, const Vec3I& size, int srcLayer, int dstLayer) override;

	const wgpu::CommandBuffer& GetCommandBuffer() const { return commandBuffer_; }
};

} // namespace LLGI