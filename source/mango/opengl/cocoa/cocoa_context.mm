/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2020 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#if !defined(__ppc__)

#include <mango/core/string.hpp>
#include <mango/opengl/opengl.hpp>

#include "CustomOpenGLView.h"

// -----------------------------------------------------------------------
// CustomNSWindowDelegate
// -----------------------------------------------------------------------

@interface CustomNSWindowDelegate : NSObject {
    mango::OpenGLContext *context;
    id view;
    mango::WindowHandle *window_handle;
}

- (id)initWithCustomWindow:(mango::OpenGLContext *)theContext 
      andView:(id)theView
      andWindowHandle:(mango::WindowHandle *)theWindowHandle;
@end

// ...

@implementation CustomNSWindowDelegate

- (id)initWithCustomWindow:(mango::OpenGLContext *)theContext 
      andView:(id)theView
      andWindowHandle:(mango::WindowHandle *)theWindowHandle;
{
    if ((self = [super init]))
    {
        context = theContext;
        window_handle = theWindowHandle;
        view = theView;
    }

    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    (void)sender;
    context->breakEventLoop();
    return NO;
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    (void)notification;
    // NOTE: window gained focus
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    (void)notification;
    // NOTE: window lost focus
}

- (void)windowDidResize:(NSNotification *)notification
{
    //[handle->ctx update];
    NSRect frame = [[notification object] contentRectForFrameRect: [[notification object] frame]];
    [view dispatchResize:frame];
}

- (void)windowDidMove:(NSNotification *)notification
{
    (void)notification;
    //[handle->ctx update];
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    (void)notification;
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    (void)notification;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
}

@end

namespace mango
{

    // -----------------------------------------------------------------------
    // OpenGLContextCocoa
    // -----------------------------------------------------------------------

    struct OpenGLContextCocoa : OpenGLContextHandle
    {
        id delegate;
        id view = nil;
        id ctx = nil;
        id window;
        u32 modifiers;

        OpenGLContextCocoa(OpenGLContext* theContext, int width, int height, u32 flags, const OpenGLContext::Config* configPtr, OpenGLContext* theShared)
        {
            WindowHandle* theHandle = *theContext;

            [NSApplication sharedApplication];

            ProcessSerialNumber psn = { 0, kCurrentProcess };
            TransformProcessType(&psn, kProcessTransformToForegroundApplication);
            [[NSApplication sharedApplication] activateIgnoringOtherApps: YES];

            [NSEvent setMouseCoalescingEnabled:NO];
            [NSApp finishLaunching];

            unsigned int styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

            NSRect frame = NSMakeRect(0, 0, width, height);

            theHandle->window = [[CustomNSWindow alloc]
                initWithContentRect:frame
                styleMask:styleMask
                backing:NSBackingStoreBuffered
                defer:NO];
            if (!theHandle->window)
            {
                printf("NSWindow initWithContentRect failed.\n");
                return;
            }

            window = theHandle->window;

            ((CustomNSWindow *)theHandle->window).window = theContext;
            [ (NSWindow*) window center];
            delegate = [[CustomNSWindowDelegate alloc] initWithCustomWindow:theContext andView:view andWindowHandle:theHandle];
            [window setDelegate:[delegate retain]];
            [window setAcceptsMouseMovedEvents:YES];
            [window setTitle:@"OpenGL"];
            [window setReleasedWhenClosed:NO];

            view = [[CustomView alloc] initWithFrame:[window frame] andCustomWindow:theContext];
            if (!view)
            {
                printf("NSView initWithFrame failed.\n");
                // TODO: delete window
                return;
            }

            view = [view retain];
            [[window contentView] addSubview:view];
            [view trackContentView:window];

            // Configure the smallest allowed window size
            [window setMinSize:NSMakeSize(32, 32)];

            // Create menu
            [window createMenu];

            // configure attributes
            OpenGLContext::Config config;
            if (configPtr)
            {
                // override defaults
                config = *configPtr;
            }

            std::vector<NSOpenGLPixelFormatAttribute> attribs;

            if (!config.version || config.version >= 4)
            {
                attribs.push_back(NSOpenGLPFAOpenGLProfile);
                attribs.push_back(NSOpenGLProfileVersion4_1Core);
            }

            attribs.push_back(NSOpenGLPFAAccelerated);
            attribs.push_back(NSOpenGLPFADoubleBuffer);
            //attribs.push_back(NSOpenGLPFATripleBuffer);
            //attribs.push_back(NSOpenGLPFAFullScreen);
            //attribs.push_back(NSOpenGLPFAWindow);

            attribs.push_back(NSOpenGLPFAColorSize);
            attribs.push_back(config.red + config.green + config.blue);
            attribs.push_back(NSOpenGLPFAAlphaSize);
            attribs.push_back(config.alpha);
            attribs.push_back(NSOpenGLPFADepthSize);
            attribs.push_back(config.depth);
            attribs.push_back(NSOpenGLPFAStencilSize);
            attribs.push_back(config.stencil);

            if (config.samples > 1)
            {
                attribs.push_back(NSOpenGLPFASampleBuffers);
                attribs.push_back(1);
                attribs.push_back(NSOpenGLPFASamples);
                attribs.push_back(config.samples);
                attribs.push_back(NSOpenGLPFAMultisample);
            }

            // terminate attribs vector
            attribs.push_back(0);

            id pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs.data()];
            if (pixelFormat == nil)
            {
                printf("NSOpenGLPixelFormat initWithAttributes failed.\n");
                // TODO: delete window
                return;
            }

            OpenGLContextCocoa* shared = theShared ? reinterpret_cast<OpenGLContextCocoa*>(theShared) : nullptr;

            ctx = [[NSOpenGLContext alloc]
                initWithFormat:pixelFormat
                shareContext: shared ? shared->ctx : nil];
            [pixelFormat release];

            if (!ctx)
            {
                printf("Failed to create NSOpenGL Context.\n");
                // TODO: delete window
                return;
            }

            [view setOpenGLContext:ctx];
            [ctx makeCurrentContext];

            if ([window respondsToSelector:@selector(setRestorable:)])
            {
                [window setRestorable:NO];
            }

            modifiers = [NSEvent modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;

            [ctx update];
            [view dispatchResize:frame];
        }

        ~OpenGLContextCocoa()
        {
            [NSOpenGLContext clearCurrentContext];

            if (ctx)
            {
                [ctx release];
                ctx = nil;
            }

            if (window)
            {
                [window setDelegate:nil];
                [delegate release];
                [window setContentView:nil];
                [view release];
                [window close];
            }

            [NSApp stop:nil];
        }

        void makeCurrent() override
        {
            [ctx makeCurrentContext];
        }

        void swapBuffers() override
        {
            [ctx flushBuffer];
        }

        void swapInterval(int interval) override
        {
            GLint sync = interval;
            [ctx setValues:&sync forParameter:NSOpenGLContextParameterSwapInterval];
        }

        void toggleFullscreen() override
        {
            [view setHidden:YES];

            if ([view isInFullScreenMode])
            {
                [view exitFullScreenModeWithOptions:nil];
                [window makeFirstResponder:view];
                [window makeKeyAndOrderFront:view];
                [view trackContentView:window];
            }
            else
            {
                [view enterFullScreenMode:[window screen] withOptions:nil];
            }

            [view dispatchResize:[view frame]];
            [view setHidden:NO];
        }

        bool isFullscreen() const override
        {
            return [view isInFullScreenMode];
        }

        math::int32x2 getWindowSize() const override
        {
            NSRect rect = [view frame];
            rect = [[window contentView] convertRectToBacking:rect]; // NOTE: Retina conversion
            return math::int32x2(rect.size.width, rect.size.height);
        }
    };

    // -----------------------------------------------------------------------
    // OpenGLContext
    // -----------------------------------------------------------------------

    OpenGLContext::OpenGLContext(int width, int height, u32 flags, const Config* configPtr, OpenGLContext* shared)
        : Window(width, height, flags)
        , m_context(nullptr)
    {
        m_context = new OpenGLContextCocoa(this, width, height, flags, configPtr, shared);

        setVisible(true);

        // parse extension string
        const GLubyte* extensions = glGetString(GL_EXTENSIONS);
        if (extensions)
        {
            parseExtensionString(m_extensions, reinterpret_cast<const char*>(extensions));
        }

        initExtensionMask();
    }

    OpenGLContext::~OpenGLContext()
    {
        delete m_context;
    }

} // namespace mango

#endif // !defined(__ppc__)
