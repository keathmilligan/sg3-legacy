/* types.h - misc types
 * Colors, UVs, etc.
 * Copyright 2012 Keath Milligan
 */

#ifndef TYPES_H_
#define TYPES_H_

#ifdef __MINGW32__
#include <windows.h>
#endif
#include <ut/utlist.h>
#include <ut/uthash.h>

#ifndef TRUE
#define TRUE -1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define COLOR(r, g, b, a) ((Color){a, r, g, b})
#define COLORF(r, g, b, a) ((Color){ (unsigned char)(a*255.0f), \
                                     (unsigned char)(r*255.0f), \
                                     (unsigned char)(g*255.0f), \
                                     (unsigned char)(b*255.0f) })
#define COLORL(x) ((Color){ (unsigned char)((x >> 24) & 0x000000FF), \
                            (unsigned char)((x >> 16) & 0x000000FF), \
                            (unsigned char)((x >> 8) & 0x000000FF), \
                            (unsigned char)(x & 0x000000FF) })
#define SETCOLOR(c, _r, _g, _b, _a) { c.r = _r; c.g = _g; c.b = _b; c.a = _a; }
#define SETCOLORL(c, x) { c.a = (unsigned char)((x >> 24) & 0x000000FF); \
                          c.r = (unsigned char)((x >> 16) & 0x000000FF); \
                          c.g = (unsigned char)((x >> 8) & 0x000000FF); \
                          c.b = (unsigned char)(x & 0x000000FF); }
#define SETCOLORF(c, _r, _g, _b, _a) { c.a = (unsigned char)(_a*255.0f); \
                                       c.r = (unsigned char)(_r*255.0f); \
                                       c.g = (unsigned char)(_g*255.0f); \
                                       c.b = (unsigned char)(_b*255.0f); }
#define COLORLV(x) ( (unsigned long)(x.a << 24) | \
                     (unsigned long)(x.r << 16) | \
                     (unsigned long)(x.g << 8) | \
                    (unsigned long)(x.b) )
#define COLORFA(x) (float[]){ (float)x.r/255.0f, \
                              (float)x.g/255.0f, \
                              (float)x.b/255.0f, \
                              (float)x.a/255.0f }
#define COLORFL(x) (float)x.r/255.0f, \
                   (float)x.g/255.0f, \
                   (float)x.b/255.0f, \
                   (float)x.a/255.0f
#define SET2D(n, _x, _y) { n.x = _x; n.y = _y; }
#define SET3D(n, _x, _y, _z)  { n.x = _x; n.y = _y; n.z = _z; }
#define SETQ(q, _w, _x, _y, _z) { q.w = _w; q.x = _x; q.y = _y; q.z = _z; }
#define SETLINE2D(l, x1, y1, x2, y2) { l.p0.x = x1; l.p0.y = y1; \
                                       l.p1.x = x2; l.p1.y = y2 }
#define SETLINE3D(l, x1, y1, z1, x2, y2, z2) { l.p0.x = x1; l.p0.y = y1; l.p0.z = z1; \
                                               l.p1.x = x2; l.p1.y = y2; l.p1.z = z2; }
#define NUM2D(x, y) (Number2D){x, y}
#define NUM3D(x, y, z) (Number3D){x, y, z}
#define NUM2DFA(n) (float[]){ n.x, n.y }
#define NUM3DFA(n) (float[]){ n.x, n.y, n.z }
#define NUM2DFL(n) n.x, n.y
#define NUM3DFL(n) n.x, n.y, n.z
#define FA(n) ((float*)&n)
#define QUAD2TRIS(_ul, _ur, _lr, _ll, _p1, _p2) { _p1.a = _ul; _p1.b = _lr; _p1.c = _ur; \
                                                  _p2.a = _ul; _p2.b = _ll; _p2.c = _lr; }

// RGBA Color
typedef struct _Color {
    unsigned char a, r, g, b;
} Color;

// 2D number
typedef struct _Number2D {
    float x, y;
} Number2D;

// 3D number
typedef struct _Number3D {
    float x, y, z;
} Number3D;

// Quaternion
typedef struct _Quaternion {
    float w, x, y, z;
} Quaternion;

// 3D line segment
typedef struct _Line3D {
    Number3D p0, p1;
} Line3D;

// 2D line segment
typedef struct _Line2D {
    Number2D p0, p1;
} Line2D;

// 2D Triangle
typedef struct _Triangle2D {
    Number2D a, b, c;
} Triangle2D;

// 2D Rectangle
typedef struct _Rect3D {
    Number2D a, b, c, d;
} Rect2D;

// 3D Triangular Plane
typedef struct _Triangle3D {
    Number3D a, b, c;
    Number3D normal;
} Triangle3D;

// 3D Quadrilateral Plane
typedef struct _Quad3D {
    Number3D a, b, c, d;
    Number3D normal;
} Quad3D;

// 3D Cube
typedef struct _Cube3D {
    Number3D min;
    Number3D max;
} Cube3D;

#endif /* TYPES_H_ */
