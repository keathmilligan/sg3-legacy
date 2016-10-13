/* overlay.c - 2D display overlay support
 * Provides support for display a "HUD" or other overlay on top of 3D graphics
 * Copyright 2012 Keath Milligan
 */

#ifndef OVERLAY_H_
#define OVERLAY_H_

#include "types.h"
#include "texture.h"
#include "font.h"

#define OVERLAYOBJ(x) ((OverlayObject*)x)

struct _OverlayObject;
typedef void (*OverlayObjectFPtr)(struct _OverlayObject *);

typedef struct _OverlayObject {
    Number2D position;
    float rotation;
    Number2D scale;
    Color color;
    Number2D *vertices;
    int vertex_count;
    int filled;
    OverlayObjectFPtr render;
    OverlayObjectFPtr destroy;
} OverlayObject;

typedef struct _OverlayObjectList {
    OverlayObject *object;
    struct _OverlayObjectList *next;
    struct _OverlayObjectList *prev;
} OverlayObjectList;

typedef OverlayObject OverlayLine;

typedef OverlayObject OverlayRectangle;

typedef OverlayObject OverlayTriangle;

typedef OverlayObject OverlayCircle;

typedef struct _OverlayImage {
    OverlayObject base;
    Texture *bitmap;
    UV *uvs;
    Face *faces;
} OverlayImage;

typedef struct _OverlayText {
    OverlayObject base;
    UV *uvs;
    int face_count;
    Face *faces;
    Font *font;
    char *text;
    int text_len;
    int max_len;
} OverlayText;

typedef struct _Overlay {
    int visible;
    int viewport_width;
    int viewport_height;
    OverlayObjectList *objects;
} Overlay;

Overlay *overlay_create();
void overlay_destroy(Overlay *overlay);
void overlay_render(Overlay *overlay);
void overlay_reshape_viewport(Overlay *overlay, int width, int height);
void overlay_add_object(Overlay *overlay, OverlayObject *object);
void overlay_remove_object(Overlay *overlay, OverlayObject *object);
OverlayLine *overlayline_create(int x1, int y1, int x2, int y2);
OverlayTriangle *overlaytriangle_create(int width, int height, int filled);
OverlayRectangle *overlayrectangle_create(int width, int height, int filled);
OverlayCircle *overlaycircle_create(int radius, int filled);
OverlayImage *overlayimage_create(int width, int height, const char *resources, const char *image_name);
OverlayText *overlaytext_create(Font *font, const char *text, int max_len);
void overlaytext_set_text(OverlayText *object, const char *text);

#endif /* OVERLAY_H_ */
