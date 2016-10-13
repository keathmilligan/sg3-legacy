/* demo.c - SG3 demo
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
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <gl3/gl3.h>
#include "sg3_demo.h"

#if defined(__MINGW32__)
#define RESOURCE_DIR "resources\\"
#elif defined(__APPLE__)
#define RESOURCE_DIR "/Users/kmilligan/Work/GLTest2/GLTest2/resources/"
#else
#define RESOURCE_DIR "resources/"
#endif

Scene *scene = NULL;
Overlay *overlay = NULL;
Model *m;
Cube *c, *c2;
WireTriangle *wt;
WireLine *wl;
Billboard *sun;
float rotate = 0.0f;
#define maxz 30.0f
#define maxx 30.0f
int zdir = 1;
int xdir = 1;
float zpos = maxz;
float xpos = 0.0f;
Font *font = NULL;
OverlayText *avg_render_time, *avg_update_time;
pthread_t update_thread;
int update_thread_running = FALSE;
int exit_flag = FALSE;
pthread_mutex_t scene_mutex;
Quaternion cam_direction;
float input_x, input_y, input_z = 0.0f;
float throttle = 0.0f;

static void *update_thread_proc();
static float get_elapsed_time();

int demo_init() {
    char *sdir = alloca(256);
    getcwd(sdir, 255);
    LOG("Initializing SG3 demo: %s\n", sdir);
    scene = scene_create();
    scene->show_axis = TRUE;
    //scene->show_grid = TRUE;
    //scene->fog_enabled = TRUE;

    SET3D(scene->camera->position, 0.0f, -60.0f, 5.0f);
    SET3D(scene->camera->up, 0.0f, 0.0f, 1.0f);
    ortq(&cam_direction, NUM3D(0.0f, 0.0f, 0.0f));

    Skybox *sb = skybox_create(RESOURCE_DIR, "nebula2", 300.0f, scene->camera);
    scene_add_skybox(scene, sb);
//    sb = skybox_create("resources\\", "planets2", 280.0f, scene->camera);
//    scene_add_skybox(scene, sb);

    sun = billboard_create(40.0f, 40.0f,
                         texture_create(RESOURCE_DIR "sun.png", TRUE, FALSE),
                         scene->camera,
                         BB_SCREEN_ALIGNED);
    OBJ3D(sun)->position.y = 300.0f;
    OBJ3D(sun)->position.z = 10.0f;
    OBJ3D(sun)->lighting_enabled = FALSE;
    OBJ3D(sun)->color = COLOR(255, 255, 225, 255);
    sun->fixed_proximity = TRUE;
    scene_add_background_object(scene, OBJ3D(sun));

    scene->lights->light->position = OBJ3D(sun)->position;

    m = model_create(RESOURCE_DIR, "trainer1");
    if (m != NULL) {
        scene_add_object(scene, OBJ3D(m));
        //OBJ3D(m)->position.z = 2.0f;
        //SET3D(OBJ3D(m)->scale, 8.0f, 8.0f, 8.0f);
        ortmx(OBJ3D(m)->rotation, EULER3D(90, 0, 0));
        OBJ3D(m)->render_aabb = TRUE;
    }

    c = cube_create(3.0f, 3.0f, 3.0f, NULL);
    OBJ3D(c)->ambient = COLOR(0, 128, 0, 255);
    OBJ3D(c)->position.y = -20.0f;
    scene_add_object(scene, OBJ3D(c));

    c2 = cube_create(3.0f, 3.0f, 3.0f, NULL);
    OBJ3D(c2)->ambient = COLOR(0, 128, 128, 255);
    OBJ3D(c2)->position.y = 30.0f;
    scene_add_object(scene, OBJ3D(c2));

    Panel *p = panel_create(8.0f, 8.0f, texture_create(RESOURCE_DIR "monkey.png", TRUE, FALSE));
    OBJ3D(p)->position.x = -20.0f;
    scene_add_object(scene, OBJ3D(p));

    Billboard *b = billboard_create(8.0f, 8.0f,
                                    texture_create(RESOURCE_DIR "flowers.jpg", TRUE, FALSE),
                                    scene->camera,
                                    BB_SCREEN_ALIGNED);
    OBJ3D(b)->position.x = 20.0f;
    scene_add_object(scene, OBJ3D(b));

    b = billboard_create(8.0f, 8.0f,
                         texture_create(RESOURCE_DIR "billboard.png", TRUE, FALSE),
                         scene->camera,
                         BB_SPHERICAL);
    SET3D(OBJ3D(b)->position, 20.0f, -20.0f, 0.0f);
    scene_add_object(scene, OBJ3D(b));

    LensFlare *lf = lensflare_create(RESOURCE_DIR, scene->camera, OBJ3D(sun)->position, 40.0f);
    scene_add_effect(scene, EFFECT(lf));

    //wt = wiretriangle_create(30.0f, 30.0f, COLOR(255, 255, 0, 255));
    //SET3D(OBJ3D(wt)->position, 0.0f, 0.0f, 0.0f);
    //ortmx(OBJ3D(wt)->rotation, NUM3D(90.0f, 0.0f, 0.0f));
    //scene_add_object(scene, OBJ3D(wt));

    overlay = overlay_create();
    overlay_add_object(overlay, overlayline_create(0, 0, 500, 500));

    OverlayTriangle *ot = overlaytriangle_create(200, 200, TRUE);
    ot->color = COLOR(255, 255, 0, 255);
    SET2D(ot->position, -100.0f, -200.0f);
    overlay_add_object(overlay, ot);

    OverlayRectangle *or = overlayrectangle_create(100, 100, TRUE);
    or->color = COLOR(255, 0, 255, 255);
    SET2D(or->position, 300.0f, 100.0f);
    overlay_add_object(overlay, or);

    OverlayCircle *oc = overlaycircle_create(300, FALSE);
    oc->color = COLOR(0, 0, 255, 255);
    overlay_add_object(overlay, oc);

    OverlayCircle *oc2 = overlaycircle_create(100, TRUE);
    oc2->color = COLOR(0, 255, 255, 255);
    overlay_add_object(overlay, oc2);

    OverlayImage *oi = overlayimage_create(200, 200, RESOURCE_DIR "monkey.png");
    SET2D(OVERLAYOBJ(oi)->position, 400.0f, -200.0f);
    overlay_add_object(overlay, OVERLAYOBJ(oi));

    font = font_create(RESOURCE_DIR "font");

    OverlayText *msg = overlaytext_create(font, "AAA This is some text. 1223456789 ABCD !@#$%^&*()-=+<>", -1);
    SET2D(OVERLAYOBJ(msg)->position, -300.0f, -300.0f);
    OVERLAYOBJ(msg)->color = COLOR(128, 255, 128, 255);
    overlay_add_object(overlay, OVERLAYOBJ(msg));

    avg_update_time = overlaytext_create(font, "", 32);
    SET2D(OVERLAYOBJ(avg_update_time)->position, -350.0f, 320.0f);
    overlay_add_object(overlay, OVERLAYOBJ(avg_update_time));

    avg_render_time = overlaytext_create(font, "", 32);
    SET2D(OVERLAYOBJ(avg_render_time)->position, 350.0f, 320.0f);
    overlay_add_object(overlay, OVERLAYOBJ(avg_render_time));

    
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
    if (font) font_destroy(font);
    pthread_mutex_destroy(&scene_mutex);
}

void demo_reshape_viewport(int width, int height) {
    LOG("Reshape viewport %d, %d\n", width, height);
    pthread_mutex_lock(&scene_mutex);
    if (scene) scene_reshape_viewport(scene, width, height);
    if (overlay) overlay_reshape_viewport(overlay, width, height);
    pthread_mutex_unlock(&scene_mutex);
}

static float get_elapsed_time() {
//#if defined(__GNU__) || defined(__APPLE__)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    static double last = 0.0;
    double ms = (double)tv.tv_sec*1000.0+(double)tv.tv_usec/1000.0;
    float el = ms-last;
    last = ms;
    if (el > 1000)
        return 0.0f;
    else
        return (float)el/1000.0f;
/*#elif defined(__MINGW32__)
#define MAX_SAMPLE_COUNT 50
    static float frameTimes[MAX_SAMPLE_COUNT];
    static float timeScale = 0.0f;
    static float actualElapsedTimeSec = 0.0f;
    static INT64 freq = 0;
    static INT64 lastTime = 0;
    static int sampleCount = 0;
    static int initialized = FALSE;
    
    INT64 time = 0;
    float elapsedTimeSec = 0.0f;
    
    if (!initialized) {
        initialized = TRUE;
        QueryPerformanceFrequency((LARGE_INTEGER*)(&freq));
        QueryPerformanceCounter((LARGE_INTEGER*)(&lastTime));
        timeScale = 1.0f / freq;
    }
    
    QueryPerformanceCounter((LARGE_INTEGER*)(&time));
    elapsedTimeSec = (time - lastTime) * timeScale;
    lastTime = time;
    
    if (fabsf(elapsedTimeSec - actualElapsedTimeSec) < 1.0f) {
        memmove(&frameTimes[1], frameTimes, sizeof(frameTimes) - sizeof(frameTimes[0]));
        frameTimes[0] = elapsedTimeSec;
        
        if (sampleCount < MAX_SAMPLE_COUNT)
            ++sampleCount;
    }
    
    actualElapsedTimeSec = 0.0f;
    
    int i;
    for (i = 0; i < sampleCount; ++i)
        actualElapsedTimeSec += frameTimes[i];
    
    if (sampleCount > 0)
        actualElapsedTimeSec /= sampleCount;
    
    return actualElapsedTimeSec;
#endif */
}

#define TARGET_DURATION 0.01666f
char etbuf[64];

static void *update_thread_proc(void *arg) {
    int count = 0;
    float accum = 0.0f;
    char *avgtext = alloca(32);
    Quaternion qA, qP, qT, qDC, qU, qUA;
    
    rotq(&qUA, NUM3D(0.0f, 0.0f, 1.0f), 1.0f);
    
    while(!exit_flag) {
        get_elapsed_time();
        
        pthread_mutex_lock(&scene_mutex);
        
        rotate += 0.01f;
        if (rotate >= 360.0f)
            rotate = 0.0f;
        zpos += 0.003f*zdir;
        xpos += 0.003f*xdir;
        if ((zpos > maxz) || (zpos < -maxz))
            zdir = zdir*-1;
        if ((xpos > maxx) || (xpos < -maxx))
            xdir = xdir*-1;
        
        //SET3D(scene->camera->position, xpos, -40.0f, 5.0f);
        //SET3D(scene->camera->up, 0.0f, 1.0f, 0.0f);
        //rotz3d(&scene->camera->position, DEG2RAD(-rotate));
        
        // adjust camera position
        ortq(&qA, NUM3D(input_x, input_y, input_z));
        mulq(&cam_direction, qA);
        qP = cam_direction;
        mulq(&qP, (Quaternion){0.0f, 0.0f, 1.0f*throttle, 0.0f});
        qDC = cam_direction;
        conjq(&qDC);
        mulq(&qP, qDC);
        add3d(&scene->camera->position, NUM3D(qP.x, qP.y, qP.z));
        
        // adjsut camera direction
        qT = cam_direction;
        mulq(&qT, (Quaternion){0.0f, 0.0f, 10.0f, 0.0f});
        mulq(&qT, qDC);
        SET3D(scene->camera->target, scene->camera->position.x+qT.x,
                                     scene->camera->position.y+qT.y,
                                     scene->camera->position.z+qT.z);
        
        // adjust camera roll
        qU = cam_direction;
        mulq(&qU, qUA);
        mulq(&qU, qDC);
        SET3D(scene->camera->up, qU.x, qU.y, qU.z);
        norm3d(&scene->camera->up);
        
        
        SET3D(OBJ3D(c)->position, 0.0f, 20.0f, 0.0f);
        rotz3d(&OBJ3D(c)->position, DEG2RAD(rotate));
        
        scene_update(scene);

        float el = get_elapsed_time();
        accum += el;
        if (++count >= 1000) {
            float avg = accum/(float)count;
            accum = 0.0f;
            count = 0;
//            sprintf(avgtext, "U:%f", avg);
//            overlaytext_set_text(avg_update_time, avgtext);
        }

        pthread_mutex_unlock(&scene_mutex);

        useconds_t slt;
        if (el < TARGET_DURATION)
            slt = (useconds_t)((TARGET_DURATION-el)*10000);
        else
            slt = 1;

        sprintf(avgtext, "U:%u", slt);
        overlaytext_set_text(avg_update_time, avgtext);

        usleep(slt);
    }
    
    return NULL;
}

void demo_render() {
    static float accum = 0.0f;
    static int count = 0;
    static char avgtext[32];
    
    pthread_mutex_lock(&scene_mutex);
    
    get_elapsed_time();

    if (scene) scene_render(scene);
    if (overlay) overlay_render(overlay);

    float el = get_elapsed_time();
    accum += el;
    if (++count >= 120) {
        float avg = accum/(float)count;
        accum = 0.0f;
        count = 0;
        sprintf(avgtext, "R:%f", avg);
        overlaytext_set_text(avg_render_time, avgtext);
    }
    
    pthread_mutex_unlock(&scene_mutex);
}

void demo_up(int state) {
    LOG("up %d\n", state);
    pthread_mutex_lock(&scene_mutex);
    input_x = state? 0.01f : 0.0f;
    pthread_mutex_unlock(&scene_mutex);
}

void demo_down(int state) {
    LOG("down %d\n", state);
    pthread_mutex_lock(&scene_mutex);
    input_x = state? -0.01f : 0.0f;
    pthread_mutex_unlock(&scene_mutex);
}

void demo_right(int state) {
    LOG("right %d\n", state);
    pthread_mutex_lock(&scene_mutex);
    input_z = state? 0.01f : 0.0f;
    pthread_mutex_unlock(&scene_mutex);
}

void demo_left(int state) {
    LOG("left %d\n", state);
    pthread_mutex_lock(&scene_mutex);
    input_z = state? -0.01f : 0.0f;
    pthread_mutex_unlock(&scene_mutex);
}

void demo_roll_right(int state) {
    LOG("roll right %d\n", state);
    pthread_mutex_lock(&scene_mutex);
    input_y = state? 0.01f : 0.0f;
    pthread_mutex_unlock(&scene_mutex);
}

void demo_roll_left(int state) {
    LOG("roll left %d\n", state);
    pthread_mutex_lock(&scene_mutex);
    input_y = state? -0.01f : 0.0f;
    pthread_mutex_unlock(&scene_mutex);
}

void demo_reset() {
    LOG("roll center\n");
    pthread_mutex_lock(&scene_mutex);
    throttle = 0.0f;
    input_x = 0.0f;
    input_y = 0.0f;
    input_z = 0.0f;
    SET3D(scene->camera->position, 0.0f, -60.0f, 5.0f);
    SET3D(scene->camera->up, 0.0f, 0.0f, 1.0f);
    ortq(&cam_direction, NUM3D(0.0f, 0.0f, 0.0f));
    pthread_mutex_unlock(&scene_mutex);
}

void demo_throttle_up() {
    LOG("throttle up\n");
    pthread_mutex_lock(&scene_mutex);
    throttle += 0.001f;
    pthread_mutex_unlock(&scene_mutex);
}

void demo_throttle_down() {
    LOG("throttle down\n");
    pthread_mutex_lock(&scene_mutex);
    throttle -= 0.001f;
    pthread_mutex_unlock(&scene_mutex);
}

void demo_throttle_reset() {
	LOG("throttle reset\n");
    pthread_mutex_lock(&scene_mutex);
    throttle = 0.0f;
    pthread_mutex_unlock(&scene_mutex);
}
