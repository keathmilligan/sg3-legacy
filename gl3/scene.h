/* scene.c - Scene management
 * Scene graph management
 * Copyright 2012 Keath Milligan
 */

#ifndef SCENE_H_
#define SCENE_H_

#include "types.h"
#include "camera.h"
#include "texture.h"
#include "objects.h"
#include "effects.h"

#define GRID_LINES 50
#define AXIS_LINE_LEN 10.0f

// Light
typedef struct _Light {
    Color ambient;
    Color diffuse;
    Color specular;
    int directional;
    Number3D position;
    Number3D direction;
    Number3D spot_direction;
    float spot_exponent;
    float spot_cutoff;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
    int enabled;
} Light;

typedef struct _LightList {
    Light *light;
    struct _LightList *next;
    struct _LightList *prev;
} LightList;

// Skybox
typedef struct _Skybox {
    float distance;
    int fixed_proximity;
    Camera *camera;
    Object3DList *planes;
} Skybox;

typedef struct _SkyboxList {
    Skybox *skybox;
    struct _SkyboxList *next;
    struct _SkyboxList *prev;
} SkyboxList;

// Octree
typedef struct _OctreeNode {
    Number3D center;
    float half_width;
    Object3DList *objects;
    struct _OctreeNode *nodes[8];
} OctreeNode;

#ifdef __APPLE__
#define Scene _Scene
#endif

// Scene - container for all displayed objects
typedef struct _Scene {
    int viewport_width;
    int viewport_height;
    Number3D grid_floor_points[2][GRID_LINES][2];
    Number3D axis_lines_points[6];
    Camera *camera;
    LightList *lights;
    SkyboxList *skyboxes;
    Object3DList *background_objects;
    Object3DList *objects;
    EffectList *effects;
    int show_grid;
    int show_axis;
    int fog_enabled;
    int fog_mode;
    float fog_start;
    float fog_end;
    Color fog_color;
    float fog_density;
    OctreeNode *static_objects;
} Scene;

Scene *scene_create();
void scene_destroy(Scene *scene);
void scene_reshape_viewport(Scene *scene, int width, int height);
void scene_update(Scene *scene);
void scene_render(Scene *scene);
void scene_add_object(Scene *scene, Object3D *object);
void scene_remove_object(Scene *scene, Object3D *object);
void scene_clear_objects(Scene *scene);
void scene_add_background_object(Scene *scene, Object3D *object);
void scene_remove_background_object(Scene *scene, Object3D *object);
void scene_clear_background_objects(Scene *scene);
void scene_add_light(Scene *scene, Light *light);
void scene_remove_light(Scene *scene, Light *light);
void scene_clear_lights(Scene *scene);
void scene_add_skybox(Scene *scene, Skybox *skybox);
void scene_remove_skybox(Scene *scene, Skybox *skybox);
void scene_clear_skyboxes(Scene *scene);
void scene_add_effect(Scene *scene, Effect *effect);
void scene_remove_effect(Scene *scene, Effect *effect);
void scene_clear_effects(Scene *scene);
int scene_point_occluded(Scene *scene, Camera *camera, Number3D point);
Light *light_create();
void light_destroy(Light *light);
Skybox *skybox_create(const char *resources, const char *texture, float distance, Camera *camera);
void skybox_destroy(Skybox *skybox);
OctreeNode *octree_build(Number3D center, float radius, int depth, int max_depth);
void octree_destroy(OctreeNode *root);

#endif /* SCENE_H_ */
