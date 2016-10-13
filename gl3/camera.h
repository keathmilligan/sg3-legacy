/* camera.h -
 * Copyright 2012 Keath Milligan
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "types.h"

// Camera - camera position & projection
typedef struct _Camera {
    float short_side_length;
    float near_clip;
    float far_clip;
    Number3D position;
    Number3D target;
    Number3D up;
    int dirty;
    int screen_width;
    int screen_height;
    float viewport_left;
    float viewport_right;
    float viewport_bottom;
    float viewport_top;
    float projection_matrix[16];
    float model_view_matrix[16];
    float frustum[6][4];
    int viewport[4];
} Camera;


Camera *camera_create();
void camera_destroy(Camera *camera);
void camera_reshape_viewport(Camera *camera, int width, int height);
void camera_update_view_frustum(Camera *camera);
void camera_extract_frustum(Camera *camera);
int camera_point_in_frustum(Camera *camera, Number3D point);
int camera_sphere_in_frustrum(Camera *camera, Number3D center, float radius);
Number3D camera_screen_coords(Camera *camera, Number3D point);
void camera_set_ortho(Camera *camera);
void camera_clear_ortho(Camera *camera);

#endif /* CAMERA_H_ */
