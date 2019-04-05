#include "LLGI.GraphicsMetal.h"
#include "LLGI.CommandListMetal.h"
#include "LLGI.ConstantBufferMetal.h"
#include "LLGI.IndexBufferMetal.h"
#include "LLGI.Metal_Impl.h"
#include "LLGI.ShaderMetal.h"
#include "LLGI.VertexBufferMetal.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{

Graphics_Impl::Graphics_Impl() {}

Graphics_Impl::~Graphics_Impl()
{
	if (device != nullptr)
	{
		[device release];
	}

	if (commandQueue != nullptr)
	{
		[commandQueue release];
	}
}

bool Graphics_Impl::Initialize()
{
	device = MTLCreateSystemDefaultDevice();
	commandQueue = [device newCommandQueue];
	return true;
}

void Graphics_Impl::Execute(CommandList_Impl* commandBuffer) { [commandBuffer->commandBuffer commit]; }

RenderPass_Impl::RenderPass_Impl() {}

RenderPass_Impl::~RenderPass_Impl()
{
	if (renderPassDescriptor != nullptr)
	{
		[renderPassDescriptor release];
		renderPassDescriptor = nullptr;
	}
}

bool RenderPass_Impl::Initialize()
{
	renderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
	return true;
}

RenderPassMetal::RenderPassMetal(GraphicsMetal* graphics, bool isStrongRef) : graphics_(graphics), isStrongRef_(isStrongRef)
{
	if (isStrongRef_)
	{
		SafeAddRef(graphics_);
	}

	impl = new RenderPass_Impl();
	impl->Initialize();
}

RenderPassMetal::~RenderPassMetal()
{
	SafeDelete(impl);

	if (isStrongRef_)
	{
		SafeRelease(graphics_);
	}
}

void RenderPassMetal::SetIsColorCleared(bool isColorCleared)
{
	impl->isColorCleared = isColorCleared;
	RenderPass::SetIsColorCleared(isColorCleared);
}

void RenderPassMetal::SetIsDepthCleared(bool isDepthCleared)
{
	impl->isDepthCleared = isDepthCleared;
	RenderPass::SetIsDepthCleared(isDepthCleared);
}

void RenderPassMetal::SetClearColor(const Color8& color)
{
	impl->clearColor = color;
	RenderPass::SetClearColor(color);
}

RenderPass_Impl* RenderPassMetal::GetImpl() const { return impl; }

GraphicsMetal::GraphicsMetal() { impl = new Graphics_Impl(); }

GraphicsMetal::~GraphicsMetal() { SafeDelete(impl); }

bool GraphicsMetal::Initialize(std::function<GraphicsView()> getGraphicsView)
{
	getGraphicsView_ = getGraphicsView;

	if (!impl->Initialize())
	{
		return false;
	}

	renderPass_ = std::make_shared<RenderPassMetal>(this, false);

	return true;
}

void GraphicsMetal::NewFrame()
{
	if (getGraphicsView_ != nullptr)
	{
		auto view = getGraphicsView_();
		impl->drawable = view.drawable;
	}
}

void GraphicsMetal::SetWindowSize(const Vec2I& windowSize) { throw "Not inplemented"; }

void GraphicsMetal::Execute(CommandList* commandList)
{
	auto commandList_ = (CommandListMetal*)commandList;
	impl->Execute(commandList_->GetImpl());
}

void GraphicsMetal::WaitFinish() { throw "Not inplemented"; }

RenderPass* GraphicsMetal::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{

	renderPass_->SetClearColor(clearColor);
	renderPass_->SetIsColorCleared(isColorCleared);
	renderPass_->SetIsDepthCleared(isDepthCleared);
	renderPass_->GetImpl()->renderPassDescriptor.colorAttachments[0].texture = impl->drawable.texture;
	return renderPass_.get();
}

VertexBuffer* GraphicsMetal::CreateVertexBuffer(int32_t size)
{
	auto ret = new VertexBufferMetal();
	if (ret->Initialize(this, size))
	{
		return ret;
	}
	SafeRelease(ret);
	return nullptr;
}

IndexBuffer* GraphicsMetal::CreateIndexBuffer(int32_t stride, int32_t count)
{
	auto ret = new IndexBufferMetal();
	if (ret->Initialize(this, stride, count))
	{
		return ret;
	}
	SafeRelease(ret);
	return nullptr;
}

Shader* GraphicsMetal::CreateShader(DataStructure* data, int32_t count)
{
	auto shader = new ShaderMetal();
	if (shader->Initialize(this, data, count))
	{
		return shader;
	}

	SafeRelease(shader);
	return nullptr;
}

PipelineState* GraphicsMetal::CreatePiplineState() { throw "Not inplemented"; }

CommandList* GraphicsMetal::CreateCommandList()
{
	auto commandList = new CommandListMetal();
	if (commandList->Initialize(this))
	{
		return commandList;
	}

	SafeRelease(commandList);
	return nullptr;
}

ConstantBuffer* GraphicsMetal::CreateConstantBuffer(int32_t size, ConstantBufferType type) { throw "Not inplemented"; }

RenderPass* GraphicsMetal::CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture)
{
	throw "Not inplemented";
}

Texture* GraphicsMetal::CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer) { throw "Not inplemented"; }

Texture* GraphicsMetal::CreateTexture(uint64_t id) { throw "Not inplemented"; }

Graphics_Impl* GraphicsMetal::GetImpl() const { return impl; }

}
