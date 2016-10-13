/* scene.c - Scene management
 * Scene graph management
 * Copyright 2012 Keath Milligan
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __MINGW32__
#include <malloc.h>
#endif

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

#include <unistd.h>
#include "scene.h"
#include "camera.h"
#include <log/log.h>
#include "math.h"

static void build_grid(Scene *scene);
static void build_axis(Scene *scene);
static void render_grid(Scene *scene);
static void render_axis(Scene *scene);
static void light_setup(Light *light, int id);
static void skybox_render(Skybox *skybox);

Scene *scene_create() {
    LOG("creating scene\n");
    Scene *scene = calloc(1, sizeof(Scene));
    scene->camera = camera_create(scene);
    scene->fog_start = 1.5f;
    scene->fog_end = 20.0f;
    scene->fog_color = COLORF(0.5f, 0.5f, 0.5f, 1.0f);
    scene->fog_density = 0.001f;
    scene_add_light(scene, light_create());
    build_grid(scene);
    build_axis(scene);
    //scene->static_objects = octree_build(NUM3D(0.0f, 0.0f, 0.0f), 150.0f, 7, 7);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);
    glDepthRange(0, 1.0f);
    glDepthMask(TRUE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return scene;
}

void scene_destroy(Scene *scene) {
    LOG("destroying scene %x\n", scene);
    Object3DList *eo, *to;
    LightList *el, *tl;
    SkyboxList *es, *ts;
    EffectList *ee, *te;
    LL_FOREACH_SAFE(scene->objects, eo, to) {
        LL_DELETE(scene->objects, eo);
        eo->object->destroy(eo->object);
        free(eo);
    }
    LL_FOREACH_SAFE(scene->background_objects, eo, to) {
        LL_DELETE(scene->background_objects, eo);
        eo->object->destroy(eo->object);
        free(eo);
    }
    LL_FOREACH_SAFE(scene->lights, el, tl) {
        LL_DELETE(scene->lights, el);
        light_destroy(el->light);
        free(el);
    }
    LL_FOREACH_SAFE(scene->skyboxes, es, ts) {
        LL_DELETE(scene->skyboxes, es);
        skybox_destroy(es->skybox);
        free(es);
    }
    LL_FOREACH_SAFE(scene->effects, ee, te) {
        LL_DELETE(scene->effects, ee);
        ee->effect->destroy(ee->effect);
        free(ee);
    }
    camera_destroy(scene->camera);
    if (scene->static_objects) octree_destroy(scene->static_objects);
    free(scene);
}

void scene_reshape_viewport(Scene *scene, int width, int height) {
    scene->viewport_width = width;
    scene->viewport_height = height;
    glViewport(0, 0, width, height);
    camera_reshape_viewport(scene->camera, width, height);
}

void scene_render(Scene *scene) {
    camera_update_view_frustum(scene->camera);
    int light_index = 0;
    LightList *le;
    SkyboxList *se;
    Object3DList *oe;
    EffectList *ee;
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    LL_FOREACH(scene->lights, le) {
        light_setup(le->light, light_index++);
    }
    LL_FOREACH(scene->skyboxes, se) {
        skybox_render(se->skybox);
    }
    glClear(GL_DEPTH_BUFFER_BIT);
    LL_FOREACH(scene->background_objects, oe) {
        oe->object->render(oe->object);
    }
    glClear(GL_DEPTH_BUFFER_BIT);
    LL_FOREACH(scene->effects, ee) {
        ee->effect->render(ee->effect, EF_BACKGROUND);
    }
    glClear(GL_DEPTH_BUFFER_BIT);
    if (scene->fog_enabled) {
        glFogf(GL_FOG_MODE, scene->fog_mode);
        glFogf(GL_FOG_START, scene->fog_start);
        glFogf(GL_FOG_END, scene->fog_end);
        glFogfv(GL_FOG_COLOR, COLORFA(scene->fog_color));
        glFogf(GL_FOG_DENSITY, scene->fog_density);
        glEnable(GL_FOG);
    } else {
        glDisable(GL_FOG);
    }
    LL_FOREACH(scene->objects, oe) {
        oe->object->render(oe->object);
    }
    LL_FOREACH(scene->effects, ee) {
        ee->effect->render(ee->effect, EF_SCENE);
    }
    camera_set_ortho(scene->camera);
    glClear(GL_DEPTH_BUFFER_BIT);
    LL_FOREACH(scene->effects, ee) {
        ee->effect->render(ee->effect, EF_OVERLAY);
    }
    camera_clear_ortho(scene->camera);
    if (scene->show_grid)
        render_grid(scene);
    if (scene->show_axis)
        render_axis(scene);
}

void scene_update(Scene *scene) {
    EffectList *ee;
    LL_FOREACH(scene->effects, ee) {
        ee->effect->update(scene, ee->effect);
    }
}

void scene_add_object(Scene *scene, Object3D *object) {
    LOG("adding object %x\n", object);
    Object3DList *e = malloc(sizeof(Object3DList));
    e->object = object;
    LL_APPEND(scene->objects, e);
}

void scene_remove_object(Scene *scene, Object3D *object) {
    Object3DList *e, *t;
    LL_FOREACH_SAFE(scene->objects, e, t) {
        if (e->object == object) {
            LL_DELETE(scene->objects, e);
            free(e);
            return;
        }
    }
    LOGERR("object not found for remove");
}

void scene_clear_objects(Scene *scene) {
    Object3DList *e, *t;
    LL_FOREACH_SAFE(scene->objects, e, t) {
        LL_DELETE(scene->objects, e);
        free(e);
    }
}

void scene_add_background_object(Scene *scene, Object3D *object) {
    LOG("adding background object %x\n", object);
    Object3DList *e = malloc(sizeof(Object3DList));
    e->object = object;
    LL_APPEND(scene->background_objects, e);
}

void scene_remove_background_object(Scene *scene, Object3D *object) {
    Object3DList *e, *t;
    LL_FOREACH_SAFE(scene->background_objects, e, t) {
        if (e->object == object) {
            LL_DELETE(scene->background_objects, e);
            free(e);
            return;
        }
    }
    LOGERR("background object not found for remove");
}

void scene_clear_background_objects(Scene *scene) {
    Object3DList *e, *t;
    LL_FOREACH_SAFE(scene->background_objects, e, t) {
        LL_DELETE(scene->background_objects, e);
        free(e);
    }
}

void scene_add_light(Scene *scene, Light *light) {
    LightList *e = malloc(sizeof(LightList));
    e->light = light;
    LL_APPEND(scene->lights, e);
}

void scene_remove_light(Scene *scene, Light *light) {
    LightList *e, *t;
    LL_FOREACH_SAFE(scene->lights, e, t) {
        if (e->light == light) {
            LL_DELETE(scene->lights, e);
            free(e);
            return;
        }
    }
    LOGERR("light not found for remove");
}

void scene_clear_lights(Scene *scene) {
    LightList *e, *t;
    LL_FOREACH_SAFE(scene->lights, e, t) {
        LL_DELETE(scene->lights, e);
        free(e);
    }
}

void scene_add_skybox(Scene *scene, Skybox *skybox) {
    SkyboxList *e = malloc(sizeof(SkyboxList));
    e->skybox = skybox;
    LL_APPEND(scene->skyboxes, e);
}

void scene_remove_skybox(Scene *scene, Skybox *skybox) {
    SkyboxList *e, *t;
    LL_FOREACH_SAFE(scene->skyboxes, e, t) {
        if (e->skybox == skybox) {
            LL_DELETE(scene->skyboxes, e);
            free(e);
            return;
        }
    }
    LOGERR("skybox not found for remove");
}

void scene_clear_skyboxes(Scene *scene) {
    SkyboxList *e, *t;
    LL_FOREACH_SAFE(scene->skyboxes, e, t) {
        LL_DELETE(scene->skyboxes, e);
        free(e);
    }
}

void scene_add_effect(Scene *scene, Effect *effect) {
    EffectList *e = malloc(sizeof(EffectList));
    e->effect = effect;
    LL_APPEND(scene->effects, e);
}

void scene_remove_effect(Scene *scene, Effect *effect) {
    EffectList *e, *t;
    LL_FOREACH_SAFE(scene->effects, e, t) {
        if (e->effect == effect) {
            LL_DELETE(scene->effects, e);
            free(e);
            return;
        }
    }
    LOGERR("effect not found for remove");
}

void scene_clear_effects(Scene *scene) {
    EffectList *e, *t;
    LL_FOREACH_SAFE(scene->effects, e, t) {
        LL_DELETE(scene->effects, e);
        free(e);
    }
}

int scene_point_occluded(Scene *scene, Camera *camera, Number3D point) {
    Line3D ray;
    ray.p0 = camera->position;
    ray.p1 = nvec3d(camera->position, point);
    Object3DList *o;
    Number3D w;
    LL_FOREACH(scene->objects, o) {
        if (object3d_ray_intersects(o->object, ray, &w))
            return TRUE;
    }
    return FALSE;
}

static void build_grid(Scene *scene) {
    float x = (float)(GRID_LINES/2);
    float y = 0.0f;
    float z = 0.0f;
    int i;
    for (i=0; i<GRID_LINES; i++) {
        y = -GRID_LINES/2+(float)i;
        SET3D(scene->grid_floor_points[0][i][0], -x, y, z);
        SET3D(scene->grid_floor_points[0][i][1], x, y, z);
    }
    y = GRID_LINES/2;
    for (i=0; i<GRID_LINES; i++) {
        x = -GRID_LINES/2+(float)i;
        SET3D(scene->grid_floor_points[1][i][0], x, y, z);
        SET3D(scene->grid_floor_points[1][i][1], x, -y, z);
    }
}

static void build_axis(Scene *scene) {
    SET3D(scene->axis_lines_points[0], -AXIS_LINE_LEN, 0.0f, 0.0f);
    SET3D(scene->axis_lines_points[1], AXIS_LINE_LEN, 0.0f, 0.0f);
    SET3D(scene->axis_lines_points[2], 0.0f, -AXIS_LINE_LEN, 0.0f);;
    SET3D(scene->axis_lines_points[3], 0.0f, AXIS_LINE_LEN, 0.0f);
    SET3D(scene->axis_lines_points[4], 0.0f, 0.0f, -AXIS_LINE_LEN);
    SET3D(scene->axis_lines_points[5], 0.0f, 0.0f, AXIS_LINE_LEN);
}

static void render_grid(Scene *scene) {
    //glClear(GL_DEPTH_BUFFER_BIT);
    //glEnable(GL_LINE_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glLineWidth(1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    glDisable(GL_LIGHTING);
    glScalef(3.0f, 3.0f, 3.0f);
    glVertexPointer(3, GL_FLOAT, 0, scene->grid_floor_points);
    glDrawArrays(GL_LINES, 0, GRID_LINES*4);
}

static void render_axis(Scene *scene) {
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_LIGHTING);
    glEnable(GL_LINE_SMOOTH);
    glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2.0f);
    //glClear(GL_DEPTH_BUFFER_BIT);
    glVertexPointer(3, GL_FLOAT, 0, scene->axis_lines_points);
    glColor4f(1.0f, 0.0f, 0.0f, 0.7f);
    glDrawArrays(GL_LINE_STRIP, 0, 2);
    glColor4f(0.0f, 1.0f, 0.0f, 0.7f);
    glDrawArrays(GL_LINE_STRIP, 2, 2);
    glColor4f(0.0f, 0.0f, 1.0f, 0.7f);
    glDrawArrays(GL_LINE_STRIP, 4, 2);
}

Light *light_create() {
    Light *light = calloc(1, sizeof(Light));
    SETCOLORF(light->ambient, 0.4f, 0.4f, 0.4f, 1.0f);
    SETCOLORF(light->diffuse, 0.5f, 0.5f, 0.5f, 1.0f);
    SETCOLORF(light->specular, 0.8f, 0.8f, 0.8f, 1.0f);
    light->directional = FALSE;
    SET3D(light->position, 0.0f, -50.0f, 10.0f);
    light->direction = nvec3d(NUM3D(0.0f, 0.0f, 0.0f), light->position);
    light->spot_direction = light->direction;
    light->spot_exponent = 0.0f;
    light->spot_cutoff = 180.0f;
    light->constant_attenuation = 1.0f;
    light->linear_attenuation = 0.0f;
    light->quadratic_attenuation = 0.0f;
    light->enabled = TRUE;
    return light;
}

void light_destroy(Light *light) {
    free(light);
}

Skybox *skybox_create(const char *resources, const char *texture, float distance, Camera *camera) {
    Skybox *skybox = calloc(1, sizeof(Skybox));
    skybox->distance = distance;
    skybox->fixed_proximity = TRUE;
    skybox->camera = camera;
    char *s = alloca(strlen(resources)+strlen(texture)+20);
    const char *sides[] = {"right1", "left2", "top3", "bottom4", "front5", "back6"};
    int i;
    for (i=0; i<6; i++) {
        sprintf(s, "%s%s_%s.png", resources, texture, sides[i]);
        Texture *t = NULL;
        if (access(s, R_OK) != -1) {
            t = texture_create(s, TRUE, FALSE);
            Panel *p = panel_create(distance*2.0f, distance*2.0f, t);
            OBJ3D(p)->lighting_enabled = FALSE;
            OBJ3D(p)->materials_enabled = FALSE;
            OBJ3D(p)->color = COLOR(255, 255, 255, 255);
            switch(i) {
            case 0: //right
                OBJ3D(p)->position.x = distance;
                ortmx(OBJ3D(p)->rotation, EULER3D(90, 0, -90));
                break;
            case 1: //left
                OBJ3D(p)->position.x = -distance;
                ortmx(OBJ3D(p)->rotation, EULER3D(90, 0, 90));
                break;
            case 2: //top
                OBJ3D(p)->position.z = distance;
                ortmx(OBJ3D(p)->rotation, EULER3D(180, 0, 0));
                break;
            case 3: //bottom
                OBJ3D(p)->position.z = -distance;
                break;
            case 4: //front
                OBJ3D(p)->position.y = distance;
                ortmx(OBJ3D(p)->rotation, EULER3D(90, 0, 0));
                break;
            case 5: //back
                OBJ3D(p)->position.y = -distance;
                ortmx(OBJ3D(p)->rotation, EULER3D(90, 0, 180));
                break;
            }
            Object3DList *pl = calloc(1, sizeof(Object3DList));
            pl->object = (Object3D*)p;
            LL_APPEND(skybox->planes, pl);
        }
    }
    return skybox;
}

void skybox_destroy(Skybox *skybox) {
    Object3DList *e, *t;
    LL_FOREACH_SAFE(skybox->planes, e, t) {
        e->object->destroy(e->object);
        free(e);
    }
    free(skybox);
}

static void skybox_render(Skybox *skybox) {
    Object3DList *pl;
    glPushMatrix();
    if (skybox->fixed_proximity) {
        glTranslatef(NUM3DFL(skybox->camera->position));
    }
    LL_FOREACH(skybox->planes, pl) {
        pl->object->render(pl->object);
    }
    glPopMatrix();
}

static void light_setup(Light *light, int id) {
    if (light->enabled) {
        glEnable(GL_LIGHT0+id);
        glLightfv(GL_LIGHT0+id, GL_AMBIENT, COLORFA(light->ambient));
        glLightfv(GL_LIGHT0+id, GL_DIFFUSE, COLORFA(light->diffuse));
        glLightfv(GL_LIGHT0+id, GL_SPECULAR, COLORFA(light->specular));
        if (light->directional)
            glLightfv(GL_LIGHT0+id, GL_POSITION, (float[]){NUM3DFL(light->direction), 0.0f});
        else
            glLightfv(GL_LIGHT0+id, GL_POSITION, (float[]){NUM3DFL(light->position), 1.0f});
        glLightfv(GL_LIGHT0+id, GL_SPOT_DIRECTION, (float[]){NUM3DFL(light->spot_direction), -1.0f});
        glLightfv(GL_LIGHT0+id, GL_SPOT_EXPONENT, &light->spot_exponent);
        glLightfv(GL_LIGHT0+id, GL_SPOT_CUTOFF, &light->spot_cutoff);
        glLightfv(GL_LIGHT0+id, GL_CONSTANT_ATTENUATION, &light->constant_attenuation);
        glLightfv(GL_LIGHT0+id, GL_LINEAR_ATTENUATION, &light->linear_attenuation);
        glLightfv(GL_LIGHT0+id, GL_QUADRATIC_ATTENUATION, &light->quadratic_attenuation);
    } else {
        glDisable(GL_LIGHT0+id);
    }
}

OctreeNode *octree_build(Number3D center, float radius, int depth, int max_depth) {
    static float min_radius = -1.0f;
    static int count = 0;
    if (depth < 0) return NULL;
    if (min_radius < 0.0f || radius < min_radius)
        min_radius = radius;
    OctreeNode *node = calloc(1, sizeof(OctreeNode));
    count++;
    node->center = center;
    node->half_width = radius;
    Number3D o;
    int i;
    float step = radius * 0.5f;
    for (i=0; i<8; i++) {
        o.x = ((i & 1) ? step : -step);
        o.y = ((i & 2) ? step : -step);
        o.z = ((i & 4) ? step : -step);
        add3d(&o, center);
        node->nodes[i] = octree_build(o, step, depth-1, max_depth);
    }
    if (depth == max_depth) {
        LOG("octree: min radius: %f, count: %d, size: %dK\n", min_radius, count, count*sizeof(OctreeNode)/1024);
    }
    return node;
}

void octree_destroy(OctreeNode *root) {
    int i;
    for (i=0; i<8; i++)
        if (root->nodes[i]) octree_destroy(root->nodes[i]);
    free(root);
}

