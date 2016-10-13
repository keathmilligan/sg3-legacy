/* mingw.c - main file for SG3 MinGW version
 * Handles window-creation and other platform-specific tasks
 * Copyright 2012 Keath Milligan
 */

#if defined(__MINGW32__)

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl3/gl3.h>
#include <tmcb/tmcb.h>
#include <mingw32/backtrace.h>
#include "sg3_demo.h"
#include "resources.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720

const char *WINDOW_CLASS = "SG3";
const char *WINDOW_NAME = "SG3mingw";

HINSTANCE hInst;
HWND hWnd;
HDC hDC;
HGLRC hRC;

LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int create_window();
void show_window();
void destroy_window();
int init_gl();
void cleanup_gl();

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
    WNDCLASSEX wc;
    DWORD dwExStyle;
    DWORD dwStyle;
    RECT rect;

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC) wnd_proc;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(AppIcon));
    wc.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(AppIcon));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = WINDOW_CLASS;
    if (!RegisterClassEx(&wc)) {
        LOGERR("RegisterClassEx failed\n");
        return FALSE;
    }

    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    dwStyle = WS_OVERLAPPEDWINDOW;

    ZeroMemory(&rect, sizeof(rect));
    rect.right = (long) DEFAULT_WIDTH;
    rect.bottom = (long) DEFAULT_HEIGHT;
    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

    if (!(hWnd = CreateWindowEx(dwExStyle, WINDOW_CLASS, WINDOW_NAME,
            dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0,
            rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInst,
            NULL ))) {
        LOGERR("CreateWindowEx failed\n");
        UnregisterClass(WINDOW_CLASS, hInst);
        return FALSE;
    }

    return TRUE;
}

/* Show/activate the window
 *
 */
void show_window() {
    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
}

/* Destory main window
 */
void destroy_window() {
    DestroyWindow(hWnd);
    UnregisterClass(WINDOW_CLASS, hInst);
}

/* Initialize OpenGL graphics
 * Returns TRUE on success
 */
int init_gl() {
    PIXELFORMATDESCRIPTOR pfd;
    GLuint pf;

    if (!(hDC = GetDC(hWnd)))
        return FALSE;

    ZeroMemory(&pfd,sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    if ((!(pf = ChoosePixelFormat(hDC, &pfd)))
            || (!SetPixelFormat(hDC, pf, &pfd))
            || (!(hRC = wglCreateContext(hDC)))) {
        ReleaseDC(hWnd, hDC);
        return FALSE;
    }

    if (!wglMakeCurrent(hDC, hRC)) {
        wglDeleteContext(hRC);
        ReleaseDC(hWnd, hDC);
        return FALSE;
    }

    return TRUE;
}

/* Cleanup GL context
 *
 */
void cleanup_gl() {
    wglMakeCurrent(NULL, NULL );
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}

static int direction_key(int state, unsigned int key) {
    switch(key) {
    case VK_UP:
    case VK_NUMPAD8:
        demo_up(state);
        return TRUE;
    case VK_DOWN:
    case VK_NUMPAD2:
        demo_down(state);
        return TRUE;
    case VK_RIGHT:
    case VK_NUMPAD6:
        demo_right(state);
        return TRUE;
    case VK_LEFT:
    case VK_NUMPAD4:
        demo_left(state);
        return TRUE;
    case VK_NEXT:
    case VK_NUMPAD3:
        demo_roll_right(state);
        return TRUE;
    case VK_END:
    case VK_NUMPAD1:
        demo_roll_left(state);
        return TRUE;
    default:
        return FALSE;
    }
}

/* Main window proc
 */
LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        demo_reshape_viewport(LOWORD(lParam), HIWORD(lParam) );
        return 0;
    case WM_KEYDOWN:
        switch(wParam) {
        case VK_HOME:
        case VK_NUMPAD7:
            demo_roll_center();
            return 0;
        default:
            if (direction_key(TRUE, wParam)) return 0;
            break;
        }
        break;
    case WM_KEYUP:
        if (direction_key(FALSE, wParam)) return 0;
        return 0;
    case WM_CHAR:
        switch(wParam) {
        case VK_ADD:
        case '+':
        case '=':
            demo_throttle_up();
            return 0;
        case VK_SUBTRACT:
        case '-':
        case '_':
            demo_throttle_down();
            return 0;
        }
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/* SG3mingw - main function
 * Create application window, setup graphics, handle main loop
 */
//int main(int argc, char **argv) {
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MSG msg;

    backtrace_register();

    LOG("SG3mingw - initializing\n");

    hInst = GetModuleHandle(NULL );

    // create the main window & initialize graphics context
    if (!create_window()) {
        LOGERR("failed to create main window\n");
        return 1;
    }

        // initialize OpenGL
    if (!init_gl()) {
        destroy_window();
        LOGERR("failed to initialize OpenGL graphics context\n");
        return 1;
    }

    LOG("GL_VENDOR = %s\n", glGetString(GL_VENDOR));
    LOG("GL_VERSION = %s\n", glGetString(GL_VERSION));
    LOG("GL_RENDERER = %s\n", glGetString(GL_RENDERER));

    // initialize the scene

    if (!demo_init()) {
        cleanup_gl();
        destroy_window();
        LOGERR("failed to initialize OpenGL scene\n");
        return 1;
    }

    show_window();

    // main loop
    while (TRUE) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        demo_render();
        SwapBuffers(hDC);
    }

    demo_cleanup();
    cleanup_gl();
    destroy_window();

    backtrace_unregister();
    tcheck();


    return EXIT_SUCCESS;
}

#endif
