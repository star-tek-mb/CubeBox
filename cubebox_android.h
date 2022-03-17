#ifndef CB_ENGINE_H
#define CB_ENGINE_H

#include <stdbool.h>

#include <stdio.h>
#include <GLES2/gl2.h>
#include <android/log.h>
FILE* android_fopen(const char* fileName, const char* mode);
#define fopen(name, mode) android_fopen(name, mode)
#define CB_LOG(...) __android_log_print(ANDROID_LOG_INFO, "cubebox", __VA_ARGS__)

typedef enum {
    CB_MOUSE_UNKNOWN = -1,
    CB_MOUSE_LEFT = 0,
    CB_MOUSE_RIGHT,
    CB_MOUSE_MIDDLE,
    CB_MOUSE_TOTAL // count
} cbMouse;

typedef enum {
    CB_KEY_UNKNOWN = -1,
    CB_KEY_A = 0,
    CB_KEY_B,
    CB_KEY_C,
    CB_KEY_D,
    CB_KEY_E,
    CB_KEY_F,
    CB_KEY_G,
    CB_KEY_H,
    CB_KEY_I,
    CB_KEY_J,
    CB_KEY_K,
    CB_KEY_L,
    CB_KEY_M,
    CB_KEY_N,
    CB_KEY_O,
    CB_KEY_P,
    CB_KEY_Q,
    CB_KEY_R,
    CB_KEY_S,
    CB_KEY_T,
    CB_KEY_U,
    CB_KEY_V,
    CB_KEY_W,
    CB_KEY_X,
    CB_KEY_Y,
    CB_KEY_Z,
    CB_KEY_NUM0,
    CB_KEY_NUM1,
    CB_KEY_NUM2,
    CB_KEY_NUM3,
    CB_KEY_NUM4,
    CB_KEY_NUM5,
    CB_KEY_NUM6,
    CB_KEY_NUM7,
    CB_KEY_NUM8,
    CB_KEY_NUM9,
    CB_KEY_ESCAPE,
    CB_KEY_LCONTROL,
    CB_KEY_LSHIFT,
    CB_KEY_LALT,
    CB_KEY_LSYSTEM,
    CB_KEY_RCONTROL,
    CB_KEY_RSHIFT,
    CB_KEY_RALT,
    CB_KEY_RSYSTEM,
    CB_KEY_MENU,
    CB_KEY_LBRACKET,
    CB_KEY_RBRACKET,
    CB_KEY_SEMICOLON,
    CB_KEY_COMMA,
    CB_KEY_PERIOD,
    CB_KEY_QUOTE,
    CB_KEY_SLASH,
    CB_KEY_BACKSLASH,
    CB_KEY_TILDE,
    CB_KEY_EQUAL,
    CB_KEY_DASH,
    CB_KEY_SPACE,
    CB_KEY_RETURN,
    CB_KEY_BACKSPACE,
    CB_KEY_TAB,
    CB_KEY_PAGEUP,
    CB_KEY_PAGEDOWN,
    CB_KEY_END,
    CB_KEY_HOME,
    CB_KEY_INSERT,
    CB_KEY_DEL,
    CB_KEY_ADD,
    CB_KEY_SUBTRACT,
    CB_KEY_MULTIPLY,
    CB_KEY_DIVIDE,
    CB_KEY_LEFT,
    CB_KEY_RIGHT,
    CB_KEY_UP,
    CB_KEY_DOWN,
    CB_KEY_NUMPAD0,
    CB_KEY_NUMPAD1,
    CB_KEY_NUMPAD2,
    CB_KEY_NUMPAD3,
    CB_KEY_NUMPAD4,
    CB_KEY_NUMPAD5,
    CB_KEY_NUMPAD6,
    CB_KEY_NUMPAD7,
    CB_KEY_NUMPAD8,
    CB_KEY_NUMPAD9,
    CB_KEY_F1,
    CB_KEY_F2,
    CB_KEY_F3,
    CB_KEY_F4,
    CB_KEY_F5,
    CB_KEY_F6,
    CB_KEY_F7,
    CB_KEY_F8,
    CB_KEY_F9,
    CB_KEY_F10,
    CB_KEY_F11,
    CB_KEY_F12,
    CB_KEY_F13,
    CB_KEY_F14,
    CB_KEY_F15,
    CB_KEY_PAUSE,
    CB_KEY_TOTAL // count
} cbKey;

typedef struct {
    int width;
    int height;
    char* title;
    void (*start)(void);
    void (*update)(float);
    void (*stop)(void);
} cbInitParams;

void cbInit(cbInitParams params);
void cbDestroy();

void cbGetSize(int* w, int* h);
void cbSetSize(int w, int h);

bool cbIsKeyDown(cbKey k);
bool cbIsMouseDown(cbMouse m);
void cbGetMousePos(int* x, int* y);
void cbSetMousePos(int x, int y);

void cbRun();
void cbExit();

#endif

#ifdef CB_IMPLEMENTATION

#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static struct {
    struct android_app* app;
    int w;
    int h;
    bool animating;
    bool active;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLConfig config;
    bool mouse[CB_MOUSE_TOTAL];
    bool keyboard[CB_KEY_TOTAL];
    void (*onStartFunc)(void);
    void (*onUpdateFunc)(float);
    void (*onStopFunc)(void);
    int mx;
    int my;
    AAssetManager* assetManager;
    const char* internalDataPath;
} cubebox;

static cbKey convert_key(int32_t code)
{
    switch (code) {
    case AKEYCODE_BACK:
        return CB_KEY_ESCAPE;
    case AKEYCODE_0:
        return CB_KEY_NUM0;
    case AKEYCODE_1:
        return CB_KEY_NUM1;
    case AKEYCODE_2:
        return CB_KEY_NUM2;
    case AKEYCODE_3:
        return CB_KEY_NUM3;
    case AKEYCODE_4:
        return CB_KEY_NUM4;
    case AKEYCODE_5:
        return CB_KEY_NUM5;
    case AKEYCODE_6:
        return CB_KEY_NUM6;
    case AKEYCODE_7:
        return CB_KEY_NUM7;
    case AKEYCODE_8:
        return CB_KEY_NUM8;
    case AKEYCODE_9:
        return CB_KEY_NUM9;
    case AKEYCODE_A:
        return CB_KEY_A;
    case AKEYCODE_B:
        return CB_KEY_B;
    case AKEYCODE_C:
        return CB_KEY_C;
    case AKEYCODE_D:
        return CB_KEY_D;
    case AKEYCODE_E:
        return CB_KEY_E;
    case AKEYCODE_F:
        return CB_KEY_F;
    case AKEYCODE_G:
        return CB_KEY_G;
    case AKEYCODE_H:
        return CB_KEY_H;
    case AKEYCODE_I:
        return CB_KEY_I;
    case AKEYCODE_J:
        return CB_KEY_J;
    case AKEYCODE_K:
        return CB_KEY_K;
    case AKEYCODE_L:
        return CB_KEY_L;
    case AKEYCODE_M:
        return CB_KEY_M;
    case AKEYCODE_N:
        return CB_KEY_N;
    case AKEYCODE_O:
        return CB_KEY_O;
    case AKEYCODE_P:
        return CB_KEY_P;
    case AKEYCODE_Q:
        return CB_KEY_Q;
    case AKEYCODE_R:
        return CB_KEY_R;
    case AKEYCODE_S:
        return CB_KEY_S;
    case AKEYCODE_T:
        return CB_KEY_T;
    case AKEYCODE_U:
        return CB_KEY_U;
    case AKEYCODE_V:
        return CB_KEY_V;
    case AKEYCODE_W:
        return CB_KEY_W;
    case AKEYCODE_X:
        return CB_KEY_X;
    case AKEYCODE_Y:
        return CB_KEY_Y;
    case AKEYCODE_Z:
        return CB_KEY_Z;
    case AKEYCODE_COMMA:
        return CB_KEY_COMMA;
    case AKEYCODE_PERIOD:
        return CB_KEY_PERIOD;
    case AKEYCODE_ALT_LEFT:
        return CB_KEY_LALT;
    case AKEYCODE_ALT_RIGHT:
        return CB_KEY_RALT;
    case AKEYCODE_SHIFT_LEFT:
        return CB_KEY_LSHIFT;
    case AKEYCODE_SHIFT_RIGHT:
        return CB_KEY_RSHIFT;
    case AKEYCODE_TAB:
        return CB_KEY_TAB;
    case AKEYCODE_SPACE:
        return CB_KEY_SPACE;
    case AKEYCODE_ENTER:
        return CB_KEY_RETURN;
    case AKEYCODE_DEL:
        return CB_KEY_DEL;
    case AKEYCODE_GRAVE:
        return CB_KEY_TILDE;
    case AKEYCODE_MINUS:
        return CB_KEY_SUBTRACT;
    case AKEYCODE_EQUALS:
        return CB_KEY_EQUAL;
    case AKEYCODE_LEFT_BRACKET:
        return CB_KEY_LBRACKET;
    case AKEYCODE_RIGHT_BRACKET:
        return CB_KEY_RBRACKET;
    case AKEYCODE_BACKSLASH:
        return CB_KEY_BACKSLASH;
    case AKEYCODE_SEMICOLON:
        return CB_KEY_SEMICOLON;
    case AKEYCODE_APOSTROPHE:
        return CB_KEY_QUOTE;
    case AKEYCODE_SLASH:
        return CB_KEY_SLASH;
    case AKEYCODE_PAGE_UP:
        return CB_KEY_PAGEUP;
    case AKEYCODE_PAGE_DOWN:
        return CB_KEY_PAGEDOWN;
    }
    return CB_KEY_UNKNOWN;
}

static void init_context()
{
    const static EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_RED_SIZE, 5,
        EGL_DEPTH_SIZE, 16,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    const static EGLint version[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    EGLConfig* supportedConfigs;

    if (cubebox.display == EGL_NO_DISPLAY && cubebox.context == EGL_NO_CONTEXT) {
        EGLint numConfigs;
        cubebox.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(cubebox.display, NULL, NULL);
        if (cubebox.display == EGL_NO_DISPLAY)
            printf("display not created");

        // choose the best config
        eglChooseConfig(cubebox.display, attribs, NULL, 0, &numConfigs);
        if (!numConfigs)
            printf("framebuffer not found");
        supportedConfigs = (EGLConfig*) malloc(sizeof(EGLConfig) * numConfigs);
        eglChooseConfig(cubebox.display, attribs, supportedConfigs, numConfigs, &numConfigs);
        cubebox.config = supportedConfigs[0]; // choose first one

        cubebox.context = eglCreateContext(cubebox.display, cubebox.config, NULL, version);
        if (cubebox.context == EGL_NO_CONTEXT)
            printf("context not created");
    }

    // reconfigure buffers
    EGLint format;
    eglGetConfigAttrib(cubebox.display, cubebox.config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(cubebox.app->window, 0, 0, format);
    cubebox.surface = eglCreateWindowSurface(cubebox.display, cubebox.config, cubebox.app->window, NULL);
    if (cubebox.surface == EGL_NO_SURFACE)
        printf("surface not created");

    if (eglMakeCurrent(cubebox.display, cubebox.surface, cubebox.surface, cubebox.context) == EGL_FALSE) {
        printf("cannot make context current");
    }

    // get width and height of window
    eglQuerySurface(cubebox.display, cubebox.surface, EGL_WIDTH, &cubebox.w);
    eglQuerySurface(cubebox.display, cubebox.surface, EGL_HEIGHT, &cubebox.h);

    free(supportedConfigs);
}

static void destroy_surface()
{
    if (cubebox.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(cubebox.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    if (cubebox.surface != EGL_NO_SURFACE) {
        eglDestroySurface(cubebox.display, cubebox.surface);
        cubebox.surface = EGL_NO_SURFACE;
    }
}

static void destroy_context()
{
    if (cubebox.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(cubebox.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (cubebox.context != EGL_NO_CONTEXT) {
            eglDestroyContext(cubebox.display, cubebox.context);
        }
        if (cubebox.surface != EGL_NO_SURFACE) {
            eglDestroySurface(cubebox.display, cubebox.surface);
        }
        eglTerminate(cubebox.display);
    }

    cubebox.display = EGL_NO_DISPLAY;
    cubebox.context = EGL_NO_CONTEXT;
    cubebox.surface = EGL_NO_SURFACE;
}

static int32_t handle_input(struct android_app* app, AInputEvent* event)
{
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        cubebox.mx = AMotionEvent_getX(event, 0);
        cubebox.my = AMotionEvent_getY(event, 0);

        int type = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

        if (type == AMOTION_EVENT_ACTION_DOWN) {
            cubebox.mouse[CB_MOUSE_LEFT] = true;
        }
        if (type == AMOTION_EVENT_ACTION_UP) {
            cubebox.mouse[CB_MOUSE_LEFT] = false;
        }
        return 1;
    }
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        int action = AKeyEvent_getAction(event);
        int key = AKeyEvent_getKeyCode(event);
        int metakey = AKeyEvent_getMetaState(event);

        cbKey code = convert_key(key);
        bool alt = metakey & AMETA_ALT_ON;
        bool control = false;
        bool shift = metakey & AMETA_SHIFT_ON;
        bool system = false;
        cubebox.keyboard[CB_KEY_LALT] = cubebox.keyboard[CB_KEY_RALT] = alt;
        cubebox.keyboard[CB_KEY_LSHIFT] = cubebox.keyboard[CB_KEY_RSHIFT] = shift;
        if (action == AKEY_EVENT_ACTION_DOWN) {
            cubebox.keyboard[code] = true;
        }
        if (action == AKEY_EVENT_ACTION_UP) {
            cubebox.keyboard[code] = false;
        }
        return 1;
    }
    return 0;
}

static void handle_command(struct android_app* app, int32_t cmd)
{
    switch (cmd) {
    case APP_CMD_INIT_WINDOW:
        if (app->window != NULL) {
            init_context();
            cubebox.active = true;
        }
        break;
    case APP_CMD_TERM_WINDOW:
        destroy_surface();
        cubebox.active = false;
        break;
    case APP_CMD_DESTROY:
        destroy_context();
        cubebox.active = false;
        break;
    case APP_CMD_PAUSE:
        cubebox.animating = false;
        break;
    case APP_CMD_RESUME:
        cubebox.animating = true;
        break;
    case APP_CMD_CONFIG_CHANGED:
        if (app->window != NULL) {
            destroy_surface();
            init_context();
        }
        break;
    }
}

extern int main(int argc, char** argv);

void android_main(struct android_app* ctx) {
    cubebox.app = ctx;
    cubebox.app->onAppCmd = handle_command;
    cubebox.app->onInputEvent = handle_input;
    cubebox.assetManager = ctx->activity->assetManager;
    cubebox.internalDataPath = ctx->activity->internalDataPath;
    main(0, NULL);
}

void cbInit(cbInitParams params) {
    if (params.start) {
        cubebox.onStartFunc = params.start;
    }
    if (params.stop) {
        cubebox.onStopFunc = params.stop;
    }
    if (params.update) {
        cubebox.onUpdateFunc = params.update;
    }
}

void cbDestroy() {
}

void cbGetSize(int* w, int* h) {
    *w = cubebox.w;
    *h = cubebox.h;
}
void cbSetSize(int w, int h) {

}

bool cbIsKeyDown(cbKey k) {
    return cubebox.keyboard[k];
}

bool cbIsMouseDown(cbMouse m) {
    return cubebox.mouse[m];
}

void cbGetMousePos(int* x, int* y) {
    *x = cubebox.mx;
    *y = cubebox.my;
}

void cbSetMousePos(int x, int y) {

}

void cbRun() {
    while (true) {
        int ident;
        int events;
        struct android_poll_source* source;

        while ((ident = ALooper_pollAll(cubebox.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) {
                source->process(cubebox.app, source);
            }

            if (cubebox.app->destroyRequested != 0) {
                destroy_context();
                return;
            }
        }

        if (cubebox.active && cubebox.animating) {
            cubebox.onUpdateFunc(0.0f);
            int rc = eglSwapBuffers(cubebox.display, cubebox.surface);
            if (rc != EGL_TRUE) {
                EGLint error = eglGetError();
                if (error == EGL_BAD_NATIVE_WINDOW) {
                    if (cubebox.app->window != NULL) {
                        destroy_surface();
                        init_context();
                    }
                    cubebox.active = true;
                }
            }
        }
    }
}

void cbExit() {
    ANativeActivity_finish(cubebox.app->activity);
}

FILE *funopen(
    const void *cookie,
    int (*readfn)(void *, char *, int),
    int (*writefn)(void *, const char *, int),
    fpos_t (*seekfn)(void *, fpos_t, int),
    int (*closefn)(void *)
);

static int android_read(void *cookie, char *buf, int size) {
    return AAsset_read((AAsset *)cookie, buf, size);
}

static int android_write(void *cookie, const char *buf, int size) {
    return EACCES;
}

static fpos_t android_seek(void *cookie, fpos_t offset, int whence) {
    return AAsset_seek((AAsset *)cookie, offset, whence);
}

static int android_close(void *cookie) {
    AAsset_close((AAsset *)cookie);
    return 0;
}

FILE *android_fopen(const char *filename, const char *mode) {
    if (mode[0] == 'w') {
        #undef fopen
        char path[512];
        sprintf(path, "%s/%s", cubebox.internalDataPath, filename);
        return fopen(path, mode);
        #define fopen(name, mode) android_fopen(name, mode)
    } else {
        AAsset* asset = AAssetManager_open(cubebox.assetManager, filename, AASSET_MODE_UNKNOWN);
        if (asset != NULL) {
            return funopen(asset, android_read, android_write, android_seek, android_close);
        } else {
            #undef fopen
            char path[512];
            sprintf(path, "%s/%s", cubebox.internalDataPath, filename);
            return fopen(path, mode);
            #define fopen(name, mode) android_fopen(name, mode)
        }
    }
}

#endif
