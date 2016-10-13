/* camera.c -
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

#include "math.h"
#include "util.h"
#include "camera.h"
#include "scene.h"
#include "objects.h"

Camera *camera_create() {
    Camera *camera = calloc(1, sizeof(Camera));
    camera->short_side_length = 1.0f;
    camera->near_clip = 1.0f;
    camera->far_clip = 1000.0f;
    SET3D(camera->position, 8.0f, -35.0f, 10.0f);
    SET3D(camera->target, 0.0f, 0.0f, 0.0f);
    SET3D(camera->up, 0.0f, 0.0f, 1.0f);
    camera->dirty = TRUE;
    idmx(camera->projection_matrix);
    idmx(camera->model_view_matrix);
    return camera;
}

void camera_destroy(Camera *camera) {
    free(camera);
}

void camera_reshape_viewport(Camera *camera, int width, int height) {
    camera->screen_width = width;
    camera->screen_height = height;
    float aspect = (float)width/(float)height;
    float n = camera->short_side_length/2.0f;
    camera->viewport_left = -n*aspect;
    camera->viewport_right = n*aspect;
    camera->viewport_bottom = -n;
    camera->viewport_top = n;
    if (aspect > 1) {
        camera->viewport_left *= 1.0f/aspect;
        camera->viewport_right *= 1.0f/aspect;
        camera->viewport_bottom *= 1.0f/aspect;
        camera->viewport_top *= 1.0f/aspect;
    }
    camera->dirty = TRUE;
}

void camera_update_view_frustum(Camera *camera) {
    if (camera->dirty) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(camera->viewport_left,
                  camera->viewport_right,
                  camera->viewport_bottom,
                  camera->viewport_top,
                  camera->near_clip,
                  camera->far_clip);
        glGetFloatv(GL_PROJECTION_MATRIX, camera->projection_matrix);
        glGetIntegerv(GL_VIEWPORT, camera->viewport);
        camera->dirty = FALSE;
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera->position.x, camera->position.y, camera->position.z,
              camera->target.x, camera->target.y, camera->target.z,
              camera->up.x, camera->up.y, camera->up.z);
    glGetFloatv(GL_MODELVIEW_MATRIX, camera->model_view_matrix);
    camera_extract_frustum(camera);
}

Number3D camera_screen_coords(Camera *camera, Number3D point) {
    Number3D result;
    gluProjectf(point.x, point.y, point.z,
                camera->model_view_matrix,
                camera->projection_matrix,
                camera->viewport,
                &result.x, &result.y, &result.z);
    result.y = camera->screen_height-result.y;
    return result;
}

void camera_extract_frustum(Camera *camera) {
    float clip[16];
    float t;
    clip[ 0] = camera->model_view_matrix[ 0] * camera->projection_matrix[ 0] + camera->model_view_matrix[ 1] * camera->projection_matrix[ 4] + camera->model_view_matrix[ 2] * camera->projection_matrix[ 8] +    camera->model_view_matrix[ 3] * camera->projection_matrix[12];
    clip[ 1] = camera->model_view_matrix[ 0] * camera->projection_matrix[ 1] + camera->model_view_matrix[ 1] * camera->projection_matrix[ 5] + camera->model_view_matrix[ 2] * camera->projection_matrix[ 9] +    camera->model_view_matrix[ 3] * camera->projection_matrix[13];
    clip[ 2] = camera->model_view_matrix[ 0] * camera->projection_matrix[ 2] + camera->model_view_matrix[ 1] * camera->projection_matrix[ 6] + camera->model_view_matrix[ 2] * camera->projection_matrix[10] +    camera->model_view_matrix[ 3] * camera->projection_matrix[14];
    clip[ 3] = camera->model_view_matrix[ 0] * camera->projection_matrix[ 3] + camera->model_view_matrix[ 1] * camera->projection_matrix[ 7] + camera->model_view_matrix[ 2] * camera->projection_matrix[11] +    camera->model_view_matrix[ 3] * camera->projection_matrix[15];
    clip[ 4] = camera->model_view_matrix[ 4] * camera->projection_matrix[ 0] + camera->model_view_matrix[ 5] * camera->projection_matrix[ 4]    + camera->model_view_matrix[ 6] * camera->projection_matrix[ 8] + camera->model_view_matrix[ 7] * camera->projection_matrix[12];
    clip[ 5] = camera->model_view_matrix[ 4] * camera->projection_matrix[ 1] + camera->model_view_matrix[ 5] * camera->projection_matrix[ 5] + camera->model_view_matrix[ 6] * camera->projection_matrix[ 9] +    camera->model_view_matrix[ 7] * camera->projection_matrix[13];
    clip[ 6] = camera->model_view_matrix[ 4] * camera->projection_matrix[ 2] + camera->model_view_matrix[ 5] * camera->projection_matrix[ 6] + camera->model_view_matrix[ 6] * camera->projection_matrix[10] +    camera->model_view_matrix[ 7] * camera->projection_matrix[14];
    clip[ 7] = camera->model_view_matrix[ 4] * camera->projection_matrix[ 3] + camera->model_view_matrix[ 5] * camera->projection_matrix[ 7] + camera->model_view_matrix[ 6] * camera->projection_matrix[11] +    camera->model_view_matrix[ 7] * camera->projection_matrix[15];
    clip[ 8] = camera->model_view_matrix[ 8] * camera->projection_matrix[ 0] + camera->model_view_matrix[ 9] * camera->projection_matrix[ 4]    + camera->model_view_matrix[10] * camera->projection_matrix[ 8] + camera->model_view_matrix[11] * camera->projection_matrix[12];
    clip[ 9] = camera->model_view_matrix[ 8] * camera->projection_matrix[ 1] + camera->model_view_matrix[ 9] * camera->projection_matrix[ 5] + camera->model_view_matrix[10] * camera->projection_matrix[ 9] +    camera->model_view_matrix[11] * camera->projection_matrix[13];
    clip[10] = camera->model_view_matrix[ 8] * camera->projection_matrix[ 2] + camera->model_view_matrix[ 9] * camera->projection_matrix[ 6] + camera->model_view_matrix[10] * camera->projection_matrix[10] +    camera->model_view_matrix[11] * camera->projection_matrix[14];
    clip[11] = camera->model_view_matrix[ 8] * camera->projection_matrix[ 3] + camera->model_view_matrix[ 9] * camera->projection_matrix[ 7] + camera->model_view_matrix[10] * camera->projection_matrix[11] +    camera->model_view_matrix[11] * camera->projection_matrix[15];
    clip[12] = camera->model_view_matrix[12] * camera->projection_matrix[ 0] + camera->model_view_matrix[13] * camera->projection_matrix[ 4]    + camera->model_view_matrix[14] * camera->projection_matrix[ 8] + camera->model_view_matrix[15] * camera->projection_matrix[12];
    clip[13] = camera->model_view_matrix[12] * camera->projection_matrix[ 1] + camera->model_view_matrix[13] * camera->projection_matrix[ 5] + camera->model_view_matrix[14] * camera->projection_matrix[ 9] +    camera->model_view_matrix[15] * camera->projection_matrix[13];
    clip[14] = camera->model_view_matrix[12] * camera->projection_matrix[ 2] + camera->model_view_matrix[13] * camera->projection_matrix[ 6] + camera->model_view_matrix[14] * camera->projection_matrix[10] +    camera->model_view_matrix[15] * camera->projection_matrix[14];
    clip[15] = camera->model_view_matrix[12] * camera->projection_matrix[ 3] + camera->model_view_matrix[13] * camera->projection_matrix[ 7] + camera->model_view_matrix[14] * camera->projection_matrix[11] +    camera->model_view_matrix[15] * camera->projection_matrix[15];
    camera->frustum[0][0] = clip[ 3] - clip[ 0];
    camera->frustum[0][1] = clip[ 7] - clip[ 4];
    camera->frustum[0][2] = clip[11] - clip[ 8];
    camera->frustum[0][3] = clip[15] - clip[12];
    t = sqrtf( camera->frustum[0][0] * camera->frustum[0][0] + camera->frustum[0][1] * camera->frustum[0][1] + camera->frustum[0][2]    * camera->frustum[0][2] );
    camera->frustum[0][0] /= t;
    camera->frustum[0][1] /= t;
    camera->frustum[0][2] /= t;
    camera->frustum[0][3] /= t;
    camera->frustum[1][0] = clip[ 3] + clip[ 0];
    camera->frustum[1][1] = clip[ 7] + clip[ 4];
    camera->frustum[1][2] = clip[11] + clip[ 8];
    camera->frustum[1][3] = clip[15] + clip[12];
    t = sqrtf( camera->frustum[1][0] * camera->frustum[1][0] + camera->frustum[1][1] * camera->frustum[1][1] + camera->frustum[1][2]    * camera->frustum[1][2] );
    camera->frustum[1][0] /= t;
    camera->frustum[1][1] /= t;
    camera->frustum[1][2] /= t;
    camera->frustum[1][3] /= t;
    camera->frustum[2][0] = clip[ 3] + clip[ 1];
    camera->frustum[2][1] = clip[ 7] + clip[ 5];
    camera->frustum[2][2] = clip[11] + clip[ 9];
    camera->frustum[2][3] = clip[15] + clip[13];
    t = sqrtf( camera->frustum[2][0] * camera->frustum[2][0] + camera->frustum[2][1] * camera->frustum[2][1] + camera->frustum[2][2]    * camera->frustum[2][2] );
    camera->frustum[2][0] /= t;
    camera->frustum[2][1] /= t;
    camera->frustum[2][2] /= t;
    camera->frustum[2][3] /= t;
    camera->frustum[3][0] = clip[ 3] - clip[ 1];
    camera->frustum[3][1] = clip[ 7] - clip[ 5];
    camera->frustum[3][2] = clip[11] - clip[ 9];
    camera->frustum[3][3] = clip[15] - clip[13];
    t = sqrtf( camera->frustum[3][0] * camera->frustum[3][0] + camera->frustum[3][1] * camera->frustum[3][1] + camera->frustum[3][2]    * camera->frustum[3][2] );
    camera->frustum[3][0] /= t;
    camera->frustum[3][1] /= t;
    camera->frustum[3][2] /= t;
    camera->frustum[3][3] /= t;
    camera->frustum[4][0] = clip[ 3] - clip[ 2];
    camera->frustum[4][1] = clip[ 7] - clip[ 6];
    camera->frustum[4][2] = clip[11] - clip[10];
    camera->frustum[4][3] = clip[15] - clip[14];
    t = sqrtf( camera->frustum[4][0] * camera->frustum[4][0] + camera->frustum[4][1] * camera->frustum[4][1] + camera->frustum[4][2]    * camera->frustum[4][2] );
    camera->frustum[4][0] /= t;
    camera->frustum[4][1] /= t;
    camera->frustum[4][2] /= t;
    camera->frustum[4][3] /= t;
    camera->frustum[5][0] = clip[ 3] + clip[ 2];
    camera->frustum[5][1] = clip[ 7] + clip[ 6];
    camera->frustum[5][2] = clip[11] + clip[10];
    camera->frustum[5][3] = clip[15] + clip[14];
    t = sqrtf( camera->frustum[5][0] * camera->frustum[5][0] + camera->frustum[5][1] * camera->frustum[5][1] + camera->frustum[5][2] * camera->frustum[5][2] );
    camera->frustum[5][0] /= t;
    camera->frustum[5][1] /= t;
    camera->frustum[5][2] /= t;
    camera->frustum[5][3] /= t;
}

int camera_point_in_frustum(Camera *camera, Number3D point) {
    int p;
    for (p=0; p<6; p++)
      if (camera->frustum[p][0]*point.x+camera->frustum[p][1]*point.y+camera->frustum[p][2]*point.z+camera->frustum[p][3] <= 0)
        return FALSE;
    return TRUE;
}

int camera_sphere_in_frustrum(Camera *camera, Number3D center, float radius) {
    int p;
    for (p=0; p<6; p++)
      if (camera->frustum[p][0]*center.x+camera->frustum[p][1]*center.y+camera->frustum[p][2]*center.z+camera->frustum[p][3] <= -radius)
        return FALSE;
    return TRUE;
}

void camera_set_ortho(Camera *camera) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, camera->screen_width, camera->screen_height, 0.0f, -1.0f, 1.0f);
    //gluOrtho2D(0.0f, camera->viewport_right, 0.0f, camera->viewport_bottom);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void camera_clear_ortho(Camera *camera) {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
