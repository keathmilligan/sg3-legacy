//
//  DemoView.h
//  sg3ios
//
//  Created by Keath Milligan on 3/13/13.
//  Copyright (c) 2013 Keath Milligan. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

@interface DemoView : UIView
{
@private
    EAGLContext *m_context;
	GLint backingWidth;
	GLint backingHeight;
	GLuint defaultFramebuffer, colorRenderbuffer;
	BOOL animating;
	NSInteger animationFrameInterval;
	id displayLink;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;

- (void) startAnimation;
- (void) stopAnimation;
- (void) drawView:(id)sender;

@end
