#ifndef CB_ENGINE_H
#define CB_ENGINE_H

#include <stdbool.h>

#ifdef CB_IMPLEMENTATION
#define GLAD_GL_IMPLEMENTATION
#define RAYMATH_IMPLEMENTATION
#define MINIAUDIO_IMPLEMENTATION
#endif

#include "gl.h"
#include "raymath.h"
#include "miniaudio.h"

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

#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HINSTANCE hinstance;
static HWND window;
static HDC context;
static HGLRC gl_context;
static HMODULE gl_library;
static int width = 640;
static int height = 480;
static bool running;
static bool mouse[CB_MOUSE_TOTAL];
static bool keyboard[CB_KEY_TOTAL];
static void (*onStartFunc)(void) = NULL;
static void (*onUpdateFunc)(float) = NULL;
static void (*onStopFunc)(void) = NULL;

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext, const int* attribList);
wglCreateContextAttribsARB_type* wglCreateContextAttribsARB;
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
wglChoosePixelFormatARB_type* wglChoosePixelFormatARB;
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB 0x202B

static void* gl_get_proc_address(const char* name)
{
	void* p = (void*)wglGetProcAddress(name);
	if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
	{
		p = (void*)GetProcAddress(gl_library, name);
	}
	return p;
}

static int convert_keyboard(WPARAM key, LPARAM flags)
{
	switch (key)
	{
	case VK_SHIFT:
	{
		UINT lshift = MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
		UINT scancode = (flags & (0xFF << 16)) >> 16;
		return scancode == lshift ? CB_KEY_LSHIFT : CB_KEY_RSHIFT;
	}

	case VK_MENU:
		return (HIWORD(flags) & KF_EXTENDED) ? CB_KEY_RALT : CB_KEY_LALT;

	case VK_CONTROL:
		return (HIWORD(flags) & KF_EXTENDED) ? CB_KEY_RCONTROL : CB_KEY_LCONTROL;

	case VK_LWIN:
		return CB_KEY_LSYSTEM;
	case VK_RWIN:
		return CB_KEY_RSYSTEM;
	case VK_APPS:
		return CB_KEY_MENU;
	case VK_OEM_1:
		return CB_KEY_SEMICOLON;
	case VK_OEM_2:
		return CB_KEY_SLASH;
	case VK_OEM_PLUS:
		return CB_KEY_EQUAL;
	case VK_OEM_MINUS:
		return CB_KEY_DASH;
	case VK_OEM_4:
		return CB_KEY_LBRACKET;
	case VK_OEM_6:
		return CB_KEY_RBRACKET;
	case VK_OEM_COMMA:
		return CB_KEY_COMMA;
	case VK_OEM_PERIOD:
		return CB_KEY_PERIOD;
	case VK_OEM_7:
		return CB_KEY_QUOTE;
	case VK_OEM_5:
		return CB_KEY_BACKSLASH;
	case VK_OEM_3:
		return CB_KEY_TILDE;
	case VK_ESCAPE:
		return CB_KEY_ESCAPE;
	case VK_SPACE:
		return CB_KEY_SPACE;
	case VK_RETURN:
		return CB_KEY_RETURN;
	case VK_BACK:
		return CB_KEY_BACKSPACE;
	case VK_TAB:
		return CB_KEY_TAB;
	case VK_PRIOR:
		return CB_KEY_PAGEUP;
	case VK_NEXT:
		return CB_KEY_PAGEDOWN;
	case VK_END:
		return CB_KEY_END;
	case VK_HOME:
		return CB_KEY_HOME;
	case VK_INSERT:
		return CB_KEY_INSERT;
	case VK_DELETE:
		return CB_KEY_DEL;
	case VK_ADD:
		return CB_KEY_ADD;
	case VK_SUBTRACT:
		return CB_KEY_SUBTRACT;
	case VK_MULTIPLY:
		return CB_KEY_MULTIPLY;
	case VK_DIVIDE:
		return CB_KEY_DIVIDE;
	case VK_PAUSE:
		return CB_KEY_PAUSE;
	case VK_F1:
		return CB_KEY_F1;
	case VK_F2:
		return CB_KEY_F2;
	case VK_F3:
		return CB_KEY_F3;
	case VK_F4:
		return CB_KEY_F4;
	case VK_F5:
		return CB_KEY_F5;
	case VK_F6:
		return CB_KEY_F6;
	case VK_F7:
		return CB_KEY_F7;
	case VK_F8:
		return CB_KEY_F8;
	case VK_F9:
		return CB_KEY_F9;
	case VK_F10:
		return CB_KEY_F10;
	case VK_F11:
		return CB_KEY_F11;
	case VK_F12:
		return CB_KEY_F12;
	case VK_F13:
		return CB_KEY_F13;
	case VK_F14:
		return CB_KEY_F14;
	case VK_F15:
		return CB_KEY_F15;
	case VK_LEFT:
		return CB_KEY_LEFT;
	case VK_RIGHT:
		return CB_KEY_RIGHT;
	case VK_UP:
		return CB_KEY_UP;
	case VK_DOWN:
		return CB_KEY_DOWN;
	case VK_NUMPAD0:
		return CB_KEY_NUMPAD0;
	case VK_NUMPAD1:
		return CB_KEY_NUMPAD1;
	case VK_NUMPAD2:
		return CB_KEY_NUMPAD2;
	case VK_NUMPAD3:
		return CB_KEY_NUMPAD3;
	case VK_NUMPAD4:
		return CB_KEY_NUMPAD4;
	case VK_NUMPAD5:
		return CB_KEY_NUMPAD5;
	case VK_NUMPAD6:
		return CB_KEY_NUMPAD6;
	case VK_NUMPAD7:
		return CB_KEY_NUMPAD7;
	case VK_NUMPAD8:
		return CB_KEY_NUMPAD8;
	case VK_NUMPAD9:
		return CB_KEY_NUMPAD9;
	case 'A':
		return CB_KEY_A;
	case 'Z':
		return CB_KEY_Z;
	case 'E':
		return CB_KEY_E;
	case 'R':
		return CB_KEY_R;
	case 'T':
		return CB_KEY_T;
	case 'Y':
		return CB_KEY_Y;
	case 'U':
		return CB_KEY_U;
	case 'I':
		return CB_KEY_I;
	case 'O':
		return CB_KEY_O;
	case 'P':
		return CB_KEY_P;
	case 'Q':
		return CB_KEY_Q;
	case 'S':
		return CB_KEY_S;
	case 'D':
		return CB_KEY_D;
	case 'F':
		return CB_KEY_F;
	case 'G':
		return CB_KEY_G;
	case 'H':
		return CB_KEY_H;
	case 'J':
		return CB_KEY_J;
	case 'K':
		return CB_KEY_K;
	case 'L':
		return CB_KEY_L;
	case 'M':
		return CB_KEY_M;
	case 'W':
		return CB_KEY_W;
	case 'X':
		return CB_KEY_X;
	case 'C':
		return CB_KEY_C;
	case 'V':
		return CB_KEY_V;
	case 'B':
		return CB_KEY_B;
	case 'N':
		return CB_KEY_N;
	case '0':
		return CB_KEY_NUM0;
	case '1':
		return CB_KEY_NUM1;
	case '2':
		return CB_KEY_NUM2;
	case '3':
		return CB_KEY_NUM3;
	case '4':
		return CB_KEY_NUM4;
	case '5':
		return CB_KEY_NUM5;
	case '6':
		return CB_KEY_NUM6;
	case '7':
		return CB_KEY_NUM7;
	case '8':
		return CB_KEY_NUM8;
	case '9':
		return CB_KEY_NUM9;
	}

	return CB_KEY_UNKNOWN;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_SIZE:
		width = LOWORD(lparam);
		height = HIWORD(lparam);
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		if (wparam & MK_LBUTTON)
		{
			mouse[CB_MOUSE_LEFT] = true;
		}
		if (wparam & MK_RBUTTON)
		{
			mouse[CB_MOUSE_RIGHT] = true;
		}
		if (wparam & MK_MBUTTON)
		{
			mouse[CB_MOUSE_MIDDLE] = true;
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if (wparam & MK_LBUTTON)
		{
			mouse[CB_MOUSE_LEFT] = false;
		}
		if (wparam & MK_RBUTTON)
		{
			mouse[CB_MOUSE_RIGHT] = false;
		}
		if (wparam & MK_MBUTTON)
		{
			mouse[CB_MOUSE_MIDDLE] = false;
		}
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		keyboard[convert_keyboard(wparam, lparam)] = !(bool)(lparam & (1 << 31));
		break;
	case WM_DESTROY:
		running = false;
		break;
	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
	return 0;
}

void cbInit(cbInitParams params) {
	if (params.width <= 0) {
		params.width = 800;
	}
	if (params.height <= 0) {
		params.height = 600;
	}
	if (params.title == NULL) {
		params.title = "CubeBox";
	}
	if (params.start) {
		onStartFunc = params.start;
	}
	if (params.stop) {
		onStopFunc = params.stop;
	}
	if (params.update) {
		onUpdateFunc = params.update;
	}

	WNDCLASSA window_class = {
		.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		.lpfnWndProc = DefWindowProcA,
		.hInstance = GetModuleHandle(0),
		.lpszClassName = "Dummy",
	};
	if (!RegisterClassA(&window_class))
	{
		printf("Failed to register dummy OpenGL window.");
	}
	HWND dummy_window = CreateWindowExA(0, window_class.lpszClassName, "Dummy OpenGL Window", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, window_class.hInstance, 0);
	if (!dummy_window)
	{
		printf("Failed to create dummy OpenGL window.");
	}
	HDC dummy_dc = GetDC(dummy_window);
	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 };
	int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
	if (!pixel_format)
	{
		printf("Failed to find a suitable pixel format.");
	}
	if (!SetPixelFormat(dummy_dc, pixel_format, &pfd))
	{
		printf("Failed to set pixel format.");
	}
	HGLRC dummy_context = wglCreateContext(dummy_dc);
	if (!dummy_context)
	{
		printf("Failed to create a dummy OpenGL rendering context.");
	}
	if (!wglMakeCurrent(dummy_dc, dummy_context))
	{
		printf("Failed to activate dummy OpenGL rendering context.");
	}
	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
		"wglCreateContextAttribsARB");
	wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
		"wglChoosePixelFormatARB");
	wglMakeCurrent(dummy_dc, 0);
	wglDeleteContext(dummy_context);
	ReleaseDC(dummy_window, dummy_dc);
	DestroyWindow(dummy_window);

	hinstance = GetModuleHandle(NULL);
	MSG msg = { 0 };
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hinstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = "cubebox_engine";
	wc.style = CS_OWNDC;
	if (!RegisterClass(&wc))
		return;
	window = CreateWindow(wc.lpszClassName, params.title, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, params.width, params.height, 0, 0, hinstance, NULL);
	context = GetDC(window);
	ShowWindow(window, SW_SHOW);

	int pixel_format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, 1,
		WGL_SUPPORT_OPENGL_ARB, 1,
		WGL_DOUBLE_BUFFER_ARB, 1,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		0
	};
	UINT num_formats;
	wglChoosePixelFormatARB(context, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
	if (!num_formats)
	{
		printf("Failed to set pixel format.");
	}
	DescribePixelFormat(context, pixel_format, sizeof(pfd), &pfd);
	if (!SetPixelFormat(context, pixel_format, &pfd))
	{
		printf("Failed to set pixel format.");
	}
	int gl_attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB,
		3,
		WGL_CONTEXT_MINOR_VERSION_ARB,
		3,
		WGL_CONTEXT_PROFILE_MASK_ARB,
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0,
	};
	gl_context = wglCreateContextAttribsARB(context, 0, gl_attribs);
	if (!gl_context)
	{
		printf("Failed to create OpenGL context.");
	}
	if (!wglMakeCurrent(context, gl_context))
	{
		printf("Failed to activate OpenGL rendering context.");
	}

	gl_library = LoadLibrary("opengl32.dll");
	gladLoadGL((GLADloadfunc)gl_get_proc_address);

	if (onStartFunc) {
		onStartFunc();
	}
}

void cbDestroy() {
	if (onStopFunc) {
		onStopFunc();
	}

	FreeLibrary(gl_library);
	wglMakeCurrent(context, 0);
	wglDeleteContext(gl_context);
	ReleaseDC(window, context);
	DestroyWindow(window);
}

static float get_time() {
	static int first = 1;
	static LARGE_INTEGER prev;
	static double factor;

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	if (first)
	{
		first = 0;
		prev = now;
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		factor = 1.0 / (double)freq.QuadPart;
		return 0;
	}

	float elapsed = (float)((double)(now.QuadPart - prev.QuadPart) * factor);
	prev = now;
	return elapsed;
}

void cbRun() {
	MSG event;

	running = true;
	while (running)
	{
		while (PeekMessage(&event, NULL, 0, 0, PM_REMOVE))
		{
			if (event.message == WM_QUIT)
				running = false;
			TranslateMessage(&event);
			DispatchMessage(&event);
		}

		if (onUpdateFunc) {
			onUpdateFunc(get_time());
		}

		SwapBuffers(context);
	}
}

void cbExit() {
	running = false;
}

bool cbIsKeyDown(cbKey k) {
	return keyboard[k];
}

bool cbIsMouseDown(cbMouse m) {
	return mouse[m];
}

void cbGetSize(int* w, int* h) {
	*w = width;
	*h = height;
}

void cbSetSize(int w, int h) {
	width = w;
	height = h;
	RECT rect = { 0, 0, width, height };
	SetWindowPos(window, HWND_TOP, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
}

void cbGetMousePos(int* x, int* y) {
	POINT pos;
	if (GetCursorPos(&pos)) {
		ScreenToClient(window, &pos);
		*x = pos.x;
		*y = pos.y;
	}
}

void cbSetMousePos(int x, int y) {
	POINT pos = { (int)x, (int)y };
	ClientToScreen(window, &pos);
	SetCursorPos(pos.x, pos.y);
}

#endif
