#include "LLGI.CommandListWebGPU.h"
#include "LLGI.BufferWebGPU.h"
#include "LLGI.PipelineStateWebGPU.h"
#include "LLGI.RenderPassWebGPU.h"
#include "LLGI.TextureWebGPU.h"

namespace LLGI
{

CommandListWebGPU::CommandListWebGPU(wgpu::Device device)
{
	for (int w = 0; w < 2; w++)
	{
		for (int f = 0; f < 2; f++)
		{
			std::array<wgpu::FilterMode, 2> filters;
			filters[0] = wgpu::FilterMode::Nearest;
			filters[1] = wgpu::FilterMode::Linear;

			std::array<wgpu::AddressMode, 2> am;
			am[0] = wgpu::AddressMode::ClampToEdge;
			am[1] = wgpu::AddressMode::Repeat;

			wgpu::SamplerDescriptor samplerDesc;

			samplerDesc.magFilter = filters[f];
			samplerDesc.minFilter = filters[f];
			samplerDesc.maxAnisotropy = 1;
			samplerDesc.addressModeU = am[w];
			samplerDesc.addressModeV = am[w];
			samplerDesc.addressModeW = am[w];
			samplers_[w][f] = device.CreateSampler(&samplerDesc);
		}
	}
}

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
	renderPassEncorder_.SetViewport(0, 0, rp->GetScreenSize().X, rp->GetScreenSize().Y, 0.0f, 1.0f);

	CommandList::BeginRenderPass(renderPass);
}

void CommandListWebGPU::EndRenderPass()
{
	if (renderPassEncorder_ != nullptr)
	{
		renderPassEncorder_.End();
		renderPassEncorder_ = nullptr;
	}
	CommandList::EndRenderPass();
}

void CommandListWebGPU::Draw(int32_t primitiveCount, int32_t instanceCount)
{
	BindingVertexBuffer bvb;
	BindingIndexBuffer bib;
	PipelineState* bpip = nullptr;

	bool isVBDirtied = false;
	bool isIBDirtied = false;
	bool isPipDirtied = false;

	GetCurrentVertexBuffer(bvb, isVBDirtied);
	GetCurrentIndexBuffer(bib, isIBDirtied);
	GetCurrentPipelineState(bpip, isPipDirtied);

	assert(bvb.vertexBuffer != nullptr);
	assert(bib.indexBuffer != nullptr);
	assert(bpip != nullptr);

	auto vb = static_cast<BufferWebGPU*>(bvb.vertexBuffer);
	auto ib = static_cast<BufferWebGPU*>(bib.indexBuffer);
	auto pip = static_cast<PipelineStateWebGPU*>(bpip);

	if (vb != nullptr)
	{
		renderPassEncorder_.SetVertexBuffer(0, vb->GetBuffer(), bvb.offset, bvb.vertexBuffer->GetSize() - bvb.offset);
	}

	if (ib != nullptr)
	{
		const auto format = bib.stride == 2 ? wgpu::IndexFormat::Uint16 : wgpu::IndexFormat::Uint32;
		renderPassEncorder_.SetIndexBuffer(ib->GetBuffer(), format, bib.offset, ib->GetSize() - bib.offset);
	}

	if (pip != nullptr)
	{
		renderPassEncorder_.SetPipeline(pip->GetRenderPipeline());
		renderPassEncorder_.SetStencilReference(pip->StencilRef);
	}

	// constant buffer
	wgpu::BindGroupLayoutDescriptor constantLayoutDesc = {};
	std::array<wgpu::BindGroupLayoutEntry, NumConstantBuffer> constantLayoutEntries;

	for (uint32_t i = 0; i < NumConstantBuffer; i++)
	{
		constantLayoutEntries[i].buffer.type = wgpu::BufferBindingType::Uniform;
		constantLayoutEntries[i].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
		constantLayoutEntries[i].binding = i;
	}

	constantLayoutDesc.entryCount = constantLayoutEntries.size();
	constantLayoutDesc.entries = constantLayoutEntries.data();
	auto constantBindingLayout = device_.CreateBindGroupLayout(&constantLayoutDesc);

	std::array<wgpu::BindGroupEntry, NumConstantBuffer> constantBindGroupEntries;

	for (size_t unit_ind = 0; unit_ind < constantBuffers_.size(); unit_ind++)
	{
		auto cb = static_cast<BufferWebGPU*>(constantBuffers_[unit_ind]);
		if (cb == nullptr)
		{
			continue;
		}

		constantBindGroupEntries[unit_ind].binding = unit_ind;
		constantBindGroupEntries[unit_ind].buffer = cb->GetBuffer();
		constantBindGroupEntries[unit_ind].size = cb->GetSize();
		constantBindGroupEntries[unit_ind].offset = cb->GetOffset();
	}

	wgpu::BindGroupDescriptor constantBindGroupDesc = {};
	constantBindGroupDesc.layout = constantBindingLayout;
	constantBindGroupDesc.entries = constantBindGroupEntries.data();
	constantBindGroupDesc.entryCount = constantBindGroupEntries.size();
	auto constantBindGroup = device_.CreateBindGroup(&constantBindGroupDesc);

	wgpu::BindGroupLayoutDescriptor textureLayoutDesc = {};
	std::array<wgpu::BindGroupLayoutEntry, NumTexture> textureLayoutEntries;
	throw "Not implemented (Cache layout)";

	for (uint32_t i = 0; i < NumTexture; i++)
	{
		textureLayoutEntries[i].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
		textureLayoutEntries[i].binding = i;
		textureLayoutEntries[i].sampler.type = wgpu::SamplerBindingType::Filtering;
		textureLayoutEntries[i].texture.sampleType = wgpu::TextureSampleType::Float;	 // TODO : Fix correct
		textureLayoutEntries[i].texture.viewDimension = wgpu::TextureViewDimension::e2D; // TODO : Fix correct
	}

	textureLayoutDesc.entryCount = NumTexture;
	textureLayoutDesc.entries = textureLayoutEntries.data();

	auto textureBindLayout = device_.CreateBindGroupLayout(&textureLayoutDesc);

	wgpu::BindGroupDescriptor textureBindGroupDesc = {};
	std::array<wgpu::BindGroupEntry, NumTexture> textureGroupEntries;

	for (int unit_ind = 0; unit_ind < static_cast<int32_t>(currentTextures_.size()); unit_ind++)
	{
		if (currentTextures_[unit_ind].texture == nullptr)
			continue;
		auto texture = static_cast<TextureWebGPU*>(currentTextures_[unit_ind].texture);
		auto wm = (int32_t)currentTextures_[unit_ind].wrapMode;
		auto mm = (int32_t)currentTextures_[unit_ind].minMagFilter;

		auto& groupEntry = textureGroupEntries[unit_ind];
		groupEntry.binding = unit_ind;
		groupEntry.textureView = texture->GetTextureView();
		groupEntry.sampler = samplers_[wm][mm];
	}

	textureBindGroupDesc.layout = textureBindLayout;
	textureBindGroupDesc.entries = textureGroupEntries.data();
	textureBindGroupDesc.entryCount = NumTexture;
	auto textureBindGroup = device_.CreateBindGroup(&textureBindGroupDesc);

	renderPassEncorder_.SetBindGroup(0, constantBindGroup);
	renderPassEncorder_.SetBindGroup(1, textureBindGroup);

	int indexPerPrim = 0;

	if (pip->Topology == TopologyType::Triangle)
	{
		indexPerPrim = 3;
	}
	else if (pip->Topology == TopologyType::Line)
	{
		indexPerPrim = 2;
	}
	else if (pip->Topology == TopologyType::Point)
	{
		indexPerPrim = 1;
	}
	else
	{
		assert(0);
	}

	renderPassEncorder_.DrawIndexed(primitiveCount * indexPerPrim, instanceCount, 0, 0, 0);
	CommandList::Draw(primitiveCount, instanceCount);
}

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
