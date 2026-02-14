#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>
#include <crtdbg.h>

#include "layout.h"

#define TITLE_SIZE 1024

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
	TCHAR window_title[TITLE_SIZE];
	HWND hwnd;
	MSG msg = { 0 };
	int i;

	init_layout();

	/*
	 * initial layout defines the hotkeys, but not the windows assigned.
	 *
	 * alt + shitf + hotkey assigns a window.
	 *
	 * alt + hotkey jumps to a window.
	 *
	 */
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		if (msg.message == WM_HOTKEY) {

			printf("hotk code: %d\n", (int) msg.wParam);

			for (i = 0; i < g_layout.wsps_count; i++) {
				if (msg.wParam == g_layout.wsps[i].hotk_code) {
					if (g_layout.wsps[i].hwnd) {
						SetForegroundWindow(g_layout.wsps[i].hwnd);
						printf("jump %c\n", g_layout.wsps[i].hotk_chr);
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
						g_layout.wsps[i].hwnd = hwnd;
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
