#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>
#include <crtdbg.h>

#include "layout.h"

#define TITLE_SIZE 1024
#define HOTK_JUMP_BASE 1010
#define HOTK_ASSIGN_BASE 1018

int monitor_sz_x;
int monitor_sz_y;

int active_wsps;
HWND active_hwnd;

void redraw_wsps(int wsps_id, int num_hwnd)
{
    int hwnd_x = (monitor_sz_x - 20) / num_hwnd;
    int hwnd_y = (monitor_sz_y - 20);
    printf("redraw_wsps\n");

    SetWindowPos(g_layout.wsps[wsps_id].hwnd_l,
                 HWND_TOP,
                 10,
                 10,
                 hwnd_x,
                 hwnd_y,
                 SWP_NOZORDER | SWP_SHOWWINDOW);
    SetForegroundWindow(g_layout.wsps[wsps_id].hwnd_l);
    UpdateWindow(g_layout.wsps[wsps_id].hwnd_l);
}

/* assess if current list of hwnd is still valid */
void refresh_ws_state(int ws)
{
    int has_l= 0;
    printf("refresh_ws_state\n");

    if (g_layout.wsps[ws].hwnd_l)
        has_l = IsWindow(g_layout.wsps[ws].hwnd_l);

    /* if hwnd is set and IsWindow returned false, clear the hwnd value */
    if (g_layout.wsps[ws].hwnd_l && !has_l)
        g_layout.wsps[ws].hwnd_l = NULL;

}

void set_focus_window(int wsps, HWND hwnd)
{
    active_wsps = wsps;
    active_hwnd = hwnd;
    SetForegroundWindow(hwnd);
    UpdateWindow(hwnd);
}

void jump_to_ws(int ws)
{
    /* TODO: show all hwnd in ws before focusing */
    int num_hwnd;
    printf("jump_to_ws\n");
    refresh_ws_state(ws);

    if (!g_layout.wsps[ws].hwnd_l) {
        printf("%c not assigned\n", g_layout.wsps[ws].hotk_chr);
        return;
    }

    printf("jump %c\n", g_layout.wsps[ws].hotk_chr);
    set_focus_window(ws, g_layout.wsps[ws].hwnd_l);
    num_hwnd = 1;
    redraw_wsps(ws, num_hwnd);
}


void assign_to_ws(int ws)
{
    /* TODO: if hwnd has previous assignment drop it */
    HWND hwnd;
	TCHAR window_title[TITLE_SIZE];
    int num_hwnd;

    refresh_ws_state(ws);

    hwnd = GetForegroundWindow();
    printf("assign %c\n", g_layout.wsps[ws].hotk_chr);

    if (NULL == hwnd) {
        printf("hwnd is null\n");
    } else {
        GetWindowText(hwnd, window_title, TITLE_SIZE);
        printf("hwnd: %s\n", window_title);

        num_hwnd = 1;
        g_layout.wsps[ws].hwnd_l = hwnd;
        redraw_wsps(ws, num_hwnd);
    }
}


int is_alt_tab_win(HWND hwnd)
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

BOOL CALLBACK win_callbk_vb(HWND hwnd, LPARAM lParam)
{
    TCHAR window_title[TITLE_SIZE];
    GetWindowText(hwnd, window_title, TITLE_SIZE);

    if (is_alt_tab_win(hwnd) && IsWindowVisible(hwnd) &&
        GetWindowTextLength(hwnd) > 0) {

        printf("%s\n", window_title);
    }

    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow)
{
    RECT rect = { 0 };
	MSG msg = { 0 };

	init_layout();

    SystemParametersInfoA(SPI_GETWORKAREA, 0, &rect, 0);
    printf("cx: %ld, cy: %ld\n", rect.right - rect.left, rect.bottom - rect.top);

    monitor_sz_x = rect.right - rect.left;
    monitor_sz_y = rect.bottom - rect.top;

	/*
	 * initial layout defines the hotkeys, but not the windows assigned.
	 *
	 * alt + shitf + hotkey assigns a window.
	 *
	 * alt + hotkey jumps to a window.
	 */
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		if (msg.message == WM_HOTKEY) {

			printf("hotk code: %d\n", (int) msg.wParam);

            if (msg.wParam >= HOTK_ASSIGN_BASE)
                assign_to_ws(msg.wParam - HOTK_ASSIGN_BASE);
            else
                jump_to_ws(msg.wParam - HOTK_JUMP_BASE);



            /* TODO: alt + J/K don't work when chrome is focused */
            /*
			for (i = 0; i < g_layout.wsps_count; i++) {
				if (msg.wParam == g_layout.wsps[i].hotk_code) {
                    jump_to_ws(i);
					break;
				} else if (msg.wParam == g_layout.wsps[i].assign_code) {
                    assign_to_ws(i);
					break;
				}
			}
            */
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
