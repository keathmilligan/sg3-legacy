/* math.c - math functions
 * 2D and 3D math support
 * Copyright 2012 Keath Milligan
 */

#include <math.h>
#include <log/log.h>
#include "math.h"

// identity matrix
const float IDMAT[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, 0.0f,
                          0.0f, 0.0f, 0.0f, 1.0f };

// identity quaternion
const Quaternion IDQUAT = {1.0f, 0.0f, 0.0f, 0.0f};


void rot2d(Number2D *n, float angle) {
    float cosRY = cosf(angle);
    float sinRY = sinf(angle);
    Number2D t = *n;
    n->x = (t.x * cosRY) - (t.y * sinRY);
    n->y = (t.x * sinRY) + (t.y * cosRY);
}

int isx2d(Line2D a, Line2D b, int segment, Number2D *where) {
    if ((a.p1.y - a.p0.y) / (a.p1.x - a.p0.x) != (b.p1.y - b.p0.y) / (b.p1.x - b.p0.x)) {
        float d = (a.p1.x - a.p0.x) * (b.p1.y - b.p0.y) - (a.p1.y - a.p0.y) * (b.p1.x - b.p0.x);
        if (fabsf(d) < FLT_EPSILON) {
            return 0;
        }
        float AB = ((a.p0.y - b.p0.y) * (b.p1.x - b.p0.x) - (a.p0.x - b.p0.x) * (b.p1.y - b.p0.y)) / d;
        if (AB > 0.0f && (!segment || (segment && (AB < 1.0f)))) {
            float CD = ((a.p0.y - b.p0.y) * (a.p1.x - a.p0.x) - (a.p0.x - b.p0.x) * (a.p1.y - a.p0.y)) / d;
            if (CD > 0.0f && CD < 1.0f) {
                where->x = a.p0.x + AB * (a.p1.x - a.p0.x);
                where->y = a.p0.y + AB * (a.p1.y - a.p0.y);
                return 1;
            }
        }
    }
    return 0;
}

void rotx3d(Number3D *n, float angle) {
    float cosRY = cosf(angle);
    float sinRY = sinf(angle);
    Number3D t = *n;
    n->y = (t.y * cosRY) - (t.z * sinRY);
    n->z = (t.y * sinRY) + (t.z * cosRY);
}

void roty3d(Number3D *n, float angle) {
    float cosRY = cosf(angle);
    float sinRY = sinf(angle);
    Number3D t = *n;
    n->x = (t.x * cosRY) + (t.z * sinRY);
    n->z = (t.x * -sinRY) + (t.z * cosRY);
}

void rotz3d(Number3D *n, float angle) {
    float cosRY = cosf(angle);
    float sinRY = sinf(angle);
    Number3D t = *n;
    n->x = (t.x * cosRY) - (t.y * sinRY);
    n->y = (t.x * sinRY) + (t.y * cosRY);
}

void trmx3d(Number3D *n, float *m) {
    Number3D v = *n;
    n->x = m[M00]*v.x+m[M01]*v.y+m[M02]*v.z;
    n->y = m[M10]*v.x+m[M11]*v.y+m[M12]*v.z;
    n->z = m[M20]*v.x+m[M21]*v.y+m[M22]*v.z;
}

int isxtri3d(Line3D ray, Triangle3D plane, Number3D *where) {
/*  Number3D e1 = vec3d(plane.a, plane.b);
    Number3D e2 = vec3d(plane.a, plane.c);
    Number3D s1 = cross3d(ray.p1, e2);
    float d = dot3d(e1, s1);
    if (close_enough(d, 0.0f))
        return FALSE;
    d = 1.0f/d;
    Number3D dist = vec3d(plane.a, ray.p0);
    float u = dot3d(dist, s1)*d;
    if (u < 0.0f || u > 1.0f)
        return FALSE;
    Number3D s2 = cross3d(dist, e1);
    float v = dot3d(ray.p1, s2)*d;
    if (v < 0.0f || u+v > 1.0f)
        return FALSE;
    float t = dot3d(e2, s2)*d;
    if (t > 0.00001) {
        *where = ray.p1;
        muls3d(where, t);
        add3d(where, ray.p0);
        return TRUE;
    }
    return FALSE;
*/
    Number3D pa = vec3d(plane.a, ray.p0);
    Number3D pb = vec3d(plane.b, ray.p0);
    Number3D pc = vec3d(plane.c, ray.p0);
    Number3D m = cross3d(ray.p1, pc);
    float u = dot3d(pb, m);
    if (u < 0.0f) return FALSE;
    float v = -dot3d(pa, m);
    if (v < 0.0f) return FALSE;
    float w = dot3d(ray.p1, cross3d(pb, pa));
    if (w < 0.0f) return FALSE;
    float d = 1.0f/(u+v+w);
    u *= d;
    v *= d;
    w *= d;
    where->x = (u*plane.a.x+v*plane.b.x+w*plane.c.x);
    where->y = (u*plane.a.y+v*plane.b.y+w*plane.c.y);
    where->z = (u*plane.a.z+v*plane.b.z+w*plane.c.z);
    return TRUE;
}

int isxquad3d(Line3D ray, Quad3D plane, Number3D *where) {
    Number3D pa = vec3d(plane.a, ray.p0);
    Number3D pb = vec3d(plane.b, ray.p0);
    Number3D pc = vec3d(plane.c, ray.p0);
    Number3D m = cross3d(pc, ray.p1);
    float u, v, w;
    v = dot3d(pa, m);
    if (v >= 0.0f) {
        u = -dot3d(pb, m);
        if (u < 0.0f) return FALSE;
        w = dot3d(ray.p0, cross3d(pb, pa));
        if (w < 0.0f) return FALSE;
    } else {
        Number3D pd = vec3d(plane.d, ray.p0);
        u = dot3d(pd, m);
        if (u < 0.0f) return FALSE;
        w = dot3d(ray.p0, cross3d(pa, pd));
        if (w < 0.0f) return FALSE;
        v = -v;
    }
    float denom = 1.0f/(u+v+w);
    u *= denom;
    v *= denom;
    w *= denom;
    where->x = (u*plane.a.x+v*plane.b.x+w*plane.c.x);
    where->y = (u*plane.a.y+v*plane.b.y+w*plane.c.y);
    where->z = (u*plane.a.z+v*plane.b.z+w*plane.c.z);
    return TRUE;
}

int isxcube3d(Line3D ray, Cube3D cube, Number3D *where) {
    float tmin = 0.0f;
    float tmax = FLT_MAX;
    int i;
    for (i=0; i<3; i++) {
        if (fabs(FA(ray.p1)[i]) < FLT_EPSILON) {
            if (FA(ray.p0)[i] < FA(cube.min)[i] || FA(ray.p0)[i] > FA(cube.max)[i]) return FALSE;
        } else {
            float ood = 1.0f/FA(ray.p1)[i];
            float t1 = (FA(cube.min)[i]-FA(ray.p0)[i])*ood;
            float t2 = (FA(cube.max)[i]-FA(ray.p0)[i])*ood;
            if (t1 > t2) SWAPF(t1, t2);
            if (t1 > tmin) tmin = t1;
            if (t2 < tmax) tmax = t2;
            if (tmin > tmax) return FALSE;
        }
    }
    *where = ray.p1;
    muls3d(where, tmin);
    add3d(where, ray.p1);
    return TRUE;
}

void rotmx(float *m, Number3D vector, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f-c;
    m[0] = (vector.x*vector.x)*t+c;
    m[1] = (vector.x*vector.y)*t+(vector.z*s);
    m[2] = (vector.x*vector.z)*t-(vector.y*s);
    m[4] = (vector.y*vector.x)*t-(vector.z*s);
    m[5] = (vector.y*vector.y)*t+c;
    m[6] = (vector.y*vector.z)*t+(vector.x*s);
    m[8] = (vector.z*vector.x)*t+(vector.y*s);
    m[9] = (vector.z*vector.y)*t-(vector.x*s);
    m[10] = (vector.z*vector.z)*t+c;
}

void ortmx(float *m, Number3D euler) {
    float h = -euler.z;
    float p = -euler.x;
    float r = -euler.y;
    float ch = cosf(h);
    float sh = sinf(h);
    float cp = cosf(p);
    float sp = sinf(p);
    float cr = cosf(r);
    float sr = sinf(r);
    m[0] = ch * cr;
    m[1] = sh*sp - ch*sr*cp;
    m[2] = ch*sr*sp + sh*cp;
    m[4] = sr;
    m[5] = cr*cp;
    m[6] = -cr*sp;
    m[8] = -sh*cr;
    m[9] = sh*sr*cp + ch*sp;
    m[10] = -sh*sr*sp + ch*cp;
}

float detmx(const float *m) {
    return    (m[M00] * m[M11] - m[M10] * m[M01])
            * (m[M22] * m[M33] - m[M32] * m[M23])
            - (m[M00] * m[M21] - m[M20] * m[M01])
            * (m[M12] * m[M33] - m[M32] * m[M13])
            + (m[M00] * m[M31] - m[M30] * m[M01])
            * (m[M12] * m[M23] - m[M22] * m[M13])
            + (m[M10] * m[M21] - m[M20] * m[M11])
            * (m[M02] * m[M33] - m[M32] * m[M03])
            - (m[M10] * m[M31] - m[M30] * m[M11])
            * (m[M02] * m[M23] - m[M22] * m[M03])
            + (m[M20] * m[M31] - m[M30] * m[M21])
            * (m[M02] * m[M13] - m[M12] * m[M03]);
}

void mulmx(float *m, const float *v) {
    float t[16];
    dupmx(t, m);

    m[M00] = (t[M00] * v[M00]) + (t[M01] * v[M10]) + (t[M02] * v[M20]) + (t[M03] * v[M30]);
    m[M01] = (t[M00] * v[M01]) + (t[M01] * v[M11]) + (t[M02] * v[M21]) + (t[M03] * v[M31]);
    m[M02] = (t[M00] * v[M02]) + (t[M01] * v[M12]) + (t[M02] * v[M22]) + (t[M03] * v[M32]);
    m[M03] = (t[M00] * v[M03]) + (t[M01] * v[M13]) + (t[M02] * v[M23]) + (t[M03] * v[M33]);

    m[M10] = (t[M10] * v[M00]) + (t[M11] * v[M10]) + (t[M12] * v[M20]) + (t[M13] * v[M30]);
    m[M11] = (t[M10] * v[M01]) + (t[M11] * v[M11]) + (t[M12] * v[M21]) + (t[M13] * v[M31]);
    m[M12] = (t[M10] * v[M02]) + (t[M11] * v[M12]) + (t[M12] * v[M22]) + (t[M13] * v[M32]);
    m[M13] = (t[M10] * v[M03]) + (t[M11] * v[M13]) + (t[M12] * v[M23]) + (t[M13] * v[M33]);

    m[M20] = (t[M20] * v[M00]) + (t[M21] * v[M10]) + (t[M22] * v[M20]) + (t[M23] * v[M30]);
    m[M21] = (t[M20] * v[M01]) + (t[M21] * v[M11]) + (t[M22] * v[M21]) + (t[M23] * v[M31]);
    m[M22] = (t[M20] * v[M02]) + (t[M21] * v[M12]) + (t[M22] * v[M22]) + (t[M23] * v[M32]);
    m[M23] = (t[M20] * v[M03]) + (t[M21] * v[M13]) + (t[M22] * v[M23]) + (t[M23] * v[M33]);

    m[M30] = (t[M30] * v[M00]) + (t[M31] * v[M10]) + (t[M32] * v[M20]) + (t[M33] * v[M30]);
    m[M31] = (t[M30] * v[M01]) + (t[M31] * v[M11]) + (t[M32] * v[M21]) + (t[M33] * v[M31]);
    m[M32] = (t[M30] * v[M02]) + (t[M31] * v[M12]) + (t[M32] * v[M22]) + (t[M33] * v[M32]);
    m[M33] = (t[M30] * v[M03]) + (t[M31] * v[M13]) + (t[M32] * v[M23]) + (t[M33] * v[M33]);
}

void invmx(float *m) {
    float d = detmx(m);
    if (close_enough(d, 0.0f)) {
        idmx(m);
    } else {
        float t[16];
        dupmx(t, m);
        d = 1.0f / d;
        m[M00] = d * (t[M11] * (t[M22] * t[M33] - t[M32] * t[M23]) + t[M21] * (t[M32] * t[M13] - t[M12] * t[M33]) + t[M31] * (t[M12] * t[M23] - t[M22] * t[M13]));
        m[M10] = d * (t[M12] * (t[M20] * t[M33] - t[M30] * t[M23]) + t[M22] * (t[M30] * t[M13] - t[M10] * t[M33]) + t[M32] * (t[M10] * t[M23] - t[M20] * t[M13]));
        m[M20] = d * (t[M13] * (t[M20] * t[M31] - t[M30] * t[M21]) + t[M23] * (t[M30] * t[M11] - t[M10] * t[M31]) + t[M33] * (t[M10] * t[M21] - t[M20] * t[M11]));
        m[M30] = d * (t[M10] * (t[M31] * t[M22] - t[M21] * t[M32]) + t[M20] * (t[M11] * t[M32] - t[M31] * t[M12]) + t[M30] * (t[M21] * t[M12] - t[M11] * t[M22]));

        m[M01] = d * (t[M21] * (t[M02] * t[M33] - t[M32] * t[M03]) + t[M31] * (t[M22] * t[M03] - t[M02] * t[M23]) + t[M01] * (t[M32] * t[M23] - t[M22] * t[M33]));
        m[M11] = d * (t[M22] * (t[M00] * t[M33] - t[M30] * t[M03]) + t[M32] * (t[M20] * t[M03] - t[M00] * t[M23]) + t[M02] * (t[M30] * t[M23] - t[M20] * t[M33]));
        m[M21] = d * (t[M23] * (t[M00] * t[M31] - t[M30] * t[M01]) + t[M33] * (t[M20] * t[M01] - t[M00] * t[M21]) + t[M03] * (t[M30] * t[M21] - t[M20] * t[M31]));
        m[M31] = d * (t[M20] * (t[M31] * t[M02] - t[M01] * t[M32]) + t[M30] * (t[M01] * t[M22] - t[M21] * t[M02]) + t[M00] * (t[M21] * t[M32] - t[M31] * t[M22]));

        m[M02] = d * (t[M31] * (t[M02] * t[M13] - t[M12] * t[M03]) + t[M01] * (t[M12] * t[M33] - t[M32] * t[M13]) + t[M11] * (t[M32] * t[M03] - t[M02] * t[M33]));
        m[M12] = d * (t[M32] * (t[M00] * t[M13] - t[M10] * t[M03]) + t[M02] * (t[M10] * t[M33] - t[M30] * t[M13]) + t[M12] * (t[M30] * t[M03] - t[M00] * t[M33]));
        m[M22] = d * (t[M33] * (t[M00] * t[M11] - t[M10] * t[M01]) + t[M03] * (t[M10] * t[M31] - t[M30] * t[M11]) + t[M13] * (t[M30] * t[M01] - t[M00] * t[M31]));
        m[M32] = d * (t[M30] * (t[M11] * t[M02] - t[M01] * t[M12]) + t[M00] * (t[M31] * t[M12] - t[M11] * t[M32]) + t[M10] * (t[M01] * t[M32] - t[M31] * t[M02]));

        m[M03] = d * (t[M01] * (t[M22] * t[M13] - t[M12] * t[M23]) + t[M11] * (t[M02] * t[M23] - t[M22] * t[M03]) + t[M21] * (t[M12] * t[M03] - t[M02] * t[M13]));
        m[M13] = d * (t[M02] * (t[M20] * t[M13] - t[M10] * t[M23]) + t[M12] * (t[M00] * t[M23] - t[M20] * t[M03]) + t[M22] * (t[M10] * t[M03] - t[M00] * t[M13]));
        m[M23] = d * (t[M03] * (t[M20] * t[M11] - t[M10] * t[M21]) + t[M13] * (t[M00] * t[M21] - t[M20] * t[M01]) + t[M23] * (t[M10] * t[M01] - t[M00] * t[M11]));
        m[M33] = d * (t[M00] * (t[M11] * t[M22] - t[M21] * t[M12]) + t[M10] * (t[M21] * t[M02] - t[M01] * t[M22]) + t[M20] * (t[M01] * t[M12] - t[M11] * t[M02]));
    }
}

void mulq(Quaternion *q, Quaternion v) {
    Quaternion t = *q;
    q->w = (t.w*v.w)-(t.x*v.x)-(t.y*v.y)-(t.z*v.z);
    q->x = (t.w*v.x)+(t.x*v.w)-(t.y*v.z)+(t.z*v.y);
    q->y = (t.w*v.y)+(t.x*v.z)+(t.y*v.w)-(t.z*v.x);
    q->z = (t.w*v.z)-(t.x*v.y)+(t.y*v.x)+(t.z*v.w);
}

void rotq(Quaternion *q, Number3D vector, float angle) {
    float ht = angle*0.5f;
    float s = sinf(ht);
    float mod = len3d(vector);
    if (mod != 0 && mod != 1) {
        mod = 1/mod;
        vector.x *= mod;
        vector.y *= mod;
        vector.z *= mod;
    }
    q->w = cosf(ht);
    q->x = vector.x*s;
    q->y = vector.y*s;
    q->z = vector.z*s;
}

void ortq(Quaternion *q, Number3D euler) {
    float h = DEG2RAD(euler.y)/2;
    float p = DEG2RAD(euler.z)/2;
    float r = DEG2RAD(euler.x)/2;
    float s1 = sinf(h);
    float s2 = sinf(p);
    float s3 = sinf(r);
    float c1 = cosf(h);
    float c2 = cosf(p);
    float c3 = cosf(r);
    float c1c2 = c1*c2;
    float s1s2 = s1*s2;
    q->w = c1c2*c3-s1s2*s3;
    q->x = c1c2*s3+s1s2*c3;
    q->y = s1*c2*c3+c1*s2*s3;
    q->z = c1*s2*c3-s1*c2*s3;
    normq(q);
}

Number3D eulq(Quaternion q) {
    float test = q.x*q.y+q.z*q.w;
    // handle singularities
    if (test > 0.4999f)
        return (Number3D){ RAD2DEG(PI/2.0f),
                           RAD2DEG(2.0f*atan2f(q.x, q.w)),
                           0.0f };
    if (test < -0.4999f)
        return (Number3D){ RAD2DEG(-PI/2),
                           RAD2DEG(-2.0f*atan2f(q.x, q.w)),
                           0.0f };
    float sqw = q.w * q.w;
    float sqx = q.x * q.x;
    float sqy = q.y * q.y;
    float sqz = q.z * q.z;
    return (Number3D){ RAD2DEG(asinf(2*test)),
                       RAD2DEG(atan2f(2*q.y*q.w-2*q.x*q.z, sqx-sqy-sqz+sqw)),
                       RAD2DEG(atan2f(2*q.x*q.w-2*q.y*q.z, -sqx+sqy-sqz+sqw)) };
}

void matq(float *m, Quaternion q) {
    m[0] = 1.0f-2.0f*(q.y*q.y+q.z*q.z);
    m[1] = 2.0f*(q.x*q.y+q.z*q.w);
    m[2] = 2.0f*(q.x*q.z-q.y*q.w);
    m[3] = 0.0f;
    m[4] = 2.0f*(q.x*q.y-q.z*q.w);
    m[5] = 1.0f-2.0f*(q.x*q.x+q.z*q.z);
    m[6] = 2.0f*(q.z*q.y+q.x*q.w);
    m[7] = 0.0f;
    m[8] = 2.0f*(q.x*q.z+q.y*q.w);
    m[9] = 2.0f*(q.y*q.z-q.x*q.w);
    m[10] = 1.0f-2.0f*(q.x*q.x+q.y*q.y);
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

Quaternion quatmx(const float *m) {
    float w, x, y, z;
    float t = m[M00] + m[M11] + m[M22];
    if (t >= 0) {
        float s = sqrtf(t+1);
        w = 0.5f * s;
        s = 0.5f / s;
        x = (m[M21] - m[M12]) * s;
        y = (m[M02] - m[M20]) * s;
        z = (m[M10] - m[M01]) * s;
    } else if ((m[M00] > m[M11]) && (m[M00] > m[M22])) {
        float s = sqrtf(1.0f + m[M00] - m[M11] - m[M22]);
        x = s * 0.5f; // |x| >= .5
        s = 0.5f / s;
        y = (m[M10] + m[M01]) * s;
        z = (m[M02] + m[M20]) * s;
        w = (m[M21] - m[M12]) * s;
    } else if (m[M11] > m[M22]) {
        float s = sqrtf(1.0f + m[M11] - m[M00] - m[M22]);
        y = s * 0.5f;
        s = 0.5f / s;
        x = (m[M10] + m[M01]) * s;
        z = (m[M21] + m[M12]) * s;
        w = (m[M02] - m[M20]) * s;
    } else {
        float s = sqrtf(1.0f + m[M22] - m[M00] - m[M11]);
        z = s * 0.5f;
        s = 0.5f / s;
        x = (m[M02] + m[M20]) * s;
        y = (m[M21] + m[M12]) * s;
        w = (m[M10] - m[M01]) * s;
    }
    return (Quaternion){ w, x, y, z };
}

Quaternion slerpq(Quaternion v, Quaternion w, float t) {
    Quaternion o = w;
    if (v.x == o.x && v.y == o.y && v.z == o.z && v.w == o.w) {
        return v;
    }
    float result = dotq(v, o);
    if (result < 0.0f) {
        o.x = -o.x;
        o.y = -o.y;
        o.z = -o.z;
        o.w = -o.w;
        result = -result;
    }
    float scale0 = 1 - t;
    float scale1 = t;
    if ((1 - result) > 0.1f) {
        float theta = acosf(result);
        float invSinTheta = 1.0f / sinf(theta);
        scale0 = sinf((1 - t) * theta) * invSinTheta;
        scale1 = sinf((t * theta)) * invSinTheta;
    }
    return (Quaternion) { (scale0 * v.w) + (scale1 * o.w),
                          (scale0 * v.x) + (scale1 * o.x),
                          (scale0 * v.y) + (scale1 * o.y),
                          (scale0 * v.z) + (scale1 * o.z) };
}
