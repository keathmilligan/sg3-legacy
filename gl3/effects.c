/* effects.c - special efforts support
 * Lens flares, explosions and other visual effects
 * Copyright 2012 Keath Milligan
 */

#ifdef __MINGW32__
#include <malloc.h>
#endif
#include <stdio.h>

#include <log/log.h>

#include "gl.h"
#include "effects.h"
#include "objects.h"
#include "math.h"
#include "camera.h"
#include "scene.h"

static void effect_init(Effect *effect, Camera *camera);
static void effect_cleanup(Effect *effect);
static void lensflare_update(Scene *scene, LensFlare *flare);
static void lensflare_render(LensFlare *flare, EffectRenderLevel level);
static void lensflare_render_element(LensFlare *flare, Texture *texture, float x, float y, float scale, float rotation, Color color);

static void effect_init(Effect *effect, Camera *camera) {
    effect->camera = camera;
}

static void effect_cleanup(Effect *effect) {
}

LensFlare *lensflare_create(const char *resources, Camera *camera, Number3D light_position, float light_radius) {
    LensFlare *flare = calloc(1, sizeof(LensFlare));
    effect_init(EFFECT(flare), camera);
    EFFECT(flare)->update = (EffectUpdateFuncPtr)lensflare_update;
    EFFECT(flare)->render = (EffectRenderFuncPtr)lensflare_render;
    EFFECT(flare)->destroy = (EffectDestroyFuncPtr)lensflare_destroy;
    flare->light_position = light_position;
    flare->light_radius = light_radius;
    flare->_streaks = texture_create(resources, "streaks.png", TRUE, FALSE);
    flare->_halo = texture_create(resources, "halo.png", TRUE, FALSE);
    flare->_glow = texture_create(resources, "glow.png", TRUE, FALSE);
    return flare;
}

void lensflare_destroy(LensFlare *flare) {
    effect_cleanup(EFFECT(flare));
    texture_destroy(flare->_streaks);
    texture_destroy(flare->_halo);
    texture_destroy(flare->_glow);
    free(flare);
}

static void lensflare_update(Scene *scene, LensFlare *flare) {
    int occluded = flare->_occluded;
    Number3D pos = flare->light_position;
    add3d(&pos, EFFECT(flare)->camera->position);
    int visible = camera_sphere_in_frustrum(EFFECT(flare)->camera, pos, flare->light_radius);
    if (visible) {
        occluded = scene_point_occluded(scene, EFFECT(flare)->camera, pos);
        if (!occluded)
            flare->_screen_position = camera_screen_coords(EFFECT(flare)->camera, pos);
    }
    //if (visible != flare->_visible)
    //    LOG("%s\n", visible? "light visible" : "light not visible");
    //if (occluded != flare->_occluded)
    //    LOG("%s\n", occluded? "light occluded" : "light not occluded");
    flare->_visible = visible;
    flare->_occluded = occluded;
}

static void lensflare_render(LensFlare *flare, EffectRenderLevel level) {
    switch(level) {
    case EF_BACKGROUND: {
        break; }
    case EF_SCENE: {
        break; }
    case EF_OVERLAY: {
        if (flare->_visible && !flare->_occluded) {
            glDisable(GL_LIGHTING);
            glEnable(GL_COLOR_MATERIAL);
            glDisableClientState(GL_NORMAL_ARRAY);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDisable(GL_DEPTH_TEST);
            static Number2D vertices[4] = { {-0.5f, -0.5f}, {-0.5f, 0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f} };
            static UV uvs[4]  = { {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f} };
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, vertices);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, uvs);
            float cx = (float)EFFECT(flare)->camera->screen_width*0.5f;
            float cy = (float)EFFECT(flare)->camera->screen_height*0.5f;
            float vx = cx-flare->_screen_position.x;
            float vy = cy-flare->_screen_position.y;
            float len = sqrtf(SQR(vx)+SQR(vy));
            vx = vx/len;
            vy = vy/len;
            float px = flare->_screen_position.x;
            float py = flare->_screen_position.y;
            float s = (float)(EFFECT(flare)->camera->screen_width+EFFECT(flare)->camera->screen_height)/2.0f;
            lensflare_render_element(flare, flare->_streaks, px, py, s*0.9f, 0.0f, COLOR(255, 255, 225, 255));
            lensflare_render_element(flare, flare->_streaks, px, py, s*0.4f, 45.0f, COLOR(255, 255, 225, 255));
            px += vx*len*0.2;
            py += vy*len*0.2;
            lensflare_render_element(flare, flare->_halo, px, py, s*0.2f, 0.0f, COLOR(128, 100, 64, 64));
            px += vx*len*0.1;
            py += vy*len*0.1;
            lensflare_render_element(flare, flare->_halo, px, py, s*0.3f, 0.0f, COLOR(128, 128, 64, 32));
            px += vx*len*0.1;
            py += vy*len*0.1;
            lensflare_render_element(flare, flare->_glow, px, py, s*0.4f, 0.0f, COLOR(64, 64, 128, 128));
            px += vx*len*0.3;
            py += vy*len*0.3;
            lensflare_render_element(flare, flare->_halo, px, py, s*0.5f, 0.0f, COLOR(64, 100, 128, 64));
            px += vx*len*0.1;
            py += vy*len*0.1;
            lensflare_render_element(flare, flare->_glow, px, py, s*0.4f, 0.0f, COLOR(64, 128, 96, 128));
            px += vx*len*0.4;
            py += vy*len*0.4;
            lensflare_render_element(flare, flare->_halo, px, py, s*0.3f, 0.0f, COLOR(100, 128, 64, 64));
            px += vx*len*0.1;
            py += vy*len*0.1;
            lensflare_render_element(flare, flare->_glow, px, py, s*0.4f, 0.0f, COLOR(255, 255, 255, 64));
        }
        break; }
    }
}

static void lensflare_render_element(LensFlare *flare, Texture *texture, float x, float y, float scale, float rotation, Color color) {
    texture_activate(texture);
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    glRotatef(rotation, 0.0f, 0.0f, 1.0f);
    glColor4f(COLORFL(color));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glPopMatrix();
    texture_deactivate(texture);
}
