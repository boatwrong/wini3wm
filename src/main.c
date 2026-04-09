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

void assign_hwnd_to_wsps(HWND hwnd, int wsps_id)
{
    int num_hwnd = 1;

    if (g_layout.wsps[wsps_id].hwnd_l) {
        g_layout.wsps[wsps_id].hwnd_r = hwnd;
        num_hwnd++;
    } else {
        g_layout.wsps[wsps_id].hwnd_l = hwnd;
    }

    redraw_wsps(wsps_id, num_hwnd);
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
	TCHAR window_title[TITLE_SIZE];
	HWND hwnd;
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
					if (g_layout.wsps[i].hwnd_l) {
						printf("jump %c\n", g_layout.wsps[i].hotk_chr);
                        set_focus_window(i, g_layout.wsps[i].hwnd_l);
					} else {
						printf("%c not assigned\n", g_layout.wsps[i].hotk_chr);
					}
					break;
				} else if (msg.wParam == g_layout.wsps[i].assign_code) {
					printf("assign %c\n", g_layout.wsps[i].hotk_chr);
					/* todo: assign window logic not working as expected */
					hwnd = GetForegroundWindow();
					if (NULL == hwnd) {
						printf("hwnd is null\n");
					} else {
						GetWindowText(hwnd, window_title, TITLE_SIZE);
						printf("hwnd: %s\n", window_title);
                        assign_hwnd_to_wsps(hwnd, i);
					}
					break;
				}
			}
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
