/* objects.h - 3D objects
 * 3D object primitives
 * Copyright 2012 Keath Milligan
 */

#ifndef OBJECTS_H_
#define OBJECTS_H_

#include "types.h"
#include "camera.h"
#include "texture.h"

#define OBJ3D(x) ((Object3D*)x)

struct _Object3D;
typedef void (*Object3DFPtr)(struct _Object3D *);

// Axially-Aligned Bounding Box
typedef Cube3D AABB;

// Base 3D object
typedef struct _Object3D {
    Number3D position;
    Number3D scale;
    float rotation[16];
    Color color;
    Color ambient;
    Color diffuse;
    Color emission;
    Color specular;
    float shininess;
    int visible;
    int double_sided;
    int textures_enabled;
    int lighting_enabled;
    int materials_enabled;
    int smooth_shading;
    int wireframe;
    int draw_points;
    int additive_blend;
    int no_depth_test;
    float line_width;
    float point_size;
    Number3D *vertices;
    Number3D *normals;
    UV *uvs;
    Face *faces;
    int vertex_count;
    int face_count;
    Texture *texture;
    float radius;
    AABB aabb;
    int render_aabb;
    Object3DFPtr render;
    Object3DFPtr destroy;;
} Object3D;

// Object List
typedef struct _Object3DList {
    Object3D *object;
    struct _Object3DList *next;
    struct _Object3DList *prev;
} Object3DList;

// A group of objects rendered as one
typedef struct _ObjectGroup {
    Object3D _base;
    Object3DList *objects;
    int size;
} ObjectGroup;

// Simple wire-frame objects

typedef struct _WireLine {
    Object3D _base;
    Line3D line;
} WireLine;

typedef struct _WireTriangle {
    Object3D _base;
    float width;
    float height;
} WireTriangle;

typedef struct _WireRectangle {
    Object3D _base;
    float width;
    float height;
} WireRectangle;

typedef struct _WireCube {
    Object3D _base;
    float width;
    float height;
    float depth;
} WireCube;

typedef struct _Panel {
    Object3D _base;
    float width;
    float height;
} Panel;

typedef struct _Cube {
    Object3D _base;
    float width;
    float height;
    float depth;
} Cube;

typedef enum {
    BB_SCREEN_ALIGNED,
    BB_SPHERICAL
} BillboardType;

typedef struct _Billboard {
    Panel _base;
    BillboardType type;
    Camera *camera;
    int fixed_proximity;
} Billboard;

typedef struct _Model {
    Object3D _base;
    char *mesh_name;
} Model;

int object3d_ray_intersects(Object3D *object, Line3D ray, Number3D *where);
ObjectGroup *objectgroup_create();
void objectgroup_add_object(ObjectGroup *group, Object3D *object);
void objectgroup_remove_object(ObjectGroup *group, Object3D *object);
WireLine *wireline_create(Line3D line, Color color);
WireTriangle *wiretriangle_create(float width, float height, Color color);
WireRectangle *wirerectangle_create(float width, float height, Color color);
WireCube *wirecube_create(float width, float height, float depth, Color color);
Panel *panel_create(float width, float height, Texture *texture);
int panel_init(Panel *obj, float width, float height, Texture *texture);
Cube *cube_create(float width, float height, float depth, Texture *texture);
Billboard *billboard_create(float width, float height, Texture *texture, Camera *camera, BillboardType type);
Model *model_create(const char *resource_path, const char *mesh_name);

#endif /* OBJECTS_H_ */
