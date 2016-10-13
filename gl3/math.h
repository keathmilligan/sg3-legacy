/* math.h - math functions
 * 2D and 3D math support
 * Copyright 2012 Keath Milligan
 */


#ifndef MATH_H_
#define MATH_H_

#include <math.h>
#include <float.h>
#include <string.h>
#include "types.h"

// Misc utilities & definitions

#define PI 3.14159265358979323846f
//#define EPSILON	1.1920928955078125E-7f

#define	DEG2RAD(x)	(((x)*PI)/180.0)
#define	RAD2DEG(x)	(((x)*180.0)/PI)
#define	SQR(x) ((x)*(x))
#define EULER2D(x, y) ((Number2D){DEG2RAD((float)x), DEG2RAD((float)y)})
#define EULER3D(x, y, z) ((Number3D){DEG2RAD((float)x), DEG2RAD((float)y), DEG2RAD((float)z)})
#define SWAPF(a, b) { float _t = a; a = b; b = _t; }

// Matrix index values
#define M00 0
#define M01 1
#define M02 2
#define M03 3
#define M10 4
#define M11 5
#define M12 6
#define M13 7
#define M20 8
#define M21 9
#define M22 10
#define M23 11
#define M30 12
#define M31 13
#define M32 14
#define M33 15

extern const float IDMAT[16];
extern const Quaternion IDQUAT;

// floating point approximate equality
static inline int close_enough(float a, float b) {
    return fabsf((a - b) / ((b == 0.0f) ? 1.0f : b)) < FLT_EPSILON;
}

// limit a floating point number to a range
static inline float clamp(float value, float low, float high) {
    if (value < low)
        return low;
    else if (value > high)
        return high;
    else
        return value;
}

// 2D math functions
static inline void add2d(Number2D *n, Number2D v) { n->x += v.x; n->y += v.y; }
static inline void sub2d(Number2D *n, Number2D v) { n->x -= v.x; n->y -= v.y; }
static inline void mul2d(Number2D *n, Number2D v) { n->x *= v.x; n->y *= v.y; }
static inline void div2d(Number2D *n, Number2D v) { n->x /= v.x; n->y /= v.y; }
static inline void adds2d(Number2D *n, float v) { n->x += v; n->y += v; }
static inline void subs2d(Number2D *n, float v) { n->x -= v; n->y -= v; }
static inline void muls2d(Number2D *n, float v) { n->x *= v; n->y *= v; }
static inline void divs2d(Number2D *n, float v) { n->x /= v; n->y /= v; }
static inline void inv2d(Number2D *n) { n->x = -n->x; n->y = -n->y; }
static inline float len2d(Number2D n) { return (float)sqrt(SQR(n.x)+SQR(n.y)); }
static inline void norm2d(Number2D *n) { float mod = len2d(*n); if (mod != 0.0f && mod != 1.0f) muls2d(n, 1.0f/mod); }
void rot2d(Number2D *n, float angle);
static inline float dot2d(Number2D v, Number2D w) { return v.x*w.x+v.y*w.y; }
static inline float angle2d(Number2D v, Number2D w) { return atan2f(w.y, w.x)-atan2(v.y, v.y); }
int isx2d(Line2D a, Line2D b, int segment, Number2D *where);
static inline Number2D vec2d(Number2D p0, Number2D p1) { Number2D v = p1; sub2d(&v, p0); return v; }
static inline Number2D nvec2d(Number2D p0, Number2D p1) { Number2D v = p1; sub2d(&v, p0); norm2d(&v); return v; }

// 3D math functions
static inline void add3d(Number3D *n, Number3D v) { n->x += v.x; n->y += v.y; n->z += v.z; }
static inline void sub3d(Number3D *n, Number3D v) { n->x -= v.x; n->y -= v.y; n->z -= v.z; }
static inline void mul3d(Number3D *n, Number3D v) { n->x *= v.x; n->y *= v.y; n->z *= v.z; }
static inline void div3d(Number3D *n, Number3D v) { n->x /= v.x; n->y /= v.y; n->z /= v.z; }
static inline void adds3d(Number3D *n, float v) { n->x += v; n->y += v; n->z += v; }
static inline void subs3d(Number3D *n, float v) { n->x -= v; n->y -= v; n->z -= v; }
static inline void muls3d(Number3D *n, float v) { n->x *= v; n->y *= v; n->z *= v; }
static inline void divs3d(Number3D *n, float v) { n->x /= v; n->y /= v; n->z /= v; }
static inline void inv3d(Number3D *n) { n->x = -n->x; n->y = -n->y; n->z = -n->z; }
static inline float len3d(Number3D n) { return (float)sqrt(SQR(n.x)+SQR(n.y)+SQR(n.z)); }
static inline void norm3d(Number3D *n) { float mod = len3d(*n); if (mod != 0.0f && mod != 1.0f) muls3d(n, 1.0/(double)mod); }
void rotx3d(Number3D *n, float angle);
void roty3d(Number3D *n, float angle);
void rotz3d(Number3D *n, float angle);
void trmx3d(Number3D *n, float *m);
static inline Number3D cross3d(Number3D v, Number3D w) { return (Number3D){(v.y*w.z)-(v.z*w.y), (v.z*w.x)-(v.x*w.z), (v.x*w.y)-(v.y*w.x)}; }
static inline float dot3d(Number3D v, Number3D w) { return (v.x*w.x+v.y*w.y+v.z*w.z); }
static inline float angle3d(Number3D v, Number3D w) { return acosf(dot3d(v, w)); }
static inline Number3D vec3d(Number3D p0, Number3D p1) { Number3D v = p1; sub3d(&v, p0); return v; }
static inline Number3D nvec3d(Number3D p0, Number3D p1) { Number3D v = p1; sub3d(&v, p0); norm3d(&v); return v; }
int isxtri3d(Line3D ray, Triangle3D plane, Number3D *where);
int isxquad3d(Line3D ray, Quad3D plane, Number3D *where);
int isxcube3d(Line3D ray, Cube3D cube, Number3D *where);

// Matrix functions
static inline void dupmx(float *m, const float *o) { memcpy((void*)m, (void*)o, sizeof(IDMAT)); }
static inline void idmx(float *m) { dupmx(m, IDMAT); }
void rotmx(float *m, Number3D vector, float angle);
void ortmx(float *m, Number3D euler);
float detmx(const float *m);
void mulmx(float *m, const float *v);
void invmx(float *m);
Quaternion quatmx(const float *m);

// Quaternion functions
static inline void addq(Quaternion *q, Quaternion v) { q->w += v.w; q->x += v.x; q->y += v.y; q->z += v.z; }
static inline void subq(Quaternion *q, Quaternion v) { q->w -= v.w; q->x -= v.x; q->y -= v.y; q->z -= v.z; }
void mulq(Quaternion *q, Quaternion v);
static inline void addsq(Quaternion *q, float v) { q->w += v; q->x += v; q->y += v; q->z += v; }
static inline void subsq(Quaternion *q, float v) { q->w -= v; q->x -= v; q->y -= v; q->z -= v; }
static inline void mulsq(Quaternion *q, float v) { q->w *= v; q->x *= v; q->y *= v; q->z *= v; }
static inline void divsq(Quaternion *q, float v) { q->w /= v; q->x /= v; q->y /= v; q->z /= v; }
static inline float magq(Quaternion q) { return sqrtf(SQR(q.w)+SQR(q.x)+SQR(q.y)+SQR(q.z)); }
static inline void normq(Quaternion *q) { mulsq(q, 1.0f/magq(*q)); }
static inline void conjq(Quaternion *q) { q->x = -q->x; q->y = -q->y; q->z = -q->z; }
static inline void invq(Quaternion *q) { float m = magq(*q); conjq(q); mulsq(q, 1.0f/m); }
static inline float dotq(Quaternion v, Quaternion w) { return (v.x * w.x) + (v.y * w.y) + (v.z * w.z) + (v.w * w.w); }
void rotq(Quaternion *q, Number3D vector, float angle);
void ortq(Quaternion *q, Number3D euler);
Number3D eulq(Quaternion q);
void matq(float *m, Quaternion q);
Quaternion slerpq(Quaternion v, Quaternion w, float t);

#endif /* MATH_H_ */
