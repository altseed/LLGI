#include "LLGI.GraphicsMetal.h"
#include "LLGI.CommandListMetal.h"
#include "LLGI.ConstantBufferMetal.h"
#include "LLGI.IndexBufferMetal.h"
#include "LLGI.Metal_Impl.h"
#include "LLGI.PipelineStateMetal.h"
#include "LLGI.ShaderMetal.h"
#include "LLGI.VertexBufferMetal.h"
#include "LLGI.TextureMetal.h"
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

void RenderPass_Impl::UpdateTarget(Graphics_Impl* graphics)
{
    renderPassDescriptor.colorAttachments[0].texture = graphics->drawable.texture;
    pixelFormat = graphics->drawable.texture.pixelFormat;
}
    
void RenderPass_Impl::UpdateTarget(Texture_Impl** textures, int32_t textureCount, Texture_Impl* depthTexture)
{
    for(int i = 0; i < textureCount; i++)
    {
        renderPassDescriptor.colorAttachments[i].texture = textures[i]->texture;
    }
    
    if(depthTexture != nullptr)
    {
        renderPassDescriptor.depthAttachment.texture = depthTexture->texture;
    }
    
    pixelFormat = textures[0]->texture.pixelFormat;
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

Texture* RenderPassMetal::GetColorBuffer(int index) { return colorBuffers_[index].get(); }
	
RenderPass_Impl* RenderPassMetal::GetImpl() const { return impl; }

RenderPassPipelineState* RenderPassMetal::CreateRenderPassPipelineState()
{
	if (renderPassPipelineState == nullptr)
	{
		renderPassPipelineState = graphics_->CreateRenderPassPipelineState(GetImpl()->pixelFormat);
	}

	auto ret = renderPassPipelineState.get();
	SafeAddRef(ret);
	return ret;
}

void RenderPassMetal::UpdateTarget(GraphicsMetal* graphics)
{
    GetImpl()->UpdateTarget(graphics->GetImpl());
	
	// create backbuffer wrapper texture if needed.
	if (!colorBuffers_[0]) {
		auto texture = std::make_shared<TextureMetal>();
		if (!texture->Initialize(graphics)) {
			return;
		}
		colorBuffers_[0] = texture;
	}

	id<MTLTexture> texture = [graphics->GetImpl()->drawable texture];
	colorBuffers_[0]->Reset(texture);
}

RenderPassPipelineStateMetal::RenderPassPipelineStateMetal() { impl = new RenderPassPipelineState_Impl(); }

RenderPassPipelineStateMetal::~RenderPassPipelineStateMetal() { SafeDelete(impl); }

RenderPassPipelineState_Impl* RenderPassPipelineStateMetal::GetImpl() const { return impl; }

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

void GraphicsMetal::SetWindowSize(const Vec2I& windowSize) { throw "Not inplemented"; }

void GraphicsMetal::Execute(CommandList* commandList)
{
	auto commandList_ = (CommandListMetal*)commandList;
	impl->Execute(commandList_->GetImpl());
}

void GraphicsMetal::WaitFinish() { throw "Not inplemented"; }

RenderPass* GraphicsMetal::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{
	if (getGraphicsView_ != nullptr)
	{
		auto view = getGraphicsView_();
		impl->drawable = view.drawable;
	}

	renderPass_->SetClearColor(clearColor);
	renderPass_->SetIsColorCleared(isColorCleared);
	renderPass_->SetIsDepthCleared(isDepthCleared);
	renderPass_->UpdateTarget(this);
	
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

PipelineState* GraphicsMetal::CreatePiplineState()
{
	auto pipelineState = new PipelineStateMetal();
	if (pipelineState->Initialize(this))
	{
		return pipelineState;
	}

	SafeRelease(pipelineState);
	return nullptr;
}

SingleFrameMemoryPool* GraphicsMetal::CreateSingleFrameMemoryPool(int32_t constantBufferPoolSize, int32_t drawingCount)
{
	return new SingleFrameMemoryPoolMetal(this, constantBufferPoolSize, drawingCount);
}

CommandList* GraphicsMetal::CreateCommandList(SingleFrameMemoryPool* memoryPool)
{
	auto commandList = new CommandListMetal();
	if (commandList->Initialize(this))
	{
		return commandList;
	}

	SafeRelease(commandList);
	return nullptr;
}

ConstantBuffer* GraphicsMetal::CreateConstantBuffer(int32_t size) {
    auto obj = new ConstantBufferMetal();
    if (obj->Initialize(this, size))
    {
        return obj;
    }
    
    SafeRelease(obj);
    return nullptr;
}

RenderPass* GraphicsMetal::CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture)
{
    auto renderPass = new RenderPassMetal(this, true);

    std::array<Texture_Impl*, 16> textures_;
    Texture_Impl* depth_ = nullptr;
    textures_.fill(nullptr);
    
    for(int i = 0; i < textureCount; i++)
    {
        textures_[i] = static_cast<const TextureMetal*>(textures[i])->GetImpl();
    }
    
    if(depthTexture != nullptr)
    {
        depth_ = static_cast<const TextureMetal*>(depthTexture)->GetImpl();
    }
    
    renderPass->GetImpl()->UpdateTarget(textures_.data(), textureCount, depth_);
    
    return renderPass;
}

Texture* GraphicsMetal::CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer) {
    auto o = new TextureMetal();
    if (o->Initialize(this, size, isRenderPass, isDepthBuffer))
    {
        return o;
    }
    
    SafeRelease(o);
    return nullptr;    
}

Texture* GraphicsMetal::CreateTexture(uint64_t id) { throw "Not inplemented"; }

std::shared_ptr<RenderPassPipelineStateMetal> GraphicsMetal::CreateRenderPassPipelineState(MTLPixelFormat format)
{
	RenderPassPipelineStateMetalKey key;
	key.format = format;

	// already?
	{
		auto it = renderPassPipelineStates.find(key);

		if (it != renderPassPipelineStates.end())
		{
			auto ret = it->second.lock();

			if (ret != nullptr)
				return ret;
		}
	}

	std::shared_ptr<RenderPassPipelineStateMetal> ret = std::make_shared<RenderPassPipelineStateMetal>();
	ret->GetImpl()->pixelFormat = format;

	renderPassPipelineStates[key] = ret;

	return ret;
}

std::vector<uint8_t> GraphicsMetal::CaptureRenderTarget(Texture* renderTarget)
{
	auto metalTexture = static_cast<TextureMetal*>(renderTarget);
	auto width = metalTexture->GetSizeAs2D().X;
	auto height = metalTexture->GetSizeAs2D().Y;
	auto impl = metalTexture->GetImpl();
	
	NSUInteger bytesPerPixel = 4;	// TODO: backbuffer only
	NSUInteger imageByteCount = width * height * bytesPerPixel;
	NSUInteger bytesPerRow = width * bytesPerPixel;
	MTLRegion region = MTLRegionMake2D(0, 0, width, height);
	
	std::vector<uint8_t> data;
	data.resize(imageByteCount);
	[impl->texture getBytes:data.data() bytesPerRow:bytesPerRow fromRegion:region mipmapLevel:0];
	// order: B=[0], G=[1], R=[2], A=[3]
	
	return data;
}

Graphics_Impl* GraphicsMetal::GetImpl() const { return impl; }

}
