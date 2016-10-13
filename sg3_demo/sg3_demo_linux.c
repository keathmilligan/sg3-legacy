/*
 * sg3_demo_linux.c
 *
 *  Created on: Mar 8, 2013
 *      Author: kmilligan
 */

#if defined(__linux__)

#include <stdio.h>
#include <unistd.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gl3/gl3.h>
#include <tmcb/tmcb.h>
#include "sg3_demo.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720

int window;

int create_window();
void resize_scene(int width, int height);
void draw_scene();
void key_pressed(unsigned char key, int x, int y);
void key_released(unsigned char key, int x, int y);
void direction_key(int state, int key);
void special_pressed(int key, int x, int y);
void special_released(int key, int x, int y);

/* Platform-specific log handlers
 */
void _log_std_output(const char *msg) {
    fprintf(stdout, "SG3: %s", msg);
    fflush(stdout);
}

void _log_err_output(const char *msg) {
    fprintf(stderr, "SG3: ERROR: %s", msg);
    fflush(stderr);
}

/* Create application window
 * Returns TRUE on success
 */
int create_window() {
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
	//glutInitWindowPosition(0, 0);
	window = glutCreateWindow("SG3 Linux");
	if (window <= 0)
		return FALSE;
	glutDisplayFunc(&draw_scene);
	glutIdleFunc(&draw_scene);
	glutReshapeFunc(&resize_scene);
	glutKeyboardFunc(&key_pressed);
	glutKeyboardUpFunc(&key_released);
	glutSpecialFunc(&special_pressed);
	glutSpecialUpFunc(&special_released);
	glutShowWindow();
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	return TRUE;
}

/* Handle window resize event
 * Resizes opengl scene
 */
void resize_scene(int width, int height) {
	if (height == 0)
		height = 1;
	demo_reshape_viewport(width, height);
}

/* Handle draw callback
 *
 */
void draw_scene() {
	demo_render();
	glutSwapBuffers();
}

/* Handle key press
 *
 */
void key_pressed(unsigned char key, int x, int y) {
	usleep(100);
	switch (key) {
	case 27:
		glutLeaveMainLoop();
		break;
	case '+':
	case '=':
		demo_throttle_up();
		break;
	case '-':
	case '_':
		demo_throttle_down();
		break;
	case '0':
		demo_throttle_reset();
		break;
	}
}

/* Handle key release
 *
 */
void key_released(unsigned char key, int x, int y) {

}

/* Handle direction keys
 *
 */
void direction_key(int state, int key) {
	switch(key) {
	case GLUT_KEY_UP:
		demo_up(state);
		break;
	case GLUT_KEY_DOWN:
		demo_down(state);
		break;
	case GLUT_KEY_RIGHT:
		demo_right(state);
		break;
	case GLUT_KEY_LEFT:
		demo_left(state);
		break;
	case GLUT_KEY_END:
		demo_roll_right(state);
		break;
	case GLUT_KEY_PAGE_DOWN:
		demo_roll_left(state);
		break;
	}
}

/* Handle special key press
 *
 */
void special_pressed(int key, int x, int y) {
	switch(key) {
	case GLUT_KEY_HOME:
		demo_reset();
		break;
	case GLUT_KEY_INSERT:
		demo_throttle_reset();
		break;
	default:
		direction_key(TRUE, key);
		break;
	}
}

/* Handle special key release
 *
 */
void special_released(int key, int x, int y) {
	direction_key(FALSE, key);
}

/* main - create app window, setup graphics, handle main loop
 */
int main(int argc, char **argv) {
	glutInit(&argc, argv);
	if (!create_window()) {
		LOGERR("could not create app window\n");
		return 1;
	}

	LOG("GL_VENDOR = %s\n", glGetString(GL_VENDOR));
    LOG("GL_VERSION = %s\n", glGetString(GL_VERSION));
    LOG("GL_RENDERER = %s\n", glGetString(GL_RENDERER));

    if (!demo_init()) {
		LOGERR("failed to initialize demo scene\n");
		return 1;
	}
	glutMainLoop();

	LOG("terminating\n");
	//glutDestroyWindow(window);
	demo_cleanup();
	tcheck();
	exit(0);
}

#endif
