#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>
#include <crtdbg.h>
#include <shellapi.h>

/* constants */
static const int hotkeys[] = { 'A', 'S', 'D', 'F', 'J', 'K', 'L', VK_OEM_1 };
static const char hotkeys_chars[] = "ASDFJKL#";
#define TITLE_SIZE 1024
#define HOTK_JUMP_BASE 1000
#define HOTK_ASSIGN_BASE 2000
#define BAR_HEIGHT  40
#define WM_APPBAR   (WM_APP + 1)
#define arr_len(arr) (sizeof(arr)/sizeof(arr[0]))

static APPBARDATA abd;

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
static void init_layout(HWND hwnd);
static int is_i3_hotkey(int mod, int key, int *ws);
static void handle_hotkey(LPARAM lParam, int ws);
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void set_bar_pos(HWND hwnd);
static void set_active_ws(int ws);

/*
 * things to hook into:
 *
 * - keyboard events
 * - timer to retile at intervals?
 * - new windows?
 */

/* todo: make this modular in a config file. */
#define SUPER_KEY MOD_ALT

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int show)
{
    HWND hwnd;
    WNDCLASS wc = {0};
	MSG msg = { 0 };


    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(32, 32, 32));
    wc.lpszClassName = TEXT("TopBar");
    RegisterClass(&wc);

    hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
                               wc.lpszClassName, TEXT(""), WS_POPUP,
                               0, 0, 0, 0, NULL, NULL, hInst, NULL);
	init_layout(hwnd);
    ShowWindow(hwnd, show);

    EnumWindows(win_callbk_vb, 0);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int ws;
    RECT rect = { 0 };
    switch (msg) {
    case WM_HOTKEY:
        if (is_i3_hotkey(LOWORD(lParam), HIWORD(lParam), &ws)) {
            SystemParametersInfoA(SPI_GETWORKAREA, 0, &rect, 0);
            monitor_sz_x = rect.right - rect.left;
            monitor_sz_y = rect.bottom - rect.top;

            handle_hotkey(lParam, ws);
            set_active_ws(ws);
        }

    case WM_CREATE:
        abd.cbSize           = sizeof(abd);
        abd.hWnd             = hwnd;
        abd.uCallbackMessage = WM_APPBAR;
        SHAppBarMessage(ABM_NEW, &abd); /* register as an appbar */
        set_bar_pos(hwnd);
        return 0;

    case WM_APPBAR:
        if (wParam == ABN_POSCHANGED) set_bar_pos(hwnd);
        return 0;

    case WM_ACTIVATE:
        SHAppBarMessage(ABM_ACTIVATE, &abd);
        break;

    case WM_WINDOWPOSCHANGED:
        SHAppBarMessage(ABM_WINDOWPOSCHANGED, &abd);
        break;

    case WM_DISPLAYCHANGE:
        set_bar_pos(hwnd);
        return 0;

    case WM_PAINT: {
        const int box = 28, gap = 6, y = (BAR_HEIGHT - box) / 2;
        PAINTSTRUCT ps;
        HDC hdc;
        HPEN pen;
        HGDIOBJ oldPen, oldBrush, oldFont;
        int i;

        hdc = BeginPaint(hwnd, &ps);
        pen      = CreatePen(PS_SOLID, 1, RGB(160, 160, 160));
        oldPen   = SelectObject(hdc, pen);
        oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        oldFont  = SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));

        for (i = 0; i < arr_len(hotkeys_chars) - 1; i++) {
            RECT r;
            r.left   = gap + i * (box + gap);
            r.top    = y;
            r.right  = r.left + box;
            r.bottom = r.top + box;
            if (i == active_wsps) { /* highlighted box */
                HBRUSH hl = CreateSolidBrush(RGB(200, 200, 200));
                FillRect(hdc, &r, hl);
                DeleteObject(hl);
                SetTextColor(hdc, RGB(0, 0, 0));
            } else {
                SetTextColor(hdc, RGB(255, 255, 255));
            }
            Rectangle(hdc, r.left, r.top, r.right, r.bottom);
            DrawText(hdc, &(hotkeys_chars[i]), 1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        SelectObject(hdc, oldFont);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_RBUTTONUP:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        SHAppBarMessage(ABM_REMOVE, &abd); /* give the space back */
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

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

static void init_layout(HWND hwnd)
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
		if (!RegisterHotKey(hwnd, HOTK_JUMP_BASE + hotkeys[i], SUPER_KEY, hotkeys[i])) 
			goto init_error;

		if (!RegisterHotKey(hwnd, HOTK_ASSIGN_BASE + hotkeys[i], SUPER_KEY | MOD_SHIFT, hotkeys[i])) 
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
    int x,y,cx,cy;

    x = 10;
    y = 10 + BAR_HEIGHT;
    cx = (monitor_sz_x - 20) / num_hwnd;
    cy = monitor_sz_y - 20;
    printf("redraw_wsps\n");

    SetWindowPos(g_layout.wsps[wsps_id].hwnd,
                 HWND_TOP,
                 x,
                 y,
                 cx,
                 cy,
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

static void handle_hotkey(LPARAM lParam, int ws)
{
        if (LOWORD(lParam) & MOD_SHIFT) {
            printf("move hwnd to workspace '%c'\n", HIWORD(lParam));
            assign_to_ws(ws);
        } else {
            printf("     jump to workspace '%c'\n", HIWORD(lParam));
            jump_to_ws(ws);
        }
}

static void set_active_ws(int idx)
{
    if (idx < 0 || idx >= arr_len(hotkeys))
        return;
    active_wsps = idx;
    if (abd.hWnd)
        InvalidateRect(abd.hWnd, NULL, TRUE);
}

static void set_bar_pos(HWND hwnd)
{
    abd.uEdge      = ABE_TOP;
    abd.rc.left    = 0;
    abd.rc.top     = 0;
    abd.rc.right   = GetSystemMetrics(SM_CXSCREEN);
    abd.rc.bottom  = BAR_HEIGHT;

    SHAppBarMessage(ABM_QUERYPOS, &abd); /* let the shell adjust the rect */
    abd.rc.bottom = abd.rc.top + BAR_HEIGHT;
    SHAppBarMessage(ABM_SETPOS, &abd); /* reserve the screen space */

    MoveWindow(hwnd, abd.rc.left, abd.rc.top,
               abd.rc.right - abd.rc.left, BAR_HEIGHT, TRUE);
}
