#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>
#include <crtdbg.h>

/* constants */
static const int hotkeys[] = { 'A', 'S', 'D', 'F', 'J', 'K', 'L', VK_OEM_1 };
#define TITLE_SIZE 1024
#define HOTK_JUMP_BASE 1000
#define HOTK_ASSIGN_BASE 2000

/* types */
struct i3_workspace {
	char title[16];
	HWND hwnd;
};

struct i3_layout {
	struct i3_workspace *wsps;
	size_t wsps_count;
};

/* globals */
static int monitor_sz_x;
static int monitor_sz_y;
static int active_wsps;
static HWND active_hwnd;
struct i3_layout g_layout;

/* helper functions */
static void redraw_wsps(int wsps_id, int num_hwnd);
static void refresh_ws_state(int ws);
static void set_focus_window(int ws, HWND hwnd);
static void jump_to_ws(int ws);
static void assign_to_ws(int ws);
static int is_alt_tab_win(HWND hwnd);
static BOOL CALLBACK win_callbk_vb(HWND hwnd, LPARAM lParam);
static void init_layout(void);
static int is_i3_hotkey(int mod, int key, int *ws);
static void handle_hotkey(MSG msg, int ws);

/*
 * things to hook into:
 *
 * - keyboard events
 * - timer to retile at intervals?
 * - new windows?
 */

/* todo: make this modular in a config file. */
#define SUPER_KEY MOD_ALT

int main(void)
{
    RECT rect = { 0 };
	MSG msg = { 0 };

    int ws;

	init_layout();

    EnumWindows(win_callbk_vb, 0);

	/*
	 * initial layout defines the hotkeys, but not the windows assigned.
	 *
	 * alt + shitf + hotkey assigns a window.
	 *
	 * alt + hotkey jumps to a window.
	 */
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		if (msg.message == WM_HOTKEY) {

            if (is_i3_hotkey(LOWORD(msg.lParam), HIWORD(msg.lParam), &ws)) {
                SystemParametersInfoA(SPI_GETWORKAREA, 0, &rect, 0);
                monitor_sz_x = rect.right - rect.left;
                monitor_sz_y = rect.bottom - rect.top;

                handle_hotkey(msg, ws);
            }
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

#define arr_len(arr) (sizeof(arr)/sizeof(arr[0]))
static int is_i3_hotkey(int mod, int key, int *ws)
{
    int i;
    int is_key;

    if (!(mod & SUPER_KEY))
        return 0;

    is_key = 0;
    for (i = 0; i < arr_len(hotkeys); i++)
    {
        printf("comparing: %c -- %c\n", key, hotkeys[i]);
        if (key == hotkeys[i]) {
            printf("is hotkey: %c\n", key);
            is_key = 1;
            *ws = i;
            break;
        }
    }
    return is_key;
}

static void init_layout(void)
{
	/* 
	 * todo: take a config file to determine workspaces instead of
	 * 	 defaults. 
	 */
	int i;
	size_t ws_size = sizeof(*(g_layout.wsps));
	g_layout.wsps_count = arr_len(hotkeys);
	g_layout.wsps = malloc(ws_size * g_layout.wsps_count);

	if (!g_layout.wsps)
		goto init_error;

	for (i = 0; i < g_layout.wsps_count; i++) {
		if (!RegisterHotKey(NULL, HOTK_JUMP_BASE + hotkeys[i], SUPER_KEY, hotkeys[i])) 
			goto init_error;

		if (!RegisterHotKey(NULL, HOTK_ASSIGN_BASE + hotkeys[i], SUPER_KEY | MOD_SHIFT, hotkeys[i])) 
			goto init_error;

		g_layout.wsps[i].hwnd = NULL;
	}

	return;

init_error:
	fprintf(stderr, "Error, initialization failed\n");
	exit(-1);
}

static void redraw_wsps(int wsps_id, int num_hwnd)
{
    int hwnd_x = (monitor_sz_x - 20) / num_hwnd;
    int hwnd_y = (monitor_sz_y - 20);
    printf("redraw_wsps\n");

    SetWindowPos(g_layout.wsps[wsps_id].hwnd,
                 HWND_TOP,
                 10,
                 10,
                 hwnd_x,
                 hwnd_y,
                 SWP_NOZORDER | SWP_SHOWWINDOW);
    SetForegroundWindow(g_layout.wsps[wsps_id].hwnd);
    UpdateWindow(g_layout.wsps[wsps_id].hwnd);
}

/* assess if current list of hwnd is still valid */
static void refresh_ws_state(int ws)
{
    int has_hwnd= 0;
    printf("refresh_ws_state\n");

    if (g_layout.wsps[ws].hwnd)
        has_hwnd = IsWindow(g_layout.wsps[ws].hwnd);

    /* if hwnd is set and IsWindow returned false, clear the hwnd value */
    if (g_layout.wsps[ws].hwnd && !has_hwnd)
        g_layout.wsps[ws].hwnd = NULL;

}

static void set_focus_window(int ws, HWND hwnd)
{
    if (IsIconic(hwnd))
        printf("workspace %c hwnd is minimized\n", hotkeys[ws]);
    active_wsps = ws;
    active_hwnd = hwnd;
    ShowWindow(hwnd, SW_NORMAL);
    SetForegroundWindow(hwnd);
    UpdateWindow(hwnd);
}

static void jump_to_ws(int ws)
{
    /* TODO: show all hwnd in ws before focusing */
    int num_hwnd;
    printf("jump_to_ws\n");
    refresh_ws_state(ws);

    if (!g_layout.wsps[ws].hwnd) {
        printf("%c not assigned\n", hotkeys[ws]);
        return;
    }

    printf("jump %c\n", hotkeys[ws]);
    set_focus_window(ws, g_layout.wsps[ws].hwnd);
    num_hwnd = 1;
    redraw_wsps(ws, num_hwnd);
}


static void assign_to_ws(int ws)
{
    /* TODO: if hwnd has previous assignment drop it */
    HWND hwnd;
	TCHAR window_title[TITLE_SIZE];
    int num_hwnd;

    refresh_ws_state(ws);

    hwnd = GetForegroundWindow();
    printf("assign %c\n", hotkeys[ws]);

    if (NULL == hwnd) {
        printf("hwnd is null\n");
    } else {
        GetWindowText(hwnd, window_title, TITLE_SIZE);
        printf("hwnd: %s\n", window_title);

        num_hwnd = 1;
        g_layout.wsps[ws].hwnd = hwnd;
        redraw_wsps(ws, num_hwnd);
    }
}


static int is_alt_tab_win(HWND hwnd)
{

    /* Start at the root owner */
    HWND hwnd_walk = GetAncestor(hwnd, GA_ROOTOWNER);

    /* See if we are the last active visible popup */
    HWND hwnd_try;

    /* Used later... */
    TITLEBARINFO ti;

    hwnd_try = GetLastActivePopup(hwnd_walk);
    do {
        if (IsWindowVisible(hwnd_try))
            break;

        hwnd_walk = hwnd_try;
        hwnd_try = GetLastActivePopup(hwnd_walk);
    } while (hwnd_walk != hwnd_try);

    ti.cbSize = sizeof(ti);
    GetTitleBarInfo(hwnd, &ti);

    if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE) return 0;

    if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) return 0;

    return hwnd_walk == hwnd;
}

static BOOL CALLBACK win_callbk_vb(HWND hwnd, LPARAM lParam)
{
    TCHAR window_title[TITLE_SIZE];
    GetWindowText(hwnd, window_title, TITLE_SIZE);

    if (is_alt_tab_win(hwnd) && IsWindowVisible(hwnd) &&
        GetWindowTextLength(hwnd) > 0) {

        printf("%s\n", window_title);
    }

    return TRUE;
}

static void handle_hotkey(MSG msg, int ws)
{
        if (LOWORD(msg.lParam) & MOD_SHIFT) {
            printf("move hwnd to workspace '%c'\n", HIWORD(msg.lParam));
            assign_to_ws(ws);
        } else {
            printf("     jump to workspace '%c'\n", HIWORD(msg.lParam));
            jump_to_ws(ws);
        }
}
