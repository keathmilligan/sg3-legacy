/* effects.h - special effects support
 * Lens flares, explosions and other visual effects
 * Copyright 2012 Keath Milligan
 */

#ifndef EFFECTS_H_
#define EFFECTS_H_

#include "math.h"
#include "types.h"
#include "objects.h"
#include "texture.h"
#include "camera.h"

#define EFFECT(c) ((Effect*)c)

typedef enum {
    EF_BACKGROUND,
    EF_SCENE,
    EF_OVERLAY
} EffectRenderLevel;

struct _Effect;
struct _Scene;
typedef void (*EffectUpdateFuncPtr)(struct _Scene *scene, struct _Effect *);
typedef void (*EffectRenderFuncPtr)(struct _Effect *, EffectRenderLevel level);
typedef void (*EffectDestroyFuncPtr)(struct _Effect *);

// Effect Base Class
typedef struct _Effect {
    Camera *camera;
    EffectUpdateFuncPtr update;
    EffectRenderFuncPtr render;
    EffectDestroyFuncPtr destroy;
} Effect;

typedef struct _EffectList {
    Effect *effect;
    struct _EffectList *next;
    struct _EffectList *prev;
} EffectList;

// Lens Flare Effect
typedef struct _LensFlare {
    Effect _base;
    Number3D light_position;
    float light_radius;
    int _visible;
    int _occluded;
    Number3D _screen_position;
    Texture *_streaks;
    Texture *_halo;
    Texture *_glow;
} LensFlare;

LensFlare *lensflare_create(const char *resources, Camera *camera, Number3D light_position, float light_radius);
void lensflare_destroy(LensFlare *flare);

#endif /* EFFECTS_H_ */
