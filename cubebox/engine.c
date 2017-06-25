#include "engine.h"

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "glad.h"
#include <GL/glx.h>
#include <X11/Xlib.h>

// configure single-file libs
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// x window system
static Display* display;
static Window surface;
static GLXContext context;
static Colormap colormap;
static Atom closeWindowEvent;
static int width = 640, height = 480;
static bool preventMouseLoop;
static bool running;

// engine params
static char title[255];
static bool keyStates[CB_KEY_TOTAL];
static bool mouseStates[CB_MOUSE_TOTAL];
static void (*onStartFunc)(void) = NULL;
static void (*onUpdateFunc)(float) = NULL;
static void (*onStopFunc)(void) = NULL;

// some extensions used
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef void (*glXSwapIntervalEXTProc)(Display* dpy, GLXDrawable drawable, int interval);

// x error check
static bool xError = false;
static int errorHandler(Display* dsp, XErrorEvent* xev) {
    printf("X Window System error");
    xError = true;
    return 0;
}

// convert mouse
static cbMouse convertMouse(unsigned int button)
{
    switch (button) {
    case Button1:
        return CB_MOUSE_LEFT;
    case Button2:
        return CB_MOUSE_MIDDLE;
    case Button3:
        return CB_MOUSE_RIGHT;
    }
    return CB_MOUSE_UNKNOWN;
}

// some magic constants compared with key codes
static cbKey convertKey(KeySym code)
{
    switch (code) {
    case 50:
        return CB_KEY_LSHIFT;
    case 62:
        return CB_KEY_RSHIFT;
    case 37:
        return CB_KEY_LCONTROL;
    case 105:
        return CB_KEY_RCONTROL;
    case 64:
        return CB_KEY_LALT;
    case 108:
        return CB_KEY_RALT;
    case 133:
        return CB_KEY_LSYSTEM;
    case 134:
        return CB_KEY_RSYSTEM;
    case XK_Menu:
        return CB_KEY_MENU;
    case 9:
        return CB_KEY_ESCAPE;
    case 47:
        return CB_KEY_SEMICOLON;
    case 61:
        return CB_KEY_SLASH;
    case 21:
        return CB_KEY_EQUAL;
    case 20:
        return CB_KEY_DASH;
    case 34:
        return CB_KEY_LBRACKET;
    case 35:
        return CB_KEY_RBRACKET;
    case 59:
        return CB_KEY_COMMA;
    case 60:
        return CB_KEY_PERIOD;
    case 48:
        return CB_KEY_QUOTE;
    case 51:
        return CB_KEY_BACKSLASH;
    case 49:
        return CB_KEY_TILDE;
    case 65:
        return CB_KEY_SPACE;
    case 36:
        return CB_KEY_RETURN;
    case 104:
        return CB_KEY_RETURN;
    case 22:
        return CB_KEY_BACKSPACE;
    case 23:
        return CB_KEY_TAB;
    case 112:
        return CB_KEY_PAGEUP;
    case 117:
        return CB_KEY_PAGEDOWN;
    case 115:
        return CB_KEY_END;
    case 110:
        return CB_KEY_HOME;
    case 118:
        return CB_KEY_INSERT;
    case 119:
        return CB_KEY_DEL;
    case 86:
        return CB_KEY_ADD;
    case 82:
        return CB_KEY_SUBTRACT;
    case 63:
        return CB_KEY_MULTIPLY;
    case 106:
        return CB_KEY_DIVIDE;
    case 127:
        return CB_KEY_PAUSE;
    case 67:
        return CB_KEY_F1;
    case 68:
        return CB_KEY_F2;
    case 69:
        return CB_KEY_F3;
    case 70:
        return CB_KEY_F4;
    case 71:
        return CB_KEY_F5;
    case 72:
        return CB_KEY_F6;
    case 73:
        return CB_KEY_F7;
    case 74:
        return CB_KEY_F8;
    case 75:
        return CB_KEY_F9;
    case 76:
        return CB_KEY_F10;
    case 95:
        return CB_KEY_F11;
    case 96:
        return CB_KEY_F12;
    case 191:
        return CB_KEY_F13;
    case 192:
        return CB_KEY_F14;
    case 193:
        return CB_KEY_F15;
    case 113:
        return CB_KEY_LEFT;
    case 114:
        return CB_KEY_RIGHT;
    case 111:
        return CB_KEY_UP;
    case 116:
        return CB_KEY_DOWN;
    case 90:
        return CB_KEY_NUMPAD0;
    case 87:
        return CB_KEY_NUMPAD1;
    case 88:
        return CB_KEY_NUMPAD2;
    case 89:
        return CB_KEY_NUMPAD3;
    case 83:
        return CB_KEY_NUMPAD4;
    case 84:
        return CB_KEY_NUMPAD5;
    case 85:
        return CB_KEY_NUMPAD6;
    case 79:
        return CB_KEY_NUMPAD7;
    case 80:
        return CB_KEY_NUMPAD8;
    case 81:
        return CB_KEY_NUMPAD9;
    case 38:
        return CB_KEY_A;
    case 56:
        return CB_KEY_B;
    case 54:
        return CB_KEY_C;
    case 40:
        return CB_KEY_D;
    case 26:
        return CB_KEY_E;
    case 41:
        return CB_KEY_F;
    case 42:
        return CB_KEY_G;
    case 43:
        return CB_KEY_H;
    case 31:
        return CB_KEY_I;
    case 44:
        return CB_KEY_J;
    case 45:
        return CB_KEY_K;
    case 46:
        return CB_KEY_L;
    case 58:
        return CB_KEY_M;
    case 57:
        return CB_KEY_N;
    case 32:
        return CB_KEY_O;
    case 33:
        return CB_KEY_P;
    case 24:
        return CB_KEY_Q;
    case 27:
        return CB_KEY_R;
    case 39:
        return CB_KEY_S;
    case 28:
        return CB_KEY_T;
    case 30:
        return CB_KEY_U;
    case 55:
        return CB_KEY_V;
    case 25:
        return CB_KEY_W;
    case 53:
        return CB_KEY_X;
    case 29:
        return CB_KEY_Y;
    case 52:
        return CB_KEY_Z;
    case 19:
        return CB_KEY_NUM0;
    case 10:
        return CB_KEY_NUM1;
    case 11:
        return CB_KEY_NUM2;
    case 12:
        return CB_KEY_NUM3;
    case 13:
        return CB_KEY_NUM4;
    case 14:
        return CB_KEY_NUM5;
    case 15:
        return CB_KEY_NUM6;
    case 16:
        return CB_KEY_NUM7;
    case 17:
        return CB_KEY_NUM8;
    case 18:
        return CB_KEY_NUM9;
    }
    return CB_KEY_UNKNOWN;
}

void cbInit() {
    // x11 display
    display = XOpenDisplay(NULL);
    if (!display) {
        printf("X Window System display is NULL\n");
    }

    // opengl window config
    static int visualAttribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        GLX_SAMPLE_BUFFERS, 1,
        GLX_SAMPLES, 4,
        None
    };

    // glx
    int glxMajor, glxMinor;
    if (!glXQueryVersion(display, &glxMajor, &glxMinor) || ((glxMajor == 1) && (glxMinor < 3)) || (glxMajor < 1)) {
        printf("GLX version < 1.3\n");
    }

    // find config
    int fbCount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visualAttribs, &fbCount);
    if (!fbc) {
        printf("GLX no config matched\n");
    }

    // select first
    GLXFBConfig bestFbc = fbc[0];
    XFree(fbc);
    XVisualInfo* vi = glXGetVisualFromFBConfig(display, bestFbc);

    // colormap
    XSetWindowAttributes swa;
    swa.colormap = colormap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
    swa.background_pixmap = None;
    swa.border_pixel = 0;
    swa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonMotionMask | ButtonPressMask | ButtonReleaseMask;

    // window
    surface = XCreateWindow(display, RootWindow(display, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
    if (!surface) {
        printf("X Window System surface is 0\n");
    }
    XFree(vi);
    XMapWindow(display, surface);
    XAutoRepeatOn(display);

    // register close event
    closeWindowEvent = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, surface, &closeWindowEvent, 1);

    // find glXCreateContextAttribsARB
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB("glXCreateContextAttribsARB");
    if (glXCreateContextAttribsARB == NULL)
        printf("glXCreateContextAttribsARB function is NULL\n");

    // set error handler
    xError = false;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&errorHandler);
    // create context with attribs
    int contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };
    context = glXCreateContextAttribsARB(display, bestFbc, 0, True, contextAttribs);

    // check for errors
    XSync(display, False);
    if (xError || !context) {
        printf("GLX context is 0\n");
    }
    XSync(display, False);
    XSetErrorHandler(oldHandler);
    if (xError || !context) {
        printf("GLX context is 0\n");
    }
    if (!glXIsDirect(display, context)) {
        printf("GLX context is not direct\n");
    }

    // load OpenGL functions
    glXMakeCurrent(display, surface, context);
    if (!gladLoadGLLoader((GLADloadproc) glXGetProcAddressARB))
        printf("GLAD failed to load OpenGL functions\n");

    glEnable(GL_MULTISAMPLE);
}

void cbDestroy() {
    glXMakeCurrent(display, 0, 0);
    glXDestroyContext(display, context);
    XDestroyWindow(display, surface);
    XFreeColormap(display, colormap);
    XCloseDisplay(display);
}

void cbOnStart(void (*func)(void)) {
    onStartFunc = func;
}

void cbOnUpdate(void (*func)(float)) {
    onUpdateFunc = func;
}

void cbOnStop(void (*func)(void)) {
    onStopFunc = func;
}

void cbGetSize(int *w, int *h) {
    *w = width;
    *h = height;
}

void cbSetSize(int w, int h) {
    width = w;
    height = h;
    XResizeWindow(display, surface, w, h);
}

const char* cbGetTitle() {
    return title;
}

void cbSetTitle(const char* t) {
    snprintf(title, 255, "%s", t);
    XStoreName(display, surface, title);
}

bool cbIsKeyDown(cbKey k) {
    return keyStates[k];
}

bool cbIsMouseDown(cbMouse m) {
    return mouseStates[m];
}

void cbGetMousePos(int *x, int *y) {
    Window w;
    int rootx, rooty, mx, my;
    unsigned int mask;
    XQueryPointer(display, surface, &w, &w, &rootx, &rooty, &mx, &my, &mask);
    *x = mx;
    *y = my;
}

void cbSetMousePos(int x, int y) {
    XWarpPointer(display, None, surface, 0, 0, 0, 0, x, y);
    preventMouseLoop = true;
}

void cbRun() {
    onStartFunc();
    running = true;

    bool keyRepeating; // prevent keyboard repeating

    // measure frame time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    XEvent event;
    while (running) {

        end = start;
        clock_gettime(CLOCK_MONOTONIC, &start);

        while (XPending(display)) {
            XNextEvent(display, &event);
            switch (event.type) {
            case ConfigureNotify:
                width = event.xconfigure.width;
                height = event.xconfigure.height;
                glViewport(0, 0, width, height);
                break;
            case ClientMessage:
                if (event.xclient.data.l[0] == closeWindowEvent) {
                    running = false;
                }
                break;
            case KeyPress:
                keyStates[convertKey(event.xkey.keycode)] = true;
                break;
            case KeyRelease:
                keyRepeating = false;
                if (XPending(display)) {
                    XEvent next;
                    XPeekEvent(display, &next);

                    if (next.type == KeyPress && next.xkey.time - event.xkey.time < 2 && next.xkey.keycode == event.xkey.keycode) {
                        // delete event
                        XNextEvent (display, &event);
                        keyRepeating = true;
                    }
                }

                if (!keyRepeating) {
                    keyStates[convertKey(event.xkey.keycode)] = false;
                }
                break;
            case ButtonPress:
                mouseStates[convertMouse(event.xbutton.button)] = true;
                break;
            case ButtonRelease:
                mouseStates[convertMouse(event.xbutton.button)] = false;
                break;
            }
        }

        float delta = (float) (double)(start.tv_sec - end.tv_sec) + ((double)(start.tv_nsec - end.tv_nsec) * 1.0e-9);
        onUpdateFunc(delta);
        glXSwapBuffers(display, surface);
    }

    onStopFunc();
}

void cbExit() {
    running = false;
}
