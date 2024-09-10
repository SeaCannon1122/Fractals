#include "platform.h"

#if defined(_WIN32)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

struct {
	HWND hwnd;
	bool active;
	int window_width;
	int window_height;
	struct {
		char* buffer;
		int buffer_size_width_termination;
		int size_used;
	} print_buffer[MAX_KEYBOARD_BUFFER_PARSERS_PER_WINDOW];
} window_resources[MAX_WINDOW_COUNT];

struct {
	int posx;
	int posy;
	int width;
	int height;
	unsigned char* name;
	bool done_flag;
	int window_resources_index;
} next_window;

WNDCLASSW wc;

bool keyStates[256] = { 0 };
int last_mouse_scroll = 0;

bool running = true;

void* window_control_thread;

void show_console_window() {
	HWND hwndConsole = GetConsoleWindow();
	if (hwndConsole != NULL) {
		ShowWindow(hwndConsole, SW_SHOW);
	}
}

void hide_console_window() {
	HWND hwndConsole = GetConsoleWindow();
	if (hwndConsole != NULL) {
		ShowWindow(hwndConsole, SW_HIDE);
	}
}

void set_console_cursor_position(int x, int y) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD) { (SHORT)x, (SHORT)y });
}

void sleep_for_ms(unsigned int _time_in_milliseconds) {
	Sleep(_time_in_milliseconds);
}

double get_time() {
	LARGE_INTEGER frequency, start;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);

	return (double)1000 * ((double)start.QuadPart / (double)frequency.QuadPart);
}

void* create_thread(void (address)(void*), void* args) {
	return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)address, args, 0, NULL);
}

void join_thread(void* thread_handle) {
	WaitForSingleObject(thread_handle, INFINITE);
	CloseHandle(thread_handle);
}

char get_key_state(int key) {

	char keyState = 0;

	SHORT currentKeyState = GetKeyState(key);

	if (currentKeyState & 0x8000) keyState |= 0b0001;
	if ((currentKeyState & 0x8000 ? 0x1 : 0x0) != keyStates[key]) keyState |= 0b0010;

	keyStates[key] = (currentKeyState & 0x8000 ? 0x1 : 0x0);

	return keyState;
}

int get_last_mouse_scroll() {
	int temp = last_mouse_scroll;
	last_mouse_scroll = 0;
	return temp;
}

void clear_mouse_scroll() {
	last_mouse_scroll = 0; 
}

//window functions

int create_window(int posx, int posy, int width, int height, unsigned char* name) {

	int next_free_window_index = 0;
	for (; next_free_window_index < MAX_WINDOW_COUNT; next_free_window_index++) if (window_resources[next_free_window_index].hwnd == NULL) break;
	if (next_free_window_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;


	next_window.posx = posx;
	next_window.posy = posy;
	next_window.width = width;
	next_window.height = height;
	next_window.name = name;
	next_window.done_flag = false;
	next_window.window_resources_index = next_free_window_index;

	while (next_window.done_flag == false) Sleep(1);

	return next_free_window_index;
}

int get_window_width(int window) {
	return window_resources[window].window_width; 
}

int get_window_height(int window) { 
	return window_resources[window].window_height; 
}

bool is_window_selected(int window) { 
	return window_resources[window].hwnd == GetForegroundWindow(); 
}

bool is_window_active(int window) { 
	return window_resources[window].active; 
}

void close_window(int window) {
	if (window_resources[window].active) SendMessage(window_resources[window].hwnd, WM_CLOSE, 0, 0);
	while (window_resources[window].active) Sleep(1);
	window_resources[window].hwnd = NULL;
}

void draw_to_window(int window, unsigned int* buffer, int width, int height, int scalar) {

	if (window_resources[window].active == false) return;

	HDC hdc = GetDC(window_resources[window].hwnd);

	BITMAPINFO bitmapInfo;

	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = -height;
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	SetStretchBltMode(hdc, COLORONCOLOR);
	StretchDIBits(hdc, 0, 0, width * scalar, height * scalar, 0, 0, width, height, buffer, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(window_resources[window].hwnd, hdc);

}

struct point2d_int get_mouse_cursor_position(int window) {
	if (window_resources[window].hwnd != GetForegroundWindow()) return (struct point2d_int) {-1, -1};

	POINT position;
	GetCursorPos(&position);
	RECT window_rect;
	GetWindowRect(window_resources[window].hwnd, &window_rect);

	struct point2d_int pos = { position.x - window_rect.left - 7, position.y - window_rect.top - 29 };
	return pos;
}

void set_cursor_rel_window(int window, int x, int y) {
	POINT position;
	GetCursorPos(&position);
	RECT window_rect;
	GetWindowRect(window_resources[window].hwnd, &window_rect);

	SetCursorPos(x + window_rect.left + 7, y + window_rect.top + 29);
}

int link_keyboard_parse_buffer(int window, char* buffer, int size, int used) {

	int next_parser_index = 0;
	for (; next_parser_index < MAX_KEYBOARD_BUFFER_PARSERS_PER_WINDOW; next_parser_index++) if (window_resources[window].print_buffer[next_parser_index].buffer == NULL) break;
	if (next_parser_index == MAX_KEYBOARD_BUFFER_PARSERS_PER_WINDOW) return KEYBOARD_BUFFER_PARSER_CREATION_FAILED;

	window_resources[window].print_buffer[next_parser_index].buffer = buffer;
	window_resources[window].print_buffer[next_parser_index].buffer_size_width_termination = size;
	window_resources[window].print_buffer[next_parser_index].size_used = used;

	return next_parser_index;
}

void unlink_keyboard_parse_buffer(int window, int link) {
	window_resources[window].print_buffer[link].buffer = NULL;
}

void WindowControl() {
	while (running) {

		for (int i = 0; i < MAX_WINDOW_COUNT; i++) {

			if (window_resources[i].active && window_resources[i].hwnd != NULL) {
				MSG message;
				while (PeekMessageW(&message, window_resources[i].hwnd, 0, 0, PM_REMOVE)) {
					TranslateMessage(&message);
					DispatchMessageW(&message);
				}
			}
		}

		//creating window

		if (next_window.done_flag == false) {
			int name_length = 1;

			for (; next_window.name[name_length - 1] != '\0'; name_length++);

			unsigned short* name_short = calloc(name_length, sizeof(unsigned short));
			if (name_short == NULL) name_short = L"Error";

			for (int i = 0; i < name_length; i++) *((char*)name_short + i * sizeof(unsigned short)) = next_window.name[i];

			HWND window = CreateWindowExW(
				0,
				wc.lpszClassName,
				name_short,
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				next_window.posx,
				next_window.posy,
				next_window.width,
				next_window.height,
				NULL,
				NULL,
				GetModuleHandleA(NULL),
				NULL
			);

			window_resources[next_window.window_resources_index].hwnd = window;
			for(int i = 0; i < MAX_KEYBOARD_BUFFER_PARSERS_PER_WINDOW; i++) window_resources[next_window.window_resources_index].print_buffer[i].buffer = NULL;
			window_resources[next_window.window_resources_index].active = true;

			SendMessage(window, WM_SIZE, 0, 0);
			next_window.done_flag = true;
		}

		Sleep(10);
	}

	return;
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	for (int i = 0; i < MAX_WINDOW_COUNT; i++) {

		if (window_resources[i].hwnd == hwnd && window_resources[i].hwnd != NULL) {

			switch (uMsg) {

			case WM_CLOSE: {
				DestroyWindow(hwnd);
			} break;


			case WM_DESTROY: {
				PostQuitMessage(0);
				window_resources[i].active = false;
			} break;

			case WM_SIZE: {
				RECT rect;
				GetClientRect(hwnd, &rect);
				window_resources[i].window_width = rect.right - rect.left;
				window_resources[i].window_height = rect.bottom - rect.top;
			} break;

			case WM_MOUSEWHEEL: {
				int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

				if (wheelDelta > 0) last_mouse_scroll++;
				else if (wheelDelta < 0) last_mouse_scroll--;
			} break;

			case WM_CHAR: {

				for (int i = 0; i < MAX_KEYBOARD_BUFFER_PARSERS_PER_WINDOW; i++) {
					if (window_resources[i].print_buffer[i].buffer != NULL) {
						char c = (char)wParam;
						if (c >= ' ' && c <= '~' && window_resources[i].print_buffer[i].buffer_size_width_termination - 1 > window_resources[i].print_buffer[i].size_used) {
							window_resources[i].print_buffer[i].buffer[window_resources[i].print_buffer[i].size_used] = c;
							window_resources[i].print_buffer[i].size_used++;
						}
						else if (c == 0x08 /*delete*/ && window_resources[i].print_buffer[i].size_used != 0) {
							window_resources[i].print_buffer[i].size_used--;
							window_resources[i].print_buffer[i].buffer[window_resources[i].print_buffer[i].size_used] = '\0';
						}
					}
				}

			} break;

			}

		}
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void platform_init() {

	AllocConsole();
	hide_console_window();

	FILE* fstdout;
	freopen_s(&fstdout, "CONOUT$", "w", stdout);
	FILE* fstderr;
	freopen_s(&fstderr, "CONOUT$", "w", stderr);
	FILE* fstdin;
	freopen_s(&fstdin, "CONIN$", "r", stdin);

	fflush(stdout);
	fflush(stderr);
	fflush(stdin);

	wc = (WNDCLASSW) {
		CS_HREDRAW | CS_VREDRAW | CS_CLASSDC,
		WinProc,
		0,
		0,
		GetModuleHandleA(NULL),
		NULL,
		LoadCursorW(NULL, IDC_ARROW),
		NULL,
		NULL,
		L"BasicWindowClass"
	};

	RegisterClassW(&wc);

	next_window.done_flag = true;

	window_control_thread = create_thread(WindowControl, NULL);

	return;
}

void platform_exit() {
	for (int i = 0; i < MAX_WINDOW_COUNT; i++) if (window_resources[i].hwnd != NULL) close_window(i);
	running = false;
	join_thread(window_control_thread);
}

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

struct {
	Window window;
	XImage* image;
	unsigned int* pixels;
	bool active;
	int window_width;
	int window_height;
	struct {
		char* buffer;
		int buffer_size_width_termination;
		int size_used;
	} print_buffer[MAX_KEYBOARD_BUFFER_PARSERS_PER_WINDOW];
} window_resources[MAX_WINDOW_COUNT];

char window_resources_active[MAX_WINDOW_COUNT] = { 0 };

int display_width;
int display_height;

Display* display;
int screen;
Atom wm_delete_window;

bool running = true;

bool keyStates[256 * 256] = { false };
int last_mouse_scroll = 0;

bool mouseButtons[3] = { false, false, false };

void* window_control_thread;

void show_console_window() { return; }

void hide_console_window() { return; }

void set_console_cursor_position(int x, int y) {
	printf("\033[%d;%dH", y, x);
}

void sleep_for_ms(unsigned int _time_in_milliseconds) {
	usleep(_time_in_milliseconds * 1000);
}

double get_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec * 1000 + (double)tv.tv_usec / 1000;
}

void* create_thread(void (address)(void*), void* args) {
	pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, (void* (*)(void*))address, args);
	return thread;
}

void join_thread(void* thread_handle) {
	pthread_join(*(pthread_t*)thread_handle, NULL);
	free(thread_handle);
}

char get_key_state(int key) {

	char key_state = 0;

	if (key == KEY_MOUSE_LEFT || key == KEY_MOUSE_MIDDLE || key == KEY_MOUSE_RIGHT) {

		Window root = DefaultRootWindow(display);
		Window root_return, child_return;
		int root_x, root_y, win_x, win_y;
		unsigned int mask_return;
		XQueryPointer(display, root, &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask_return);

		if (mask_return & (key == KEY_MOUSE_LEFT ? Button1Mask : (key == KEY_MOUSE_MIDDLE ? Button2Mask : Button3Mask))) key_state = 0b1;

		if (key_state != mouseButtons[(key == KEY_MOUSE_LEFT ? 0 : (key == KEY_MOUSE_MIDDLE ? 1 : 2))]) key_state |= 0b10;

		mouseButtons[(key == KEY_MOUSE_LEFT ? 0 : (key == KEY_MOUSE_MIDDLE ? 1 : 2))] = key_state & 0b1;

		return key_state;
	}

	char keys[32];
	XQueryKeymap(display, keys);

	KeySym keysym = (KeySym)key;
	KeyCode keycode = XKeysymToKeycode(display, keysym);

	int byteIndex = keycode / 8;
	int bitIndex = keycode % 8;

	if (keys[byteIndex] & (1 << bitIndex)) key_state = 0b1;
	if (key_state != keyStates[key]) key_state |= 0b10;
	keyStates[key] = key_state & 0b1;

	return key_state;
}

int get_last_mouse_scroll() {
	int temp = last_mouse_scroll;
	last_mouse_scroll = 0;
	return temp;
}

void clear_mouse_scroll() { last_mouse_scroll = 0; }

int create_window(int posx, int posy, int width, int height, unsigned char* name) {

	int next_free_window_index = 0;
	for (; next_free_window_index < MAX_WINDOW_COUNT; next_free_window_index++) if (!window_resources_active[next_free_window_index]) break;
	if (next_free_window_index == MAX_WINDOW_COUNT) return WINDOW_CREATION_FAILED;

	window_resources_active[next_free_window_index] = 1;

	Window window = XCreateSimpleWindow(display, RootWindow(display, screen), posx, posy, width, height, 1, BlackPixel(display, screen), WhitePixel(display, screen));

	unsigned int* pixels = malloc(display_width * display_height * sizeof(unsigned int));

	XImage* image = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen), ZPixmap, 0, (char*)pixels, display_width, display_height, 32, 0);

	window_resources[next_free_window_index].window = window;
	window_resources[next_free_window_index].image = image;
	window_resources[next_free_window_index].pixels = pixels;
	window_resources[next_free_window_index].active = true;
	window_resources[next_free_window_index].window_width = width;
	window_resources[next_free_window_index].window_height = height;

	XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask);
	XStoreName(display, window, name);
	XSetWMProtocols(display, window, &wm_delete_window, 1);
	XMapWindow(display, window);

	return next_free_window_index;
}

int get_window_width(int window) {
	return window_resources[window].window_width;
}

int get_window_height(int window) {
	return window_resources[window].window_height;
}

bool is_window_selected(int window) {
	Window focused_window;
	int revert_to;

	XGetInputFocus(display, &focused_window, &revert_to);

	return focused_window == window_resources[window].window;
}

bool is_window_active(int window) {
	return window_resources[window].active;
}

void close_window(int window) {
	if (window_resources[window].active) XDestroyWindow(display, window_resources[window].window);
	window_resources_active[window] = 0;

	XDestroyImage(window_resources[window].image);
}

void draw_to_window(int window, unsigned int* buffer, int width, int height, int scalar) {
	if (!window_resources[window].active) return;
	for (int i = 0; i < width * scalar && i < display_width; i++) {
		for (int j = 0; j < height * scalar && j < display_height; j++) {
			window_resources[window].pixels[i + display_width * j] = buffer[i / scalar + width * (j / scalar)];
		}
	}

	XPutImage(display, window_resources[window].window, DefaultGC(display, screen), window_resources[window].image, 0, 0, 0, 0, width * scalar, height * scalar);
}

struct point2d_int get_mouse_cursor_position(int window) {
	if (!window_resources[window].active) return (struct point2d_int) { -1, -1 };
	Window root, child;
	int root_x, root_y;
	int win_x, win_y;
	unsigned int mask;

	XQueryPointer(display, window_resources[window].window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);

	struct point2d_int pos = { win_x + 1, win_y + 1 };

	return pos;
}

void set_cursor_rel_window(int window, int x, int y) {
	if (!window_resources[window].active) return;
	Window root, child;
	int root_x, root_y;
	int win_x, win_y;
	unsigned int mask;

	XQueryPointer(display, window_resources[window].window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
	XWarpPointer(display, None, DefaultRootWindow(display), 0, 0, 0, 0, root_x - win_x + x + 2, root_y - win_y + window_resources[window].window_height - y + 1);
	XFlush(display);
	return;
}

int link_keyboard_parse_buffer(int window, char* buffer, int size, int used) {

	int next_parser_index = 0;
	for (; next_parser_index < MAX_KEYBOARD_BUFFER_PARSERS_PER_WINDOW; next_parser_index++) if (window_resources[window].print_buffer[next_parser_index].buffer == NULL) break;
	if (next_parser_index == MAX_KEYBOARD_BUFFER_PARSERS_PER_WINDOW) return KEYBOARD_BUFFER_PARSER_CREATION_FAILED;

	window_resources[window].print_buffer[next_parser_index].buffer = buffer;
	window_resources[window].print_buffer[next_parser_index].buffer_size_width_termination = size;
	window_resources[window].print_buffer[next_parser_index].size_used = used;

	return next_parser_index;
}

void unlink_keyboard_parse_buffer(int window, int link) {
	window_resources[window].print_buffer[link].buffer = NULL;
}

void WindowControl() {
	XEvent event;
	while (running) {

		while (XPending(display) && running) {

			XNextEvent(display, &event);

			int index = 0;

			for (; index < MAX_WINDOW_COUNT && window_resources[index].window != event.xany.window; index++);

			if (window_resources[index].active) {

				switch (event.type) {

				case ConfigureNotify: {
					window_resources[index].window_width = (event.xconfigure.width > display_width ? display_width : event.xconfigure.width);
					window_resources[index].window_height = (event.xconfigure.height > display_height ? display_height : event.xconfigure.height);
				} break;

				case ClientMessage: {
					if ((Atom)event.xclient.data.l[0] == wm_delete_window) {
						window_resources[index].active = false;
						XDestroyWindow(display, window_resources[index].window);
					}
				} break;

				case ButtonPress: {
					if (event.xbutton.button == Button4) last_mouse_scroll++;
					else if (event.xbutton.button == Button5) last_mouse_scroll--;
				} break;

				case KeyPress: {
					KeySym keysym;
					char buf[32];
					int len = XLookupString(&event.xkey, buf, sizeof(buf), &keysym, NULL);
					if (len > 0) {
						for (int i = 0; i < len; ++i) {
							unsigned char c = (unsigned char)buf[i];
							if (c >= ' ' && c <= '~' && window_resources[i].print_buffer[i].buffer_size_width_termination - 1 > window_resources[i].print_buffer[i].size_used) {
								window_resources[i].print_buffer[i].buffer[window_resources[i].print_buffer[i].size_used] = c;
								window_resources[i].print_buffer[i].size_used++;
							}
							else if (c == 0x08 /*delete*/ && window_resources[i].print_buffer[i].size_used != 0) {
								window_resources[i].print_buffer[i].size_used--;
								window_resources[i].print_buffer[i].buffer[window_resources[i].print_buffer[i].size_used] = '\0';
							}
						}
					}

				} break;

				}

			}

		}

		sleep_for_ms(10);
	}

	return;
}

void platform_init() {
	XInitThreads();

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		fprintf(stderr, "Unable to open X display\n");
		return;
	}

	screen = DefaultScreen(display);

	display_width = DisplayWidth(display, screen);
	display_height = DisplayHeight(display, screen);

	wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);

	window_control_thread = create_thread(WindowControl, NULL);

	return;
}

void platform_exit() {
	running = false;
	join_thread(window_control_thread);
	XCloseDisplay(display);
}

#endif 