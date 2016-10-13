/* font.h -
 * Copyright 2012 Keath Milligan
 */

#ifndef FONT_H_
#define FONT_H_

#include "texture.h"

typedef struct _Glyph {
    unsigned short x, y;
    unsigned short width, height;
    short xoffset, yoffset;
    short xadvance;
} Glyph;

typedef struct _Font {
    char *name;
    Texture *texture;
    int width, height;
    Glyph chars[256];
} Font;

Font *font_create(const char *resources, const char *name);
void font_destroy(Font *font);

#endif /* FONT_H_ */
