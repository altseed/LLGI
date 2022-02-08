#include "LLGI.CommandListWebGPU.h"
#include "LLGI.RenderPassWebGPU.h"
#include "LLGI.TextureWebGPU.h"

namespace LLGI
{
void CommandListWebGPU::Begin()
{
	wgpu::CommandEncoderDescriptor desc = {};
	commandEncorder_ = device_.CreateCommandEncoder(&desc);

	CommandList::Begin();
}

void CommandListWebGPU::End()
{
	commandBuffer_ = commandEncorder_.Finish();
	commandEncorder_ = nullptr;

	CommandList::End();
}

void CommandListWebGPU::BeginRenderPass(RenderPass* renderPass)
{
	auto rp = static_cast<RenderPassWebGPU*>(renderPass);
	const auto& desc = rp->GetDescriptor();

	renderPassEncorder_ = commandEncorder_.BeginRenderPass(&desc);

	// TODO
	// Set render targets
	throw "Not implemented";

	CommandList::BeginRenderPass(renderPass);
}

void CommandListWebGPU::EndRenderPass()
{
	if (renderPassEncorder_ != nullptr)
	{
		renderPassEncorder_.EndPass();
		renderPassEncorder_ = nullptr;
	}

	throw "Not implemented";
}

void CommandListWebGPU::Draw(int32_t primitiveCount, int32_t instanceCount) { throw "Not implemented"; }

void CommandListWebGPU::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
	renderPassEncorder_.SetScissorRect(x, y, width, height);
}

void CommandListWebGPU::CopyTexture(Texture* src, Texture* dst)
{
	auto srcTex = static_cast<TextureWebGPU*>(src);
	CopyTexture(src, dst, {0, 0, 0}, {0, 0, 0}, srcTex->GetParameter().Size, 0, 0);
}

void CommandListWebGPU::CopyTexture(
	Texture* src, Texture* dst, const Vec3I& srcPos, const Vec3I& dstPos, const Vec3I& size, int srcLayer, int dstLayer)
{
	if (isInRenderPass_)
	{
		Log(LogType::Error, "Please call CopyTexture outside of RenderPass");
		return;
	}

	auto srcTex = static_cast<TextureWebGPU*>(src);
	auto dstTex = static_cast<TextureWebGPU*>(dst);

	wgpu::ImageCopyTexture srcTexCopy;
	wgpu::ImageCopyTexture dstTexCopy;
	wgpu::Extent3D extend3d;

	srcTexCopy.texture = srcTex->GetTexture();
	srcTexCopy.origin = {static_cast<uint32_t>(srcPos.X), static_cast<uint32_t>(srcPos.Y), static_cast<uint32_t>(srcLayer + srcPos.Z)};

	dstTexCopy.texture = dstTex->GetTexture();
	dstTexCopy.origin = {static_cast<uint32_t>(dstPos.X), static_cast<uint32_t>(dstPos.Y), static_cast<uint32_t>(dstLayer + dstPos.Z)};

	extend3d.width = size.X;
	extend3d.height = size.Y;
	extend3d.depthOrArrayLayers = size.Z;

	commandEncorder_.CopyTextureToTexture(&srcTexCopy, &dstTexCopy, &extend3d);
}

} // namespace LLGI