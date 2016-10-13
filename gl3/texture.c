/* texture.c - Textures support
 * Texture management
 * Copyright 2012 Keath Milligan
 */

#if defined(__MINGW32__)
#include <malloc.h>
#endif
#include "gl.h"
#include <stdio.h>
#include <soil/SOIL.h>
#include "texture.h"
#include <log/log.h>

Texture *texture_create(const char *resources, const char *name, int generate_mipmap, int flip_y) {
    Texture *texture = calloc(1, sizeof(Texture));
    if (!texture_init(texture, resources, name, generate_mipmap, flip_y)) {
        free(texture);
        return NULL;
    }
    return texture;
}

int texture_init(Texture *texture, const char *resources, const char *name, int generate_mipmap, int flip_y) {
    LOG("loading texture: %s from %s\n", name, resources);
    int flags = 0;
    texture->name = malloc(strlen(resources)+strlen(name)+1);
    sprintf(texture->name, "%s%s", resources, name);
    if (generate_mipmap) {
        flags |= SOIL_FLAG_MIPMAPS;
        texture->has_MIP_map = TRUE;
    }
    if (flip_y)
        flags |= SOIL_FLAG_INVERT_Y;
    texture->id = SOIL_load_OGL_texture(texture->name, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, flags);
    if (texture->id == 0) {
        LOGERR("failed to load texture %s\n", texture->name);
        free(texture->name);
        return FALSE;
    }
    LOG("texture id %d loaded\n", texture->id);
    return TRUE;
}

void texture_destroy(Texture *texture) {
    glDeleteTextures(1, (GLuint*)&texture->id);
    free(texture->name);
    free(texture);
}

void texture_activate(Texture *texture) {
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glEnable(GL_TEXTURE_2D);
    int minFilter = texture->has_MIP_map? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->repeat_U? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture->repeat_V? GL_REPEAT : GL_CLAMP_TO_EDGE);
    if (texture->offset_U != 0 || texture->offset_V != 0) {
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslatef(texture->offset_U, texture->offset_V, 0);
        glMatrixMode(GL_MODELVIEW);
    }
}

void texture_deactivate(Texture *texture) {
    glBindTexture(GL_TEXTURE_2D, 0);
}

