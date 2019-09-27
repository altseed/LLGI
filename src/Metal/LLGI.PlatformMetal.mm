
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#import "../LLGI.Platform.h"
#import "LLGI.GraphicsMetal.h"
#import "LLGI.PlatformMetal.h"
#import "LLGI.RenderPassMetal.h"
#import "LLGI.TextureMetal.h"

@interface LLGIApplication : NSApplication
{
	NSArray* nibObjects;
}

@end

@implementation LLGIApplication

- (void)sendEvent:(NSEvent*)event
{
	[super sendEvent:event];
}

- (void)doNothing:(id)object
{
}

@end

@interface LLGIApplicationDelegate : NSObject
@end

@implementation LLGIApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
	return NSTerminateCancel;
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
	[NSApp stop:nil];

	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
										location:NSMakePoint(0, 0)
								   modifierFlags:0
									   timestamp:0
									windowNumber:0
										 context:nil
										 subtype:0
										   data1:0
										   data2:0];
	[NSApp postEvent:event atStart:YES];
	[pool drain];
}

@end

namespace LLGI
{

struct Cocoa_Impl
{
	static void initialize()
	{
		if (NSApp)
			return;
		[LLGIApplication sharedApplication];

		[NSThread detachNewThreadSelector:@selector(doNothing:) toTarget:NSApp withObject:nil];

		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

		NSMenu* menubar = [NSMenu new];
		[NSApp setMainMenu:menubar];

		id delegate = [[LLGIApplicationDelegate alloc] init];

		[NSApp setDelegate:delegate];

		[NSApp run];
	}
};

struct PlatformMetal_Impl
{
	NSWindow* window = nullptr;
	NSAutoreleasePool* pool = nullptr;
	id<MTLDevice> device;
	id<MTLCommandQueue> commandQueue;
	id<MTLCommandBuffer> commandBuffer;
	CAMetalLayer* layer;
	id<CAMetalDrawable> drawable;

	PlatformMetal_Impl()
	{
		int width = 640;
		int height = 480;

		NSRect frame = NSMakeRect(0, 0, width, height);
		window = [[NSWindow alloc] initWithContentRect:frame
											 styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable)
											   backing:NSBackingStoreBuffered
												 defer:NO];

		window.title = @"LLGI";
		window.releasedWhenClosed = false;
		[window center];
		[window orderFrontRegardless];

		device = MTLCreateSystemDefaultDevice();
		layer = [CAMetalLayer layer];
		layer.device = device;
		layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
		window.contentView.layer = layer;
		window.contentView.wantsLayer = YES;
		layer.drawableSize = CGSizeMake(width, height);
		layer.framebufferOnly = false;	// Enable capture (getBytes)

		pool = [[NSAutoreleasePool alloc] init];

		commandQueue = [device newCommandQueue];
	}

	~PlatformMetal_Impl()
	{
		if (window != nullptr)
		{
			[window release];
			window = nullptr;
		}

		[pool drain];
	}

	void gc()
	{
		[pool drain];
		pool = [[NSAutoreleasePool alloc] init];
	}

	bool newFrame()
	{
		for (;;)
		{
			NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
												untilDate:[NSDate distantPast]
												   inMode:NSDefaultRunLoopMode
												  dequeue:YES];
			if (event == nil)
				break;

			[NSApp sendEvent:event];
		}

		gc();

		if (!window.isVisible)
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

PlatformMetal::PlatformMetal()
{
	Cocoa_Impl::initialize();

	impl = new PlatformMetal_Impl();
}

PlatformMetal::~PlatformMetal()
{
    SafeRelease(renderTexture_);
    SafeRelease(renderPass_);
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
    if(renderPass_ == nullptr)
    {
        renderTexture_ = new TextureMetal();
        renderTexture_->Initialize();
        renderTexture_->Reset(this->impl->drawable.texture);
        
        renderPass_ = new RenderPassMetal((Texture**)&renderTexture_, 1, nullptr);
    }
    
    renderTexture_->Reset(this->impl->drawable.texture);
    renderPass_->SetClearColor(clearColor);
    renderPass_->SetIsColorCleared(isColorCleared);
    renderPass_->SetIsDepthCleared(isDepthCleared);
    return renderPass_;
}


}
