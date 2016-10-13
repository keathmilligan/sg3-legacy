/* util.h - misc utilities
 * Copyright 2012 Keath Milligan
 */

#ifndef UTIL_H_
#define UTIL_H_

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

#if SG3_OPENGLES
void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
               GLfloat centerx, GLfloat centery, GLfloat centerz,
               GLfloat upx, GLfloat upy, GLfloat upz);
#endif

#endif /* UTIL_H_ */
