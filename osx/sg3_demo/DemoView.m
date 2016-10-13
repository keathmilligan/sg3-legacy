//
//  DemoView.m
//  sg3osx
//
//  Created by Keath Milligan on 3/12/13.
//
//

#import <OpenGL/gl.h>
#import <examples/sg3_demo.h>
#import "DemoView.h"

void _log_std_output(const char *msg)
{
    NSLog(@"%s", msg);
}

void _log_err_output(const char *msg)
{
    NSLog(@"ERROR: %s", msg);
}


@interface DemoView (PrivateMethods)
- (void) renderView;
@end

@implementation DemoView

- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	[self renderView];
	
	[pool release];
	return kCVReturnSuccess;
}

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    CVReturn result = [(DemoView*)displayLinkContext getFrameForTime:outputTime];
    return result;
}

- (void) prepareOpenGL
{
	[super prepareOpenGL];
	[[self openGLContext] makeCurrentContext];
	GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    demo_init();
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
	CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
	CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
	CVDisplayLinkStart(displayLink);
}

- (void) reshape
{
	[super reshape];
	CGLLockContext([[self openGLContext] CGLContextObj]);
    
    demo_reshape_viewport([self bounds].size.width, [self bounds].size.height);
	
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

- (void) renderView
{
	[[self openGLContext] makeCurrentContext];
	CGLLockContext([[self openGLContext] CGLContextObj]);
    
    demo_render();
    
    CGLFlushDrawable([[self openGLContext] CGLContextObj]);
	CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
        NSOpenGLPixelFormatAttribute attrs[] =
        {
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFADepthSize, 24,
            0
        } ;
        
        NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs];
        
        if (!fmt)
            NSLog(@"No OpenGL pixel format");
        
        return self = [super initWithFrame:frame pixelFormat: [fmt autorelease]];
    }
    
    return self;
}

- (void) dealloc
{
	CVDisplayLinkStop(displayLink);
	CVDisplayLinkRelease(displayLink);
    
    demo_cleanup();
    
	[super dealloc];
}

- (void)drawRect:(NSRect)dirtyRect
{
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)becomeFirstResponder
{
    return YES;
}

static void handleNavKey(NSEvent *theEvent, BOOL down)
{
    unichar key = 0;
    if ([theEvent modifierFlags] & NSNumericPadKeyMask)
    {
        NSString *theArrow = [theEvent charactersIgnoringModifiers];
        if ( [theArrow length] == 1 )
            key = [theArrow characterAtIndex:0];
    }
    else
    {
        NSString *characters = [theEvent characters];
        if ([characters length])
            key = [characters characterAtIndex:0];
    }
    switch(key)
    {
        case NSLeftArrowFunctionKey:
        case '4':
            demo_left(down);
            break;
        case NSRightArrowFunctionKey:
        case '6':
            demo_right(down);
            break;
        case NSUpArrowFunctionKey:
        case '8':
            demo_up(down);
            break;
        case NSDownArrowFunctionKey:
        case '2':
            demo_down(down);
            break;
        case 'z':
        case 'Z':
        case ',':
        case '<':
        case '1':
            demo_roll_left(down);
            break;
        case 'x':
        case 'X':
        case '.':
        case '>':
        case '3':
            demo_roll_right(down);
            break;
        case NSHomeFunctionKey:
        case '7':
            if (down)
                demo_reset();
            break;
        case '+':
        case '=':
            if (down)
                demo_throttle_up();
            break;
        case '-':
        case '_':
            if (down)
                demo_throttle_down();
            break;
        case '0':
            if (down)
                demo_throttle_reset();
            break;
    }
}

-(void)keyDown:(NSEvent *)theEvent
{
    NSLog(@"keyDown");
    handleNavKey(theEvent, TRUE);
}

-(void)keyUp:(NSEvent *)theEvent
{
    NSLog(@"keyUp");
    handleNavKey(theEvent, FALSE);
}

@end
