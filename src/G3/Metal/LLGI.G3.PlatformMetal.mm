
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#import "LLGI.G3.PlatformMetal.h"
#import "../LLGI.G3.Platform.h"

@interface LLGIApplication : NSApplication
{
    NSArray* nibObjects;
}

@end

@implementation LLGIApplication

- (void)sendEvent:(NSEvent *)event
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

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    return NSTerminateCancel;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
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

namespace LLGI {
namespace G3{
    
    struct Cocoa_Impl
    {
        static void initialize()
        {
            if (NSApp) return;
            [LLGIApplication sharedApplication];
         
            [NSThread
             detachNewThreadSelector:@selector(doNothing:)
             toTarget:NSApp
             withObject:nil];
            
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
        id <MTLCommandQueue> commandQueue;
        id <MTLCommandBuffer> commandBuffer;
        CAMetalLayer* layer;
        
        PlatformMetal_Impl()
        {
            int width = 640;
            int height = 480;
            
            NSRect frame = NSMakeRect(0, 0, width, height);
            window = [[NSWindow alloc]
                      initWithContentRect:frame
                      styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable)
                      backing:NSBackingStoreBuffered
                      defer:NO];
            
            window.title = @"LLGI";
            [window center];
            [window orderFrontRegardless];

            device = MTLCreateSystemDefaultDevice();
            layer = [CAMetalLayer layer];
            layer.device = device;
            layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
            window.contentView.layer = layer;
            window.contentView.wantsLayer = YES;
            layer.drawableSize = CGSizeMake(width, height);
            
            pool = [[NSAutoreleasePool alloc] init];
            
            commandQueue = [device newCommandQueue];
        }
        
        ~PlatformMetal_Impl()
        {
            [pool drain];
        }
        
        void gc()
        {
            [pool drain];
            pool = [[NSAutoreleasePool alloc] init];
        }
        
        
        void newFrame()
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
        }
        
        void preset()
        {
            commandBuffer = [commandQueue commandBuffer];
            [commandBuffer presentDrawable:layer.nextDrawable];
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
            delete impl;
        }
        
        void PlatformMetal::NewFrame()
        {
            impl->newFrame();
        }
        
        void PlatformMetal::Present()
        {
            impl->preset();
        }
        
        Graphics* PlatformMetal::CreateGraphics()
        {
            return nullptr;
        }
    
    
}
}
