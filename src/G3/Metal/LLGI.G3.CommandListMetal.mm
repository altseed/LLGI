#include "LLGI.G3.CommandListMetal.h"
#include "LLGI.G3.GraphicsMetal.h"
#include "LLGI.G3.IndexBufferMetal.h"
#include "LLGI.G3.Metal_Impl.h"
#include "LLGI.G3.VertexBufferMetal.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{
namespace G3
{

CommandList_Impl::CommandList_Impl() {}

CommandList_Impl::~CommandList_Impl()
{
	if (commandBuffer != nullptr)
	{
		[commandBuffer release];
	}

	if (renderEncoder != nullptr)
	{
		[renderEncoder release];
	}
}

bool CommandList_Impl::Initialize(Graphics_Impl* graphics)
{
	graphics_ = graphics;
	return true;
}

void CommandList_Impl::Begin()
{
	// is it true?
	commandBuffer = [graphics_->commandQueue commandBuffer];
}

void CommandList_Impl::End() {}

void CommandList_Impl::BeginRenderPass(RenderPass_Impl* renderPass)
{
	if (renderPass->isColorCleared)
	{
		auto r_ = renderPass->clearColor.R / 255.0;
		auto g_ = renderPass->clearColor.G / 255.0;
		auto b_ = renderPass->clearColor.B / 255.0;
		auto a_ = renderPass->clearColor.A / 255.0;

		renderPass->renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
		renderPass->renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(r_, g_, b_, a_);
	}
	else
	{
		renderPass->renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
	}

	renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPass->renderPassDescriptor];
}

void CommandList_Impl::EndRenderPass()
{
	if (renderEncoder)
	{
		[renderEncoder endEncoding];
		[renderEncoder release];
		renderEncoder = nullptr;
	}
}

void CommandList_Impl::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
	MTLScissorRect rect;
	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height;
	[renderEncoder setScissorRect:rect];
}

void CommandList_Impl::SetVertexBuffer(Buffer_Impl* vertexBuffer, int32_t stride, int32_t offset)
{
	[renderEncoder setVertexBuffer:vertexBuffer->buffer offset:offset atIndex:0];
}

CommandListMetal::CommandListMetal() { impl = new CommandList_Impl(); }

CommandListMetal::~CommandListMetal()
{
    SafeRelease(currentIndexBuffer);
	SafeDelete(impl);
	SafeRelease(graphics_);
}

bool CommandListMetal::Initialize(Graphics* graphics)
{
	SafeAddRef(graphics);
	SafeRelease(graphics_);
	graphics_ = graphics;

	auto graphics_metal_ = static_cast<GraphicsMetal*>(graphics);
	return impl->Initialize(graphics_metal_->GetImpl());
}

void CommandListMetal::Begin() { impl->Begin(); }

void CommandListMetal::End() { impl->End(); }

void CommandListMetal::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height) { impl->SetScissor(x, y, width, height); }

void CommandListMetal::Draw(int32_t pritimiveCount) { throw "Not inplemented"; }

void CommandListMetal::SetVertexBuffer(VertexBuffer* vertexBuffer, int32_t stride, int32_t offset)
{
	auto vertexBuffer_metal_ = (VertexBufferMetal*)vertexBuffer;
	impl->SetVertexBuffer(vertexBuffer_metal_->GetImpl(), stride, offset);
}

void CommandListMetal::SetIndexBuffer(IndexBuffer* indexBuffer) {
    // index buffer is specfied on Draw.
    throw "Not inplemented";
    
}

void CommandListMetal::SetPipelineState(PipelineState* pipelineState) { throw "Not inplemented"; }

void CommandListMetal::SetConstantBuffer(ConstantBuffer* constantBuffer, ShaderStageType shaderStage) { throw "Not inplemented"; }

void CommandListMetal::SetTexture(
	Texture* texture, TextureWrapMode wrapMode, TextureMinMagFilter minmagFilter, int32_t unit, ShaderStageType shaderStage)
{
	throw "Not inplemented";
}

void CommandListMetal::BeginRenderPass(RenderPass* renderPass)
{
	auto renderPass_ = (RenderPass_Impl*)renderPass;
	impl->BeginRenderPass(renderPass_);
}

void CommandListMetal::EndRenderPass() { impl->EndRenderPass(); }

CommandList_Impl* CommandListMetal::GetImpl() { return impl; }

}
}
