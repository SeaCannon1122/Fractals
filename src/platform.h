#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>

struct window_state {
	void* window_handle;
	int window_width;
	int window_height;
};

struct point2d_int {
	int x;
	int y;
};

//general

void show_console_window();

void hide_console_window();

void set_console_cursor_position(int x, int y);

void sleep_for_ms(unsigned int time_in_milliseconds);

double get_time();

void* create_thread(void* address, void* args);

void join_thread(void* thread_handle);

char get_key_state(int key);

int get_last_mouse_scroll();

void clear_mouse_scroll();

//Window functions

struct window_state* create_window(int posx, int posy, int width, int height, unsigned char* name);

bool is_window_selected(struct window_state* state);

bool is_window_active(struct window_state* state);

void close_window(struct window_state* state);

void draw_to_window(struct window_state* ws, unsigned int* buffer, int width, int height);

struct point2d_int get_mouse_cursor_position(struct window_state* state);

void set_cursor_rel_window(struct window_state* state, int x, int y);

//Entry Method

void Entry();

//Keysymbol Mapping

#if defined(_WIN64)

#include <windows.h>

#define KEY_SPACE VK_SPACE
#define KEY_SHIFT_L VK_SHIFT
#define KEY_SHIFT_R VK_RSHIFT
#define KEY_CONTROL_L VK_CONTROL
#define KEY_CONTROL_R VK_RCONTROL
#define KEY_ALT_L VK_LMENU
#define KEY_ALT_R VK_RMENU
#define KEY_ESCAPE VK_ESCAPE
#define KEY_BACKSPACE VK_BACK
#define KEY_TAB VK_TAB
#define KEY_ENTER VK_RETURN
#define KEY_CAPS_LOCK VK_CAPITAL
#define KEY_MINUS VK_OEM_MINUS
#define KEY_PLUS VK_OEM_PLUS
#define KEY_ARROW_LEFT VK_LEFT
#define KEY_ARROW_RIGHT VK_RIGHT
#define KEY_ARROW_UP VK_UP
#define KEY_ARROW_DOWN VK_DOWN
#define KEY_MOUSE_LEFT VK_LBUTTON
#define KEY_MOUSE_MIDDLE VK_MBUTTON
#define KEY_MOUSE_RIGHT VK_RBUTTON

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#define KEY_SPACE XK_space
#define KEY_SHIFT_L XK_Shift_L
#define KEY_SHIFT_R XK_Shift_R
#define KEY_CONTROL_L XK_Control_L
#define KEY_CONTROL_R XK_Control_R
#define KEY_ESCAPE XK_Escape
#define KEY_BACKSPACE XK_BackSpace
#define KEY_ALT_L XK_Alt_L
#define KEY_ALT_R XK_Alt_R
#define KEY_TAB XK_Tab
#define KEY_ENTER XK_Return
#define KEY_CAPS_LOCK XK_Caps_Lock
#define KEY_MINUS XK_minus
#define KEY_PLUS XK_plus
#define KEY_ARROW_LEFT XK_Left
#define KEY_ARROW_RIGHT XK_Right
#define KEY_ARROW_UP XK_Up
#define KEY_ARROW_DOWN XK_Down
#define KEY_MOUSE_LEFT 0x1234
#define KEY_MOUSE_MIDDLE 0x1235
#define KEY_MOUSE_RIGHT 0x1236

#endif

#endif // PLATFORM_H
