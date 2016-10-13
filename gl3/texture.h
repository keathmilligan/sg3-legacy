/* texture.h - Textures support
 * Texture management
 * Copyright 2012 Keath Milligan
 */

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "types.h"

typedef struct _UV {
    float u, v;
} UV;

typedef struct _Face {
    short a, b, c;
} Face;

typedef struct _Texture {
    int id;
    char *name;
    int has_MIP_map;
    int repeat_U;
    int repeat_V;
    int offset_U;
    int offset_V;
} Texture;

typedef struct _TextureList {
    Texture *texture;
    struct _TextureList *next;
    struct _TextureList *prev;
} TextureList;

Texture *texture_create(const char *resources, const char *name, int generate_mipmap, int flip_y);
int texture_init(Texture *texture, const char *resources, const char *name, int generate_mipmap, int flip_y);
void texture_destroy(Texture *texture);
void texture_activate(Texture *texture);
void texture_deactivate(Texture *texture);

#endif /* TEXTURE_H_ */
