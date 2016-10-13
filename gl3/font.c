/* font.c -
 * Copyright 2012 Keath Milligan
 */

#include <stdio.h>
#include <errno.h>
#if __MINGW32__
#include <malloc.h>
#endif

#include <log/log.h>

#include "font.h"

typedef struct _common_block {
    unsigned short line_height;
    unsigned short base;
    unsigned short scale_w;
    unsigned short scale_h;
    unsigned short pages;
    unsigned char field;
    unsigned char alpha;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} __attribute((packed)) common_block;

typedef struct _char_descriptor {
    unsigned int id;
    unsigned short x;
    unsigned short y;
    unsigned short width;
    unsigned short height;
    short xoffset;
    short yoffset;
    short xadvance;
    unsigned char page;
    unsigned char chnl;
} __attribute((packed)) char_descriptor;

Font *font_create(const char *resources, const char *name) {
    char *fname = alloca(strlen(resources)+strlen(name)+5);
    sprintf(fname, "%s%s.fnt", resources, name);
    FILE *f = fopen(fname, "rb");
    if (f == NULL) {
        LOGERR("could not open: %s errno = %d\n", f, errno);
        return NULL;
    }
    unsigned char fileheader[5];
    fileheader[4] = '\0';
    fread(fileheader, 4, 1, f);
    if (fileheader[0] != 'B' && fileheader[1] != 'M' && fileheader[2] != 'F') {
        LOGERR("invalid font file header\n");
        return NULL;
    }
    Font *font = calloc(1, sizeof(Font));
    font->name = strdup(name);
    int blocktype;
    while ((blocktype = fgetc(f)) != EOF) {
        unsigned int blocksize;
        fread(&blocksize, sizeof(unsigned int), 1, f);
        switch(blocktype) {
        case 1: // info block
        case 3: // page block
        case 5: // kerning pairs
            LOG("skipping block type: %d size: %d\n", blocktype, blocksize);
            fseek(f, blocksize, SEEK_CUR);
            break;
        case 2: { // common block
            LOG("reading common block: size: %d (%d)\n", blocksize, sizeof(common_block));
            common_block cb;
            fread(&cb, sizeof(common_block), 1, f);
            font->width = cb.scale_w;
            font->height = cb.scale_h;
            break; }
        case 4: { // chars
            int numchars = blocksize/sizeof(char_descriptor);
            LOG("reading %d character descriptors\n", numchars);
            int i;
            char_descriptor chd;
            for (i=0; i<numchars; i++) {
                fread(&chd, sizeof(char_descriptor), 1, f);
                font->chars[chd.id].x = chd.x;
                font->chars[chd.id].y = chd.y;
                font->chars[chd.id].width = chd.width;
                font->chars[chd.id].height = chd.height;
                font->chars[chd.id].xoffset = chd.xoffset;
                font->chars[chd.id].yoffset = chd.yoffset;
                font->chars[chd.id].xadvance = chd.xadvance;
            }
            break; }
        default:
            LOGERR("invalid block type: %d\n", blocktype);
            font_destroy(font);
            return NULL;
        }
    }
    char *tname = alloca(strlen(name)+7);
    sprintf(tname, "%s_0.png", name);
    font->texture = texture_create(resources, tname, FALSE, FALSE);
    if (font->texture == NULL) {
        font_destroy(font);
        return NULL;
    }
    return font;
}

void font_destroy(Font *font) {
    if (font->texture) texture_destroy(font->texture);
    if (font->name) free(font->name);
    free(font);
}

