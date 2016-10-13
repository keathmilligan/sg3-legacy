//
//  main.m
//  sg3_demo
//
//  Created by Keath Milligan on 3/12/13.
//
//

#import <Cocoa/Cocoa.h>
#import "DemoView.h"

int main(int argc, char *argv[])
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    NSLog(@"NSApp=%@", NSApp);
    [NSApplication sharedApplication];
    NSLog(@"NSApp=%@", NSApp);
    [NSBundle loadNibNamed:@"MainMenu" owner:NSApp];
    NSRect frame = NSMakeRect(100, 100, 1280, 720);
    NSUInteger styleMask = NSResizableWindowMask | NSTitledWindowMask | NSMiniaturizableWindowMask;
    NSRect rect = [NSWindow contentRectForFrameRect:frame styleMask:styleMask];
    NSWindow * window = [[NSWindow alloc] initWithContentRect:rect styleMask:styleMask
                                                      backing: NSBackingStoreBuffered defer:false];
    DemoView *view;
    view = [[DemoView alloc] initWithFrame:rect];
    [window setContentView:view];
    [view display];
    [window makeKeyAndOrderFront: window];
    [pool drain];
    [NSApp run];
}
