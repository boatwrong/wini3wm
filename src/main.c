#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>
#include <crtdbg.h>

#include "layout.h"

#define TITLE_SIZE 1024

int monitor_sz_x;
int monitor_sz_y;

int active_wsps;
HWND active_hwnd;

void redraw_wsps(int wsps_id, int num_hwnd)
{
    int hwnd_x = (monitor_sz_x - 20) / num_hwnd;
    int hwnd_y = (monitor_sz_y - 20); /* / num_hwnd; */
    printf("redraw_wsps\n");

    if (num_hwnd == 2) {
        SetWindowPos(g_layout.wsps[wsps_id].hwnd_r,
                     HWND_TOP,
                     10 + hwnd_x + 5,
                     10,
                     hwnd_x,
                     hwnd_y,
                     SWP_NOZORDER | SWP_SHOWWINDOW);
    UpdateWindow(g_layout.wsps[wsps_id].hwnd_r);
    }

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
    int has_r= 0;
    /* 
    int num_hwnd = 0;
    */

    printf("refresh_ws_state\n");

    if (g_layout.wsps[ws].hwnd_l)
        has_l = IsWindow(g_layout.wsps[ws].hwnd_l);

    if (g_layout.wsps[ws].hwnd_r)
        has_r = IsWindow(g_layout.wsps[ws].hwnd_r);

    /* if hwnd is set and IsWindow returned false, clear the hwnd value */
    if (g_layout.wsps[ws].hwnd_l && !has_l)
        g_layout.wsps[ws].hwnd_l = NULL;

    if (g_layout.wsps[ws].hwnd_r && !has_r)
        g_layout.wsps[ws].hwnd_r = NULL;

    /* shift the right hwnd over if left hwnd is closed */
    if (g_layout.wsps[ws].hwnd_r && !g_layout.wsps[ws].hwnd_l) {
        g_layout.wsps[ws].hwnd_l = g_layout.wsps[ws].hwnd_r;
        g_layout.wsps[ws].hwnd_r = NULL;
    }

}

void set_focus_window(int wsps, HWND hwnd)
{
    active_wsps = wsps;
    active_hwnd = hwnd;
    /*
    SetWindowPos(hwnd,
                 HWND_TOP,
                 10,
                 10,
                 monitor_sz_x - 20,
                 monitor_sz_y - 20,
                 SWP_NOZORDER | SWP_SHOWWINDOW);
    */
    SetForegroundWindow(hwnd);
    UpdateWindow(hwnd);
}

void jump_to_ws(int ws)
{
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
    if (g_layout.wsps[ws].hwnd_r)
        num_hwnd++;

    redraw_wsps(ws, num_hwnd);
}


void assign_to_ws(int ws)
{
    HWND hwnd;
	TCHAR window_title[TITLE_SIZE];
    int num_hwnd;

    refresh_ws_state(ws);

    hwnd = GetForegroundWindow();
    printf("assign %c\n", g_layout.wsps[ws].hotk_chr);

    /* todo: assign window logic not working as expected */
    if (NULL == hwnd) {
        printf("hwnd is null\n");
    } else {
        GetWindowText(hwnd, window_title, TITLE_SIZE);
        printf("hwnd: %s\n", window_title);

        num_hwnd = 1;

        if (g_layout.wsps[ws].hwnd_l) {
            g_layout.wsps[ws].hwnd_r = hwnd;
            num_hwnd++;
        } else {
            g_layout.wsps[ws].hwnd_l = hwnd;
        }

        redraw_wsps(ws, num_hwnd);
    }

    /*
    if (g_layout.wsps[ws].hwnd_l) num_hwnd++;
    if (g_layout.wsps[ws].hwnd_r) num_hwnd++;

    redraw_wsps(ws, num_hwnd);
    */
}

void try_swap_focus(void)
{
    /* check for multiple windows in workspace */
    if (g_layout.wsps[active_wsps].hwnd_l == active_hwnd) {

        if (g_layout.wsps[active_wsps].hwnd_r) {
            set_focus_window(active_wsps, g_layout.wsps[active_wsps].hwnd_r);
        }

    } else if (g_layout.wsps[active_wsps].hwnd_r == active_hwnd) {
        set_focus_window(active_wsps, g_layout.wsps[active_wsps].hwnd_l);
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
	int i;

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
	 *
	 * alt + space switches windows in a workspace.
	 */
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		if (msg.message == WM_HOTKEY) {

			printf("hotk code: %d\n", (int) msg.wParam);

            if (msg.wParam == hotk_spc_code) {
                printf("alt+space!\n");
                try_swap_focus();
            }

			for (i = 0; i < g_layout.wsps_count; i++) {
				if (msg.wParam == g_layout.wsps[i].hotk_code) {
                    jump_to_ws(i);
					break;
				} else if (msg.wParam == g_layout.wsps[i].assign_code) {
                    /* TODO: if hwnd has previous assignment drop it */
                    assign_to_ws(i);
					break;
				}
			}
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
