/* ctrl_demo.c - SG3 demo
 * Copyright 2012 Keath Milligan
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#if defined(__MINGW32__)
#include <windows.h>
#include <malloc.h>
#include <gl/gl.h>
#include <gl/glu.h>
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <gl/gl.h>
#include <gl/glu.h>
#endif

#include <gl3/gl3.h>
#include "ctrl_demo.h"

#if defined(__MINGW32__)
#define RESOURCE_DIR "resources\\"
#elif defined(__APPLE__)
#define RESOURCE_DIR "/Users/kmilligan/Work/GLTest2/GLTest2/resources/"
#else
#define RESOURCE_DIR "resources/"
#endif

// physics/control thread handling
static pthread_t update_thread;
static int update_thread_running = FALSE;
static int exit_flag = FALSE;
static pthread_mutex_t scene_mutex;

// controls
#define CONTROL_RATE 0.0001
#define THROTTLE_RATE 0.0005
static int input_x, input_y, input_z = 0;
static double time_x, time_y, time_z;
static double pitch, yaw, roll = 0.0;
static int throttle_input = 0;
static double throttle_time;
static double throttle = 0.0;

// graphics
static Scene *scene = NULL;
static Overlay *overlay = NULL;
static OverlayCircle *dot;
static OverlayLine *line;
static OverlayRectangle *thrust_indicator;

static void *update_thread_proc();
static double get_time();
static void control_update(double st, double et);
static void view_update();

int demo_init() {
    scene = scene_create();
    overlay = overlay_create();

    OverlayCircle *oc = overlaycircle_create(300, FALSE);
    oc->color = COLOR(128, 128, 255, 255);
    overlay_add_object(overlay, oc);
    OverlayRectangle *or = overlayrectangle_create(600, 600, FALSE);
    or->color = COLOR(255, 0, 0, 255);
    overlay_add_object(overlay, or);

    dot = overlaycircle_create(20, TRUE);
    dot->color = COLOR(0, 255, 255, 255);
    overlay_add_object(overlay, dot);

    line = overlayline_create(-300, 0, 300, 0);
    line->color = COLOR(255, 255, 0, 255);
    overlay_add_object(overlay, line);

    OverlayRectangle *or2 = overlayrectangle_create(20, 600, FALSE);
    or2->color = COLOR(128, 128, 128, 255);
    or2->position.x = 330.0f;
    overlay_add_object(overlay, or2);

    thrust_indicator = overlayrectangle_create(16, 596, TRUE);
    thrust_indicator->color = COLOR(0, 128, 0, 255);
    thrust_indicator->position.x = 330.0f;
    thrust_indicator->scale.y = 0.0f;
    overlay_add_object(overlay, thrust_indicator);

    // create the scene mutex
    int rc = pthread_mutex_init(&scene_mutex, NULL);
    if (rc) {
        LOGERR("failed to create scene mutex\n");
        demo_cleanup();
        return FALSE;
    }
    
    // create & start update thread
    rc = pthread_create(&update_thread, NULL, update_thread_proc, NULL);
    if (rc) {
        LOGERR("failed to create update thread\n");
        demo_cleanup();
        return FALSE;
    } else {
        update_thread_running = TRUE;
    }

    return TRUE;
}

void demo_cleanup() {
    LOG("Cleaning up demo\n");
    exit_flag = TRUE;
    pthread_join(update_thread, NULL);
    if (scene) scene_destroy(scene);
    if (overlay) overlay_destroy(overlay);
    pthread_mutex_destroy(&scene_mutex);
}

void demo_reshape_viewport(int width, int height) {
    LOG("Reshape viewport %d, %d\n", width, height);
    pthread_mutex_lock(&scene_mutex);
    if (scene) scene_reshape_viewport(scene, width, height);
    if (overlay) overlay_reshape_viewport(overlay, width, height);
    pthread_mutex_unlock(&scene_mutex);
}

static double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec*1000.0+(double)tv.tv_usec/1000.0;
}

static void *update_thread_proc(void *arg) {
    double last = 0.0;
    while(!exit_flag) {
        double st = get_time();
        double et = st-last;
        last = st;
        if (et > 1000.0)
            et = 0.0;
        control_update(st, et);
        pthread_mutex_lock(&scene_mutex);
        view_update();
        scene_update(scene);
        pthread_mutex_unlock(&scene_mutex);
        usleep(1000);
    }
    return NULL;
}

void demo_render() {
    pthread_mutex_lock(&scene_mutex);
    if (scene) scene_render(scene);
    if (overlay) overlay_render(overlay);
    pthread_mutex_unlock(&scene_mutex);
}

static void view_update() {
    dot->position.y = 300.0f*pitch;
    dot->position.x = 300.0f*yaw;
    line->rotation = 90.0f*roll;
    thrust_indicator->scale.y = (float)throttle;
    thrust_indicator->position.y = 300.0f-300.0f*throttle;
}

static void adj_control(int *input, double input_time, double *control, double rate, double et) {
    double velocity = rate*(10.0*(input_time/1000.0));
    if (*input != 0) {
        *control += (et*velocity)*(*input);
        if (*control > 1.0) *control = 1.0;
        else if (*control < -1.0) *control = -1.0;
    } else {
        if (*control < 0.0) {
            *control += et*velocity;
            if (*control > 0.0) *control = 0.0;
        } else if (*control > 0.0) {
            *control -= et*velocity;
            if (*control < 0.0) *control = 0.0;
        }
    }
}

static void control_update(double st, double et) {
    adj_control(&input_x, st-time_x, &pitch, CONTROL_RATE, et);
    adj_control(&input_z, st-time_z, &yaw, CONTROL_RATE, et);
    adj_control(&input_y, st-time_y, &roll, CONTROL_RATE, et);
    if (throttle_input != 0) {
        throttle += et*THROTTLE_RATE*throttle_input;
        if (throttle > 1.0) throttle = 1.0;
        else if (throttle < 0.0) throttle = 0.0;
    }
}

static void adj_input(int *control, int state, double *time, int val) {
    int new_ctrl = state? val : 0;
    if (*control != new_ctrl) {
        pthread_mutex_lock(&scene_mutex);
        *control = new_ctrl;
        *time = get_time();
        pthread_mutex_unlock(&scene_mutex);
    }
}

void demo_up(int state) {
    adj_input(&input_x, state, &time_x, 1);
}

void demo_down(int state) {
    adj_input(&input_x, state, &time_x, -1);
}

void demo_right(int state) {
    adj_input(&input_z, state, &time_z, 1);
}

void demo_left(int state) {
    adj_input(&input_z, state, &time_z, -1);
}

void demo_roll_right(int state) {
    adj_input(&input_y, state, &time_y, 1);
}

void demo_roll_left(int state) {
    adj_input(&input_y, state, &time_y, -1);
}

void demo_throttle_up(int state) {
    adj_input(&throttle_input, state, &throttle_time, 1);
}

void demo_throttle_down(int state) {
    adj_input(&throttle_input, state, &throttle_time, -1);
}
