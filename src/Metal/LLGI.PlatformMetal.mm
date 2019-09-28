
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#import "../LLGI.Platform.h"
#import "LLGI.GraphicsMetal.h"
#import "LLGI.PlatformMetal.h"
#import "LLGI.RenderPassMetal.h"
#import "LLGI.TextureMetal.h"
#import "../Mac/LLGI.WindowMac.h"

namespace LLGI
{

struct PlatformMetal_Impl
{
    std::shared_ptr<WindowMac> window_ = nullptr;

	id<MTLDevice> device;
	id<MTLCommandQueue> commandQueue;
	id<MTLCommandBuffer> commandBuffer;
	CAMetalLayer* layer;
	id<CAMetalDrawable> drawable;

	PlatformMetal_Impl(Vec2I windowSize)
	{
        window_ = std::make_shared<WindowMac>();
        window_->Initialize("LLGI", windowSize);

        NSWindow* nswindow = (NSWindow*)window_->GetNSWindowAsVoidPtr();
        
		device = MTLCreateSystemDefaultDevice();
		layer = [CAMetalLayer layer];
		layer.device = device;
		layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
		nswindow.contentView.layer = layer;
		nswindow.contentView.wantsLayer = YES;
		layer.drawableSize = CGSizeMake(windowSize.X, windowSize.Y);
		layer.framebufferOnly = false;	// Enable capture (getBytes)

		commandQueue = [device newCommandQueue];
	}

	~PlatformMetal_Impl()
	{
        window_->Terminate();
        window_.reset();
	}

	bool newFrame()
	{
        if(!window_->DoEvent())
        {
            return false;
        }

		drawable = layer.nextDrawable;

		return true;
	}

	void preset()
	{
		commandBuffer = [commandQueue commandBuffer];
		[commandBuffer presentDrawable:drawable];
		[commandBuffer commit];
	}
};

PlatformMetal::PlatformMetal(Vec2I windowSize)
{
	impl = new PlatformMetal_Impl(windowSize);
    
    ringBuffers_.resize(6);
    for(size_t i = 0; i < ringBuffers_.size(); i++)
    {
        ringBuffers_[i].renderPass = CreateSharedPtr(new RenderPassMetal());
        ringBuffers_[i].renderTexture = CreateSharedPtr(new TextureMetal());
        ringBuffers_[i].renderTexture->Initialize();
    }
}

PlatformMetal::~PlatformMetal()
{
    delete impl;
}

bool PlatformMetal::NewFrame() { return impl->newFrame(); }

void PlatformMetal::Present() { impl->preset(); }

Graphics* PlatformMetal::CreateGraphics()
{
	auto ret = new GraphicsMetal();

	auto getGraphicsView = [this]() -> GraphicsView {
		GraphicsView view;
		view.drawable = this->impl->drawable;
		return view;
	};

	if (ret->Initialize(getGraphicsView))
	{
		return ret;
	}

	SafeRelease(ret);
	return nullptr;
}
    
RenderPass* PlatformMetal::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{
    // delay init
    ringBuffers_[ringIndex_].renderTexture->Reset(this->impl->drawable.texture);
    auto texPtr = ringBuffers_[ringIndex_].renderTexture.get();
    ringBuffers_[ringIndex_].renderPass->UpdateRenderTarget((Texture**)&texPtr, 1, nullptr);
    
    ringBuffers_[ringIndex_].renderPass->SetClearColor(clearColor);
    ringBuffers_[ringIndex_].renderPass->SetIsColorCleared(isColorCleared);
    ringBuffers_[ringIndex_].renderPass->SetIsDepthCleared(isDepthCleared);
    return ringBuffers_[ringIndex_].renderPass.get();
}


}
