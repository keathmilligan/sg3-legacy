/* objects.c - 3D objects
 * 3D object primitives
 * Copyright 2012 Keath Milligan
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#if defined(__MINGW32__)
#include <malloc.h>
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
#include <log/log.h>
#include "objects.h"
#include "math.h"
#include "scene.h"

static void object3d_init(Object3D *object);
static void object3d_cleanup(Object3D *object);
static void object3d_render_setup(const Object3D *object, int orient);
static void object3d_render(const Object3D *object);
static void object3d_render_cleanup(const Object3D *object, int pop);
static int object3d_add_vertex(Object3D *object, Number3D coord, UV uv, Number3D normal);
static void object3d_add_face(Object3D *object, int a, int b, int c);
static void object3d_add_quad_face(Object3D *object, int ul, int ur, int lr, int ll);
static void object3d_build_aabb(Object3D *object);
static void object3d_calculate_radius(Object3D *object);
static void objectgroup_render(const ObjectGroup *group);
static void objectgroup_destroy(ObjectGroup *group);
static void wireline_render(const WireLine *obj);
static void wireline_destroy(WireLine *obj);
static void wirerectangle_render(const WireRectangle *obj);
static void wirerectangle_destroy(WireRectangle *obj);
static void wiretriangle_render(const WireTriangle *obj);
static void wiretriangle_destroy(WireTriangle *obj);
static void wirecube_render(const WireCube *obj);
static void wirecube_destroy(WireCube *obj);
static void panel_destroy(Panel *obj);
static void cube_destroy(Cube *obj);
static void model_destroy(Model *obj);
static void billboard_render(Billboard *obj);

static void object3d_init(Object3D *object) {
    SET3D(object->scale, 1.0f, 1.0f, 1.0f);
    idmx(object->rotation);
    SETCOLORF(object->color, 1.0f, 0.0f, 0.0f, 1.0f);
    SETCOLORF(object->ambient, 0.5f, 0.5f, 0.5f, 1.0f);
    SETCOLORF(object->diffuse, 0.5f, 0.5f, 0.5f, 1.0f);
    SETCOLORF(object->emission, 0.0f, 0.0f, 0.0f, 1.0f);
    SETCOLORF(object->specular, 0.0f, 0.0f, 0.0f, 1.0f);
    object->visible = TRUE;
    object->textures_enabled = TRUE;
    object->lighting_enabled = TRUE;
    object->materials_enabled = TRUE;
    object->smooth_shading = TRUE;
    object->line_width = 2.0f;
    object->point_size = 3.0f;
}

static void object3d_cleanup(Object3D *object) {
    if (object->texture) texture_destroy(object->texture);
    if (object->vertices) free(object->vertices);
    if (object->normals) free(object->normals);
    if (object->uvs) free(object->uvs);
    if (object->faces) free(object->faces);
}

static void object3d_render_setup(const Object3D *object, int orient) {
    if (object->no_depth_test) glDisable(GL_DEPTH_TEST);
    if (object->visible) {
        if (!object->wireframe && !object->draw_points) {
            if (object->additive_blend) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glNormalPointer(GL_FLOAT, 0, object->normals);
            glEnableClientState(GL_NORMAL_ARRAY);
            object->lighting_enabled? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
            object->smooth_shading? glShadeModel(GL_SMOOTH) : glShadeModel(GL_FLAT);
            if (object->materials_enabled) {
                glDisable(GL_COLOR_MATERIAL);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, COLORFA(object->ambient));
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, COLORFA(object->diffuse));
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, COLORFA(object->specular));
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, COLORFA(object->emission));
                glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &object->shininess);
            } else {
                glEnable(GL_COLOR_MATERIAL);
                glColor4f(COLORFL(object->color));
            }
            if (object->texture && object->textures_enabled) {
                texture_activate(object->texture);
                glTexCoordPointer(2, GL_FLOAT, 0, object->uvs);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            } else {
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }

            glFrontFace(GL_CCW);
            if (object->double_sided)
                glDisable(GL_CULL_FACE);
            else
                glEnable(GL_CULL_FACE);
        } else {
            glEnable(GL_COLOR_MATERIAL);
            glColor4f(COLORFL(object->color));
            glNormalPointer(GL_FLOAT, 0, object->normals);
            glDisable(GL_LIGHTING);
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glLineWidth(object->line_width);
            glPointSize(object->point_size);
        }
    }
    if (orient) {
        glPushMatrix();
        glTranslatef(object->position.x, object->position.y, object->position.z);
        glMultMatrixf(object->rotation);
        glScalef(object->scale.x, object->scale.y, object->scale.z);
    }
}

static void object3d_render(const Object3D *object) {
    object3d_render_setup(object, TRUE);
    glVertexPointer(3, GL_FLOAT, 0, object->vertices);
    int draw_mode;
    if (object->wireframe) {
        draw_mode = GL_LINES;
    } else if (object->draw_points) {
        draw_mode = GL_POINTS;
    } else {
        draw_mode = GL_TRIANGLES;
    }
    glDrawElements(draw_mode, object->face_count*3, GL_UNSIGNED_SHORT, object->faces);
    object3d_render_cleanup(object, TRUE);
    if (object->render_aabb) {
        Number3D verts[8] = {{object->aabb.min.x, object->aabb.min.y, object->aabb.max.z},
                             {object->aabb.max.x, object->aabb.min.y, object->aabb.max.z},
                             {object->aabb.max.x, object->aabb.min.y, object->aabb.min.z},
                             {object->aabb.min.x, object->aabb.min.y, object->aabb.min.z},
                             {object->aabb.min.x, object->aabb.max.y, object->aabb.max.z},
                             {object->aabb.max.x, object->aabb.max.y, object->aabb.max.z},
                             {object->aabb.max.x, object->aabb.max.y, object->aabb.min.z},
                             {object->aabb.min.x, object->aabb.max.y, object->aabb.min.z}};
        float rot[16];
        dupmx(rot, object->rotation);
        invmx(rot);
        int i;
        for (i=0; i<8; i++) {
            trmx3d(&verts[i], rot);
            mul3d(&verts[i], object->scale);
            add3d(&verts[i], object->position);
        }
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glColor4f(COLORFL(object->color));
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glLineWidth(object->line_width);
        glPointSize(object->point_size);
        glVertexPointer(3, GL_FLOAT, 0, verts);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glDrawArrays(GL_LINE_LOOP, 4, 4);
        Number3D v[8];
        v[0] = verts[0];
        v[1] = verts[4];
        v[2] = verts[1];
        v[3] = verts[5];
        v[4] = verts[2];
        v[5] = verts[6];
        v[6] = verts[3];
        v[7] = verts[7];
        glVertexPointer(3, GL_FLOAT, 0, v);
        glDrawArrays(GL_LINES, 0, 8);
    }
}

static void object3d_render_cleanup(const Object3D *object, int pop) {
    if (object->texture) texture_deactivate(object->texture);
    if (pop) glPopMatrix();
}

static int object3d_add_vertex(Object3D *object, Number3D coord, UV uv, Number3D normal) {
    object->vertices[object->vertex_count] = coord;
    object->uvs[object->vertex_count] = uv;
    object->normals[object->vertex_count] = normal;
    object->vertex_count++;
    return object->vertex_count-1;
}

static void object3d_add_face(Object3D *object, int a, int b, int c) {
    object->faces[object->face_count] = (Face){(short)a, (short)b, (short)c};
    object->face_count++;
}

static void object3d_add_quad_face(Object3D *object, int ul, int ur, int lr, int ll) {
    object3d_add_face(object, ul, lr, ur);
    object3d_add_face(object, ul, ll, lr);
}

int object3d_ray_intersects(Object3D *object, Line3D ray, Number3D *where) {
    if (object->face_count == 0) return FALSE;
    Number3D dx = nvec3d(ray.p0, object->position);
    if (dot3d(dx, ray.p1) < 0.0f) return FALSE;
    trmx3d(&ray.p0, object->rotation);
    trmx3d(&ray.p1, object->rotation);
    sub3d(&ray.p0, object->position);
    div3d(&ray.p0, object->scale);
    Number3D w;
    if (isxcube3d(ray, object->aabb, &w)) {
        int i;
        for (i=0; i<object->face_count; i++) {
            Triangle3D face;
            face.a = object->vertices[object->faces[i].a];
            face.b = object->vertices[object->faces[i].b];
            face.c = object->vertices[object->faces[i].c];
            Number3D n = cross3d(vec3d(face.a, face.b), vec3d(face.a, face.c));
            norm3d(&n);
            float a = dot3d(ray.p1, n);
            if (a < 0.0f) {
                if (isxtri3d(ray, face, where)) return TRUE;
            }
        }
    }
    return FALSE;
}

static void object3d_build_aabb(Object3D *object) {
    int i;
    for (i=0; i<object->vertex_count; i++) {
        if (object->vertices[i].x > object->aabb.max.x) object->aabb.max.x = object->vertices[i].x;
        if (object->vertices[i].x < object->aabb.min.x) object->aabb.min.x = object->vertices[i].x;
        if (object->vertices[i].y > object->aabb.max.y) object->aabb.max.y = object->vertices[i].y;
        if (object->vertices[i].y < object->aabb.min.y) object->aabb.min.y = object->vertices[i].y;
        if (object->vertices[i].z > object->aabb.max.z) object->aabb.max.z = object->vertices[i].z;
        if (object->vertices[i].z < object->aabb.min.z) object->aabb.min.z = object->vertices[i].z;
    }
    object3d_calculate_radius(object);
}

static void object3d_calculate_radius(Object3D *object) {
    Number3D verts[8] = {{object->aabb.min.x, object->aabb.min.y, object->aabb.max.z},
                         {object->aabb.max.x, object->aabb.min.y, object->aabb.max.z},
                         {object->aabb.max.x, object->aabb.min.y, object->aabb.min.z},
                         {object->aabb.min.x, object->aabb.min.y, object->aabb.min.z},
                         {object->aabb.min.x, object->aabb.max.y, object->aabb.max.z},
                         {object->aabb.max.x, object->aabb.max.y, object->aabb.max.z},
                         {object->aabb.max.x, object->aabb.max.y, object->aabb.min.z},
                         {object->aabb.min.x, object->aabb.max.y, object->aabb.min.z}};
    int i;
    for (i=0; i<8; i++) {
        float m = len3d(verts[i]);
        if (m > object->radius) object->radius = m;
    }
}

#define BASE ((Object3D*)group)

ObjectGroup *objectgroup_create() {
    ObjectGroup *group = calloc(1, sizeof(ObjectGroup));
    object3d_init(BASE);
    BASE->render = (Object3DFPtr)objectgroup_render;
    BASE->destroy = (Object3DFPtr)objectgroup_destroy;
    return group;
}

void objectgroup_add_object(ObjectGroup *group, Object3D *object) {
    Object3DList *e = malloc(sizeof(Object3DList));
    e->object = object;
    LL_APPEND(group->objects, e);
    group->size++;
}

void objectgroup_remove_object(ObjectGroup *group, Object3D *object) {
    Object3DList *e, *t;
    LL_FOREACH_SAFE(group->objects, e, t) {
        if (e->object == object) {
            LL_DELETE(group->objects, e);
        }
    }
}

static void objectgroup_render(const ObjectGroup *group) {
    Object3DList *e;
    if (BASE->visible) {
        glPushMatrix();
        glTranslatef(BASE->position.x, BASE->position.y, BASE->position.z);
        glMultMatrixf(BASE->rotation);
        glScalef(BASE->scale.x, BASE->scale.y, BASE->scale.z);
        LL_FOREACH(group->objects, e) {
            e->object->render(e->object);
        }
        glPopMatrix();
    }
}

static void objectgroup_destroy(ObjectGroup *group) {
    Object3DList *e, *t;
    LL_FOREACH_SAFE(group->objects, e, t) {
        e->object->destroy(e->object);
        free(e);
    }
    object3d_cleanup(BASE);
    free(group);
}

#undef BASE
#define BASE ((Object3D*)obj)

WireLine *wireline_create(Line3D line, Color color) {
    WireLine *obj = calloc(1, sizeof(WireLine));
    object3d_init(BASE);
    BASE->wireframe = TRUE;
    BASE->color = color;
    obj->line = line;
    BASE->vertices = malloc(sizeof(Line3D));
    BASE->vertex_count = 2;
    memcpy(BASE->vertices, &line, sizeof(Line3D));
    BASE->render = (Object3DFPtr)wireline_render;
    BASE->destroy = (Object3DFPtr)wireline_destroy;
    object3d_build_aabb(BASE);
    return obj;
}

static void wireline_render(const WireLine *obj) {
    object3d_render_setup(BASE, TRUE);
    glVertexPointer(3, GL_FLOAT, 0, BASE->vertices);
    glDrawArrays(GL_LINES, 0, 2);
    object3d_render_cleanup(BASE, TRUE);
}

static void wireline_destroy(WireLine *obj) {
    object3d_cleanup(BASE);
    free(obj);
}

WireTriangle *wiretriangle_create(float width, float height, Color color) {
    WireTriangle *obj = calloc(1, sizeof(WireTriangle));
    float w = width/2.0f;
    float h = height/2.0f;
    object3d_init(BASE);
    BASE->wireframe = TRUE;
    BASE->color = color;
    obj->width = width;
    obj->height = height;
    BASE->vertices = malloc(sizeof(Number3D)*3);
    BASE->vertex_count = 3;
    SET3D(BASE->vertices[0], w, -h, 0.0f);
    SET3D(BASE->vertices[1], 0.0f, h, 0.0f);
    SET3D(BASE->vertices[2], -w, -h, 0.0f);
    BASE->render = (Object3DFPtr)wiretriangle_render;
    BASE->destroy = (Object3DFPtr)wiretriangle_destroy;
    object3d_build_aabb(BASE);
    return obj;
}

static void wiretriangle_render(const WireTriangle *obj) {
    object3d_render_setup(BASE, TRUE);
    glVertexPointer(3, GL_FLOAT, 0, BASE->vertices);
    glDrawArrays(GL_LINE_LOOP, 0, 3);
    object3d_render_cleanup(BASE, TRUE);
}

static void wiretriangle_destroy(WireTriangle *obj) {
    object3d_cleanup(BASE);
    free(obj);
}

WireRectangle *wirerectangle_create(float width, float height, Color color) {
    WireRectangle *obj = calloc(1, sizeof(WireRectangle));
    float w = width/2.0f;
    float h = height/2.0f;
    object3d_init(BASE);
    BASE->wireframe = TRUE;
    BASE->color = color;
    obj->width = width;
    obj->height = height;
    BASE->vertices = malloc(sizeof(Number3D)*4);
    BASE->vertex_count = 4;
    SET3D(BASE->vertices[0], -w,  h, 0.0f);
    SET3D(BASE->vertices[1],  w,  h, 0.0f);
    SET3D(BASE->vertices[2],  w, -h, 0.0f);
    SET3D(BASE->vertices[3], -w, -h, 0.0f);
    BASE->render = (Object3DFPtr)wirerectangle_render;
    BASE->destroy = (Object3DFPtr)wirerectangle_destroy;
    object3d_build_aabb(BASE);
    return obj;
}

static void wirerectangle_render(const WireRectangle *obj) {
    object3d_render_setup(BASE, TRUE);
    glVertexPointer(3, GL_FLOAT, 0, BASE->vertices);
    glDrawArrays(GL_LINES, 0, 4);
    object3d_render_cleanup(BASE, TRUE);
}

static void wirerectangle_destroy(WireRectangle *obj) {
    object3d_cleanup(BASE);
    free(obj);
}

WireCube *wirecube_create(float width, float height, float depth, Color color) {
    WireCube *obj = calloc(1, sizeof(WireCube));
    float w = width/2.0f;
    float h = height/2.0f;
    float d = depth/2.0f;
    object3d_init(BASE);
    BASE->wireframe = TRUE;
    BASE->color = color;
    obj->width = width;
    obj->height = height;
    obj->depth = depth;
    BASE->vertices = malloc(sizeof(Number3D)*16);
    BASE->vertex_count = 16;
    SET3D(BASE->vertices[ 0], -w,  h, -d);
    SET3D(BASE->vertices[ 1],  w,  h, -d);
    SET3D(BASE->vertices[ 2],  w, -h, -d);
    SET3D(BASE->vertices[ 3], -w, -h, -d);
    SET3D(BASE->vertices[ 4], -w,  h,  d);
    SET3D(BASE->vertices[ 5],  w,  h,  d);
    SET3D(BASE->vertices[ 6],  w, -h,  d);
    SET3D(BASE->vertices[ 7], -w, -h,  d);
    SET3D(BASE->vertices[ 8], -w,  h,  d);
    SET3D(BASE->vertices[ 9], -w,  h, -d);
    SET3D(BASE->vertices[10], -w, -h,  d);
    SET3D(BASE->vertices[11], -w, -h, -d);
    SET3D(BASE->vertices[12],  w,  h,  d);
    SET3D(BASE->vertices[13],  w,  h, -d);
    SET3D(BASE->vertices[14],  w, -h,  d);
    SET3D(BASE->vertices[15],  w, -h, -d);
    BASE->render = (Object3DFPtr)wirecube_render;
    BASE->destroy = (Object3DFPtr)wirecube_destroy;
    object3d_build_aabb(BASE);
    return obj;
}

static void wirecube_render(const WireCube *obj) {
    object3d_render_setup(BASE, TRUE);
    glVertexPointer(3, GL_FLOAT, 0, BASE->vertices);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDrawArrays(GL_LINE_LOOP, 4, 4);
    glDrawArrays(GL_LINES, 8, 8);
    object3d_render_cleanup(BASE, TRUE);
}

static void wirecube_destroy(WireCube *obj) {
    object3d_cleanup(BASE);
    free(obj);
}

Panel *panel_create(float width, float height, Texture *texture) {
    Panel *obj = calloc(1, sizeof(Panel));
    panel_init(obj, width, height, texture);
    return obj;
}

int panel_init(Panel *obj, float width, float height, Texture *texture) {
    object3d_init(BASE);
    obj->width = width;
    obj->height = height;
    float w = width/2.0f;
    float h = height/2.0f;
    BASE->vertices = malloc(sizeof(Number3D)*4);
    BASE->uvs = malloc(sizeof(UV)*4);
    BASE->normals = malloc(sizeof(Number3D)*4);
    BASE->faces = malloc(sizeof(Face)*2);
    int ul, ur, lr, ll;
    ur = object3d_add_vertex(BASE, (Number3D){ w,  h, 0.0f}, (UV){1.0f, 0.0f}, (Number3D){0.0f, 0.0f, 1.0f});
    ul = object3d_add_vertex(BASE, (Number3D){-w,  h, 0.0f}, (UV){0.0f, 0.0f}, (Number3D){0.0f, 0.0f, 1.0f});
    ll = object3d_add_vertex(BASE, (Number3D){-w, -h, 0.0f}, (UV){0.0f, 1.0f}, (Number3D){0.0f, 0.0f, 1.0f});
    lr = object3d_add_vertex(BASE, (Number3D){ w, -h, 0.0f}, (UV){1.0f, 1.0f}, (Number3D){0.0f, 0.0f, 1.0f});
    object3d_add_quad_face(BASE, ul, ur, lr, ll);
    BASE->texture = texture;
    BASE->render = (Object3DFPtr)object3d_render;
    BASE->destroy = (Object3DFPtr)panel_destroy;
    object3d_build_aabb(BASE);
    return TRUE;
}

static void panel_destroy(Panel *obj) {
    object3d_cleanup(BASE);
    free(obj);
}

Cube *cube_create(float width, float height, float depth, Texture *texture) {
    Cube *obj = calloc(1, sizeof(Cube));
    object3d_init(BASE);
    obj->width = width;
    obj->height = height;
    obj->depth = depth;
    BASE->texture = texture;
    BASE->render = (Object3DFPtr)object3d_render;
    BASE->destroy = (Object3DFPtr)cube_destroy;
    BASE->vertices = malloc(sizeof(Number3D)*24);
    BASE->normals = malloc(sizeof(Number3D)*24);
    BASE->uvs = malloc(sizeof(Number3D)*24);
    BASE->faces = malloc(sizeof(Face)*12);
    float w = width/2.0f;
    float h = height/2.0f;
    float d = depth/2.0f;
    int ul, ur, ll, lr;
    ul = object3d_add_vertex(BASE, (Number3D){-w, +h, +d}, (UV){0.0f, 0.0f}, (Number3D){0.0f, 0.0f, 1.0f});
    ur = object3d_add_vertex(BASE, (Number3D){+w, +h, +d}, (UV){1.0f, 0.0f}, (Number3D){0.0f, 0.0f, 1.0f});
    lr = object3d_add_vertex(BASE, (Number3D){+w, -h, +d}, (UV){1.0f, 1.0f}, (Number3D){0.0f, 0.0f, 1.0f});
    ll = object3d_add_vertex(BASE, (Number3D){-w, -h, +d}, (UV){0.0f, 1.0f}, (Number3D){0.0f, 0.0f, 1.0f});
    object3d_add_quad_face(BASE, ul, ur, lr, ll);
    ul = object3d_add_vertex(BASE, (Number3D){+w, +h, +d}, (UV){0.0f, 0.0f}, (Number3D){1.0f, 0.0f, 0.0f});
    ur = object3d_add_vertex(BASE, (Number3D){+w, +h, -d}, (UV){1.0f, 0.0f}, (Number3D){1.0f, 0.0f, 0.0f});
    lr = object3d_add_vertex(BASE, (Number3D){+w, -h, -d}, (UV){1.0f, 1.0f}, (Number3D){1.0f, 0.0f, 0.0f});
    ll = object3d_add_vertex(BASE, (Number3D){+w, -h, +d}, (UV){0.0f, 1.0f}, (Number3D){1.0f, 0.0f, 0.0f});
    object3d_add_quad_face(BASE, ul, ur, lr, ll);
    ul = object3d_add_vertex(BASE, (Number3D){+w, +h, -d}, (UV){0.0f, 0.0f}, (Number3D){0.0f, 0.0f, -1.0f});
    ur = object3d_add_vertex(BASE, (Number3D){-w, +h, -d}, (UV){1.0f, 0.0f}, (Number3D){0.0f, 0.0f, -1.0f});
    lr = object3d_add_vertex(BASE, (Number3D){-w, -h, -d}, (UV){1.0f, 1.0f}, (Number3D){0.0f, 0.0f, -1.0f});
    ll = object3d_add_vertex(BASE, (Number3D){+w, -h, -d}, (UV){0.0f, 1.0f}, (Number3D){0.0f, 0.0f, -1.0f});
    object3d_add_quad_face(BASE, ul, ur, lr, ll);
    ul = object3d_add_vertex(BASE, (Number3D){-w, +h, -d}, (UV){0.0f, 0.0f}, (Number3D){-1.0f, 0.0f, 0.0f});
    ur = object3d_add_vertex(BASE, (Number3D){-w, +h, +d}, (UV){1.0f, 0.0f}, (Number3D){-1.0f, 0.0f, 0.0f});
    lr = object3d_add_vertex(BASE, (Number3D){-w, -h, +d}, (UV){1.0f, 1.0f}, (Number3D){-1.0f, 0.0f, 0.0f});
    ll = object3d_add_vertex(BASE, (Number3D){-w, -h, -d}, (UV){0.0f, 1.0f}, (Number3D){-1.0f, 0.0f, 0.0f});
    object3d_add_quad_face(BASE, ul, ur, lr, ll);
    ul = object3d_add_vertex(BASE, (Number3D){-w, +h, -d}, (UV){0.0f, 0.0f}, (Number3D){0.0f, 1.0f, 0.0f});
    ur = object3d_add_vertex(BASE, (Number3D){+w, +h, -d}, (UV){1.0f, 0.0f}, (Number3D){0.0f, 1.0f, 0.0f});
    lr = object3d_add_vertex(BASE, (Number3D){+w, +h, +d}, (UV){1.0f, 1.0f}, (Number3D){0.0f, 1.0f, 0.0f});
    ll = object3d_add_vertex(BASE, (Number3D){-w, +h, +d}, (UV){0.0f, 1.0f}, (Number3D){0.0f, 1.0f, 0.0f});
    object3d_add_quad_face(BASE, ul, ur, lr, ll);
    ul = object3d_add_vertex(BASE, (Number3D){-w, -h, +d}, (UV){0.0f, 0.0f}, (Number3D){0, -1.0f, 0.0f});
    ur = object3d_add_vertex(BASE, (Number3D){+w, -h, +d}, (UV){1.0f, 0.0f}, (Number3D){0, -1.0f, 0.0f});
    lr = object3d_add_vertex(BASE, (Number3D){+w, -h, -d}, (UV){1.0f, 1.0f}, (Number3D){0, -1.0f, 0.0f});
    ll = object3d_add_vertex(BASE, (Number3D){-w, -h, -d}, (UV){0.0f, 1.0f}, (Number3D){0, -1.0f, 0.0f});
    object3d_add_quad_face(BASE, ul, ur, lr, ll);
    object3d_build_aabb(BASE);
    return obj;
}

static void cube_destroy(Cube *obj) {
    object3d_cleanup(BASE);
    free(obj);
}

Billboard *billboard_create(float width, float height, Texture *texture, Camera *camera, BillboardType type) {
    Billboard *obj = calloc(1, sizeof(Billboard));
    panel_init((Panel*)obj, width, height, texture);
    BASE->render = (Object3DFPtr)billboard_render;
    BASE->materials_enabled = FALSE;
    BASE->color = COLOR(255, 255, 255, 255);
    obj->type = type;
    obj->camera = camera;
    object3d_build_aabb(BASE);
    return obj;
}

static void billboard_render(Billboard *obj) {
    object3d_render_setup(BASE, FALSE);
    glPushMatrix();
    if (obj->fixed_proximity)
        glTranslatef(NUM3DFL(obj->camera->position));
    switch(obj->type) {
    case BB_SCREEN_ALIGNED: {
        float m[16];
        glTranslatef(NUM3DFL(BASE->position));
        glGetFloatv(GL_MODELVIEW_MATRIX, m);
        m[1] = m[2] = m[4] = m[6] = m[8] = m[9] = 0.0f;
        m[0] = m[5] = m[10] = 1.0f;
        glLoadMatrixf(m);
        break; }
    case BB_SPHERICAL: {
        Number3D look = nvec3d(BASE->position, obj->camera->position);
        Number3D right = cross3d(obj->camera->up, look);
        norm3d(&right);
        look = cross3d(right, obj->camera->up);
        float m[16] = { NUM3DFL(right), 0.0f,
                        NUM3DFL(obj->camera->up), 0.0f,
                        NUM3DFL(look), 0.0f,
                        NUM3DFL(BASE->position), 1.0f };
        glMultMatrixf(m);
        break; }
    }
    glVertexPointer(3, GL_FLOAT, 0, BASE->vertices);
    glDrawElements(GL_TRIANGLES, BASE->face_count*3, GL_UNSIGNED_SHORT, BASE->faces);
    object3d_render_cleanup(BASE, TRUE);
}

Model *model_create(const char *resource_path, const char *mesh_name) {
    Model *obj = calloc(1, sizeof(Model));
    object3d_init(BASE);
    BASE->materials_enabled = TRUE;
    SETCOLORF(BASE->specular, 0.8f, 0.8f, 0.8f, 1.0f);
    BASE->shininess = 5.0f;
    obj->mesh_name = strdup(mesh_name);
    BASE->render = (Object3DFPtr)object3d_render;
    BASE->destroy = (Object3DFPtr)model_destroy;
    int pathlen = (int)(strlen(resource_path)+strlen(mesh_name)+5);
    char *texpath = alloca(pathlen);
    sprintf(texpath, "%s%s.png", resource_path, mesh_name);
    BASE->texture = texture_create(texpath, TRUE, TRUE);
    if (BASE->texture == NULL) {
        LOGERR("could not load texture: %s\n", texpath);
        //model_destroy(obj);
        //return NULL;
    }
    char *meshpath = alloca(pathlen);
    sprintf(meshpath, "%s%s.3ds", resource_path, mesh_name);
    FILE *mesh = fopen(meshpath, "rb");
    if (mesh == NULL) {
        LOGERR("%s not found\n", meshpath);
        model_destroy(obj);
        return NULL;
    }
    struct stat fst;
    fstat(fileno(mesh), &fst);
    unsigned short chunkid;
    unsigned int chunklen;
    int i;
    unsigned short uvcount = 0;
    unsigned char ch;
    while (ftell(mesh) < fst.st_size) {
        fread(&chunkid, 2, 1, mesh);
        fread(&chunklen, 4, 1, mesh);
        LOG("chunkid: %x len: %d\n", chunkid, chunklen);
        switch(chunkid) {
        case 0x4d4d:
        case 0x3d3d:
        case 0x4100:
            break;
        case 0x4000:
            i = 0;
            do {
                fread(&ch, 1, 1, mesh);
                i++;
            } while (ch != '\0' && i<20);
            break;
        case 0x4110:
            if (BASE->vertices != NULL) {
                LOGERR("%s: multiple vertex chunks not supported\n", meshpath);
                fclose(mesh);
                model_destroy(obj);
                return NULL;
            }
            fread(&BASE->vertex_count, 2, 1, mesh);
            BASE->vertices = malloc(sizeof(Number3D)*BASE->vertex_count);
            for (i=0; i<BASE->vertex_count; i++)
                fread(&BASE->vertices[i], sizeof(Number3D), 1, mesh);
            break;
        case 0x4120:
            if (BASE->faces != NULL) {
                LOGERR("%s: multiple face chunks not supported\n", meshpath);
                fclose(mesh);
                model_destroy(obj);
                return NULL;
            }
            fread(&BASE->face_count, 2, 1, mesh);
            BASE->faces = malloc(sizeof(Face)*BASE->face_count);
            for (i=0; i<BASE->face_count; i++) {
                fread(&BASE->faces[i], sizeof(Face), 1, mesh);
                fseek(mesh, 2, SEEK_CUR);
            }
            break;
        case 0x4140:
            if (BASE->uvs != NULL) {
                LOGERR("%s: multiple UV chunks not supported\n", meshpath);
                fclose(mesh);
                model_destroy(obj);
                return NULL;
            }
            fread(&uvcount, 2, 1, mesh);
            BASE->uvs = malloc(sizeof(UV)*uvcount);
            for (i=0; i<uvcount; i++)
                fread(&BASE->uvs[i], sizeof(UV), 1, mesh);
            break;
        default:
            fseek(mesh, chunklen-6, SEEK_CUR);
            break;
        }
    }
    fclose(mesh);
    if (BASE->vertex_count == 0 || BASE->face_count == 0 || uvcount != BASE->vertex_count) {
        LOGERR("%s: invalid counts: vertices=%d, uvs=%d, faces=%d\n", meshpath, BASE->vertex_count, uvcount, BASE->face_count);
        //model_destroy(obj);
        //return NULL;
    }
    LOG("%s: read vertices=%d, uvs=%d, faces=%d\n", meshpath, BASE->vertex_count, uvcount, BASE->face_count);
    BASE->normals = malloc(sizeof(Number3D)*BASE->vertex_count);
    Number3D *face_normals = alloca(sizeof(Number3D)*BASE->face_count);
    Number3D v1, v2, v3, n1, n2;
    for (i=0; i<BASE->face_count; i++) {
        v1 = BASE->vertices[BASE->faces[i].a];
        v2 = BASE->vertices[BASE->faces[i].b];
        v3 = BASE->vertices[BASE->faces[i].c];
        n1 = nvec3d(v1, v2);
        n2 = nvec3d(v1, v3);
        face_normals[i] = cross3d(n1, n2);
        norm3d(&face_normals[i]);
    }
    Number3D sum;
    int j;
    float max_x = 0.0f, min_x = 0.0f, max_y = 0.0f, min_y = 0.0f, max_z = 0.0f, min_z = 0.0f;
    for (i=0; i<BASE->vertex_count; i++) {
        sum = NUM3D(0.0f, 0.0f, 0.0f);
        for (j=0; j<BASE->face_count; j++) {
            if ((BASE->faces[j].a == i) ||
                (BASE->faces[j].b == i) ||
                (BASE->faces[j].c == i)) {
                add3d(&sum, face_normals[j]);
            }
        }
        norm3d(&sum);
        BASE->normals[i] = sum;
        if (BASE->vertices[i].x > max_x) max_x = BASE->vertices[i].x;
        if (BASE->vertices[i].x < min_x) min_x = BASE->vertices[i].x;
        if (BASE->vertices[i].y > max_y) max_y = BASE->vertices[i].y;
        if (BASE->vertices[i].y < min_y) min_y = BASE->vertices[i].y;
        if (BASE->vertices[i].z > max_z) max_z = BASE->vertices[i].z;
        if (BASE->vertices[i].z < min_z) min_z = BASE->vertices[i].z;
    }
    BASE->aabb.min.x = min_x;
    BASE->aabb.max.x = max_x;
    BASE->aabb.min.y = min_y;
    BASE->aabb.max.y = max_y;
    BASE->aabb.min.z = min_z;
    BASE->aabb.max.z = max_z;
    object3d_calculate_radius(BASE);
    return obj;
}

static void model_destroy(Model *obj) {
    object3d_cleanup(BASE);
    free(obj->mesh_name);
    free(obj);
}
