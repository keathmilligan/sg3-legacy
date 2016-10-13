/* util.h - misc utilities
 * Copyright 2012 Keath Milligan
 */

#ifndef UTIL_H_
#define UTIL_H_

#if defined(__MINGW32__)
#include <gl/gl.h>
#include <gl/glu.h>
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

GLint gluProjectf(GLfloat objx, GLfloat objy, GLfloat objz,
                  const GLfloat modelMatrix[16],
                  const GLfloat projMatrix[16],
                  const GLint viewport[4],
                  GLfloat *winx, GLfloat *winy, GLfloat *winz);

GLint gluUnProjectf(GLfloat winx, GLfloat winy, GLfloat winz,
                    const GLfloat modelMatrix[16],
                    const GLfloat projMatrix[16],
                    const GLint viewport[4],
                    GLfloat *objx, GLfloat *objy, GLfloat *objz);



#endif /* UTIL_H_ */
