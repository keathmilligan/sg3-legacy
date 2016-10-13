/* overlay.c - 2D display overlay support
 * Provides support for display a "HUD" or other overlay on top of 3D graphics
 * Copyright 2012 Keath Milligan
 */

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
#include "overlay.h"
#include "math.h"
#include <log/log.h>

static void overlayobject_destroy(OverlayObject *object);
static void overlayline_render(OverlayLine *object);
static void overlayrectangle_render(OverlayRectangle *object);
static void overlaytriangle_render(OverlayTriangle *object);
static void overlaycircle_render(OverlayCircle *object);
static void overlayimage_render(OverlayImage *object);
static void overlayimage_destroy(OverlayImage *object);
static void overlaytext_render(OverlayText *object);
static void overlaytext_destroy(OverlayText *object);

Overlay *overlay_create() {
    LOG("creating overlay\n");
    Overlay *overlay = calloc(1, sizeof(Overlay));
    overlay->visible = TRUE;
    return overlay;
}

void overlay_destroy(Overlay *overlay) {
    LOG("destroying overlay %x\n", overlay);
    OverlayObjectList *eo, *to;
    LL_FOREACH_SAFE(overlay->objects, eo, to) {
        LL_DELETE(overlay->objects, eo);
        eo->object->destroy(eo->object);
        free(eo);
    }
    free(overlay);
}

void overlay_render(Overlay *overlay) {
    OverlayObjectList *o;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-overlay->viewport_width/2, overlay->viewport_width/2, overlay->viewport_height/2, -overlay->viewport_height/2, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_COLOR_MATERIAL);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    LL_FOREACH(overlay->objects, o) {
        o->object->render(o->object);
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void overlay_reshape_viewport(Overlay *overlay, int width, int height) {
    overlay->viewport_width = width;
    overlay->viewport_height = height;
}

void overlay_add_object(Overlay *overlay, OverlayObject *object) {
    LOG("adding object %x\n", object);
    OverlayObjectList *e = malloc(sizeof(OverlayObjectList));
    e->object = object;
    LL_APPEND(overlay->objects, e);
}

void overlay_remove_object(Overlay *overlay, OverlayObject *object) {
    OverlayObjectList *e, *t;
    LL_FOREACH_SAFE(overlay->objects, e, t) {
        if (e->object == object) {
            LL_DELETE(overlay->objects, e);
            free(e);
            return;
        }
    }
    LOGERR("object not found for remove");
}

void overlay_clear_objects(Overlay *overlay) {
    OverlayObjectList *eo, *to;
    LL_FOREACH_SAFE(overlay->objects, eo, to) {
        LL_DELETE(overlay->objects, eo);
        eo->object->destroy(eo->object);
        free(eo);
    }
}

OverlayLine *overlayline_create(int x1, int y1, int x2, int y2) {
    OverlayLine *object = calloc(1, sizeof(OverlayLine));
    SETCOLOR(object->color, 255, 255, 255, 255);
    object->scale = NUM2D(1.0f, 1.0f);
    object->vertex_count = 2;
    object->vertices = calloc(2, sizeof(Number2D));
    object->vertices[0] = NUM2D((float)x1, (float)y1);
    object->vertices[1] = NUM2D((float)x2, (float)y2);
    object->render = overlayline_render;
    object->destroy = overlayobject_destroy;
    return object;
}

static void overlayline_render(OverlayLine *object) {
    glPushMatrix();
    glRotatef(object->rotation, 0.0f, 0.0f, 1.0f);
    glTranslatef(object->position.x, object->position.y, 0.0f);
    glScalef(object->scale.x, object->scale.y, 1.0f);
    glColor4f(COLORFL(object->color));
    glLineWidth(1.0f);
    glVertexPointer(2, GL_FLOAT, 0, object->vertices);
    glDrawArrays(GL_LINES, 0, 2);
    glPopMatrix();
}

OverlayTriangle *overlaytriangle_create(int width, int height, int filled) {
    OverlayTriangle *object = calloc(1, sizeof(OverlayTriangle));
    SETCOLOR(object->color, 255, 255, 255, 255);
    object->scale = NUM2D(1.0f, 1.0f);
    object->vertex_count = 3;
    object->vertices = calloc(3, sizeof(Number2D));
    object->vertices[0] = NUM2D(0.0f, (float)-(height/2));
    object->vertices[1] = NUM2D((float)-(width/2), (float)height/2);
    object->vertices[2] = NUM2D((float)width/2, (float)height/2);
    object->filled = filled;
    object->render = overlaytriangle_render;
    object->destroy = overlayobject_destroy;
    return object;
}

static void overlaytriangle_render(OverlayTriangle *object) {
    glPushMatrix();
    glRotatef(object->rotation, 0.0f, 0.0f, 1.0f);
    glTranslatef(object->position.x, object->position.y, 0.0f);
    glScalef(object->scale.x, object->scale.y, 1.0f);
    glColor4f(COLORFL(object->color));
    glLineWidth(1.0f);
    glVertexPointer(2, GL_FLOAT, 0, object->vertices);
    glDrawArrays(object->filled?GL_TRIANGLE_FAN:GL_LINE_LOOP, 0, 3);
    glPopMatrix();
}

OverlayRectangle *overlayrectangle_create(int width, int height, int filled) {
    OverlayRectangle *object = calloc(1, sizeof(OverlayRectangle));
    SETCOLOR(object->color, 255, 255, 255, 255);
    object->scale = NUM2D(1.0f, 1.0f);
    object->vertex_count = 4;
    object->vertices = calloc(4, sizeof(Number2D));
    if (!filled) {
        object->vertices[0] = NUM2D((float)-(width/2), (float)-(height/2));
        object->vertices[1] = NUM2D((float)-(width/2), (float)(height/2));
        object->vertices[2] = NUM2D((float)(width/2), (float)(height/2));
        object->vertices[3] = NUM2D((float)(width/2), (float)-(height/2));
    } else {
        object->vertices[0] = NUM2D((float)-(width/2), (float)-(height/2));
        object->vertices[1] = NUM2D((float)-(width/2), (float)(height/2));
        object->vertices[2] = NUM2D((float)(width/2), (float)-(height/2));
        object->vertices[3] = NUM2D((float)(width/2), (float)(height/2));
    }
    object->filled = filled;
    object->render = overlayrectangle_render;
    object->destroy = overlayobject_destroy;
    return object;
}

static void overlayrectangle_render(OverlayRectangle *object) {
    glPushMatrix();
    glRotatef(object->rotation, 0.0f, 0.0f, 1.0f);
    glTranslatef(object->position.x, object->position.y, 0.0f);
    glScalef(object->scale.x, object->scale.y, 1.0f);
    glColor4f(COLORFL(object->color));
    glLineWidth(1.0f);
    glVertexPointer(2, GL_FLOAT, 0, object->vertices);
    glDrawArrays(object->filled?GL_TRIANGLE_STRIP:GL_LINE_LOOP, 0, 4);
    glPopMatrix();
}

OverlayCircle *overlaycircle_create(int radius, int filled) {
    OverlayCircle *object = calloc(1, sizeof(OverlayCircle));
    SETCOLOR(object->color, 255, 255, 255, 255);
    object->scale = NUM2D(1.0f, 1.0f);
    object->vertex_count = 360;
    object->vertices = calloc(360, sizeof(Number2D));
    int i;
    for (i=0; i<360; i++) {
        object->vertices[359-i] = NUM2D(0.0f, (float)radius);
        rot2d(&object->vertices[359-i], DEG2RAD(i));
    }
    object->filled = filled;
    object->render = overlaycircle_render;
    object->destroy = overlayobject_destroy;
    return object;
}

static void overlaycircle_render(OverlayCircle *object) {
    glPushMatrix();
    glTranslatef(object->position.x, object->position.y, 0.0f);
    glScalef(object->scale.x, object->scale.y, 1.0f);
    glColor4f(COLORFL(object->color));
    glLineWidth(1.0f);
    glVertexPointer(2, GL_FLOAT, 0, object->vertices);
    glDrawArrays(object->filled?GL_TRIANGLE_FAN:GL_LINE_LOOP, 0, object->vertex_count);
    glPopMatrix();
}

OverlayImage *overlayimage_create(int width, int height, const char *image_name) {
    OverlayImage *object = calloc(1, sizeof(OverlayImage));
    SETCOLOR(((OverlayObject*)object)->color, 255, 255, 255, 255);
    ((OverlayObject*)object)->scale = NUM2D(1.0f, 1.0f);
    ((OverlayObject*)object)->vertices = calloc(4, sizeof(Number2D));
    ((OverlayObject*)object)->vertices[0] = NUM2D((float)(width/2), (float)-(height/2));
    ((OverlayObject*)object)->vertices[1] = NUM2D((float)-(width/2), (float)-(height/2));
    ((OverlayObject*)object)->vertices[2] = NUM2D((float)-(width/2), (float)(height/2));
    ((OverlayObject*)object)->vertices[3] = NUM2D((float)(width/2), (float)(height/2));
    ((OverlayObject*)object)->render = (OverlayObjectFPtr)overlayimage_render;
    ((OverlayObject*)object)->destroy = (OverlayObjectFPtr)overlayimage_destroy;
    object->bitmap = texture_create(image_name, TRUE, FALSE);
    object->uvs = calloc(4, sizeof(UV));
    object->faces = malloc(sizeof(Face)*2);
    object->uvs[0] = (UV){1.0f, 0.0f};
    object->uvs[1] = (UV){0.0f, 0.0f};
    object->uvs[2] = (UV){0.0f, 1.0f};
    object->uvs[3] = (UV){1.0f, 1.0f};
    object->faces[0] = (Face){0, 1, 2};
    object->faces[1] = (Face){0, 2, 3};
    return object;
}

static void overlayimage_render(OverlayImage *object) {
    glPushMatrix();
    glRotatef(((OverlayObject*)object)->rotation, 0.0f, 0.0f, 1.0f);
    glTranslatef(((OverlayObject*)object)->position.x, ((OverlayObject*)object)->position.y, 0.0f);
    glScalef(((OverlayObject*)object)->scale.x, ((OverlayObject*)object)->scale.y, 1.0f);
    glColor4f(COLORFL(((OverlayObject*)object)->color));
    glVertexPointer(2, GL_FLOAT, 0, ((OverlayObject*)object)->vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, object->uvs);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    texture_activate(object->bitmap);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, object->faces);;
    texture_deactivate(object->bitmap);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
}

static void overlayimage_destroy(OverlayImage *object) {
    if (object->bitmap) texture_destroy(object->bitmap);
    if (object->uvs) free(object->uvs);
    if (object->faces) free(object->faces);
    overlayobject_destroy((OverlayObject*)object);
}

static void overlayobject_destroy(OverlayObject *object) {
    if (object->vertices) free(object->vertices);
    free(object);
}

OverlayText *overlaytext_create(Font *font, const char *text, int max_len) {
    OverlayText *object = calloc(1, sizeof(OverlayText));
    OVERLAYOBJ(object)->color = COLOR(255, 255, 255, 255);
    OVERLAYOBJ(object)->scale = NUM2D(1.0f, 1.0f);
    OVERLAYOBJ(object)->render = (OverlayObjectFPtr)overlaytext_render;
    OVERLAYOBJ(object)->destroy = (OverlayObjectFPtr)overlaytext_destroy;
    if (max_len < 0) max_len = (int)strlen(text);
    object->max_len = max_len;
    object->text = malloc(max_len+1);
    int vertex_count = max_len*4;
    OVERLAYOBJ(object)->vertex_count = vertex_count;
    OVERLAYOBJ(object)->vertices = malloc(sizeof(Number2D)*vertex_count);
    object->uvs = malloc(sizeof(UV)*vertex_count);
    object->faces = malloc(sizeof(Face)*2*max_len);
    object->font = font;
    overlaytext_set_text(object, text);
    return object;
}

void overlaytext_set_text(OverlayText *object, const char *text) {
    object->text_len = (int)strlen(text);
    object->face_count = 2*object->text_len;
    strncpy(object->text, text, object->max_len);
    float cursorx = 0.0f;
    int i;
    float setwidth = (float)object->font->width;
    float setheight = (float)object->font->height;
    for (i=0; i<strlen(text); i++) {
        float x = (float)object->font->chars[(int)text[i]].x;
        float y = (float)object->font->chars[(int)text[i]].y;
        float width = (float)object->font->chars[(int)text[i]].width;
        float height = (float)object->font->chars[(int)text[i]].height;
        float xoffset = (float)object->font->chars[(int)text[i]].xoffset;
        float yoffset = (float)object->font->chars[(int)text[i]].yoffset;
        object->uvs[i*4]   = (UV){(x+width)/setwidth, y/setheight};
        object->uvs[i*4+1] = (UV){x/setwidth, y/setheight};
        object->uvs[i*4+2] = (UV){x/setwidth, (y+height)/setheight};
        object->uvs[i*4+3] = (UV){(x+width)/setwidth, (y+height)/setheight};
        OVERLAYOBJ(object)->vertices[i*4]   = NUM2D(cursorx+xoffset+width, yoffset);
        OVERLAYOBJ(object)->vertices[i*4+1] = NUM2D(cursorx+xoffset, yoffset);
        OVERLAYOBJ(object)->vertices[i*4+2] = NUM2D(cursorx+xoffset, yoffset+height);
        OVERLAYOBJ(object)->vertices[i*4+3] = NUM2D(cursorx+xoffset+width, yoffset+height);
        object->faces[i*2] = (Face){i*4, i*4+1, i*4+2};
        object->faces[i*2+1] = (Face){i*4, i*4+2, i*4+3};
        cursorx += (float)object->font->chars[(int)text[i]].xadvance;
    }
}

static void overlaytext_render(OverlayText *object) {
    glPushMatrix();
    glRotatef(((OverlayObject*)object)->rotation, 0.0f, 0.0f, 1.0f);
    glTranslatef(OVERLAYOBJ(object)->position.x, OVERLAYOBJ(object)->position.y, 0.0f);
    glScalef(OVERLAYOBJ(object)->scale.x, OVERLAYOBJ(object)->scale.y, 1.0f);
    glColor4f(COLORFL(OVERLAYOBJ(object)->color));
    glVertexPointer(2, GL_FLOAT, 0, OVERLAYOBJ(object)->vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, object->uvs);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    texture_activate(object->font->texture);
    glDrawElements(GL_TRIANGLES, object->face_count*3, GL_UNSIGNED_SHORT, object->faces);
    texture_deactivate(object->font->texture);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
}

static void overlaytext_destroy(OverlayText *object) {
    if (object->uvs) free(object->uvs);
    if (object->text) free(object->text);
    if (object->faces) free(object->faces);
    overlayobject_destroy((OverlayObject*)object);
}
