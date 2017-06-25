#ifndef CB_ENGINE_H
#define CB_ENGINE_H

#include <stdbool.h>

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

// init/destroy engine
void cbInit();
void cbDestroy();

// callbacks
void cbOnStart(void (*func)(void));
void cbOnUpdate(void (*func)(float));
void cbOnStop(void (*func)(void));

// window width, height, title
void cbGetSize(int *w, int *h);
void cbSetSize(int w, int h);
const char* cbGetTitle();
void cbSetTitle(const char *title);

// input system
bool cbIsKeyDown(cbKey k);
bool cbIsMouseDown(cbMouse m);
void cbGetMousePos(int *x, int *y);
void cbSetMousePos(int x, int y);

// enter/exit main cycle
void cbRun();
void cbExit();

#endif
