// Load OpenGL/OpenGLES headers as appropriate for target

#ifndef sg3ios_gl_h
#define sg3ios_gl_h

// Windows (with MinGW)
#if defined(__MINGW32__)
#   include <gl/gl.h>
#   include <gl/glu.h>

// Apple products
#elif defined(__APPLE__)
#   include "TargetConditionals.h"

    // IPhone/IPad
#   if (TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
#       define SG3_OPENGLES 1
#       include <OpenGLES/ES1/gl.h>
#       include <OpenGLES/ES1/glext.h>
//#       include <OpenGLES/ES2/gl.h>
//#       include <OpenGLES/ES2/glext.h>

    // Mac OSX
#   else
#       include <OpenGL/OpenGL.h>
#       include <OpenGL/gl.h>
#       include <OpenGL/glu.h>
#   endif

// Linux
#else
#   include <GL/gl.h>
#   include <GL/glu.h>
#endif

#if SG3_OPENGLES
#define glClearDepth glClearDepthf
#define glDepthRange glDepthRangef
#define glFrustum glFrustumf
#define glOrtho glOrthof
#endif

#ifndef GL_CLAMP
#define GL_CLAMP GL_CLAMP_TO_EDGE
#endif

#include "util.h"

#endif
