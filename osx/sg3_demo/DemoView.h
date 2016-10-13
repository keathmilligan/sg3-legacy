//
//  DemoView.h
//  sg3osx
//
//  Created by Keath Milligan on 3/12/13.
//
//

#import <Cocoa/Cocoa.h>

@interface DemoView : NSOpenGLView
{
    CVDisplayLinkRef displayLink;
}

- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;
- (void) keyDown:(NSEvent *)theEvent;
- (void) keyUp:(NSEvent *)theEvent;

@end
