#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>
#include <crtdbg.h>

#include "wini3wm.h"

HWND hwnd_mingw;
HWND hwnd_vs;
HWND hwnd_chrome;
HWND hwnd_explorer;
HWND hwnd_psh;
HWND hwnd_npp;
HWND hwnd_excel;
HWND hwnd_adobe;

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
    TCHAR windowTitle[TITLE_SIZE];
    GetWindowText(hwnd, windowTitle, TITLE_SIZE);

    if (is_alt_tab_win(hwnd) && IsWindowVisible(hwnd) &&
        GetWindowTextLength(hwnd) > 0) {

        printf("%s\n", windowTitle);

        if (strstr(windowTitle, mingw)) { hwnd_mingw = hwnd; }
        if (strstr(windowTitle, chrome)) { hwnd_chrome = hwnd; }
        if (strstr(windowTitle, vs)) { hwnd_vs = hwnd; }
        if (strstr(windowTitle, explorer)) { hwnd_explorer = hwnd; }
        if (strstr(windowTitle, psh)) { hwnd_psh = hwnd; }
        if (strstr(windowTitle, npp)) { hwnd_npp = hwnd; }
        if (strstr(windowTitle, excel)) { hwnd_excel = hwnd; }
        if (strstr(windowTitle, adobe)) { hwnd_adobe = hwnd; }
    }

    return TRUE;
}


BOOL CALLBACK win_callbk(HWND hwnd, LPARAM lParam)
{
    TCHAR windowTitle[TITLE_SIZE];
    GetWindowText(hwnd, windowTitle, TITLE_SIZE);

    if (is_alt_tab_win(hwnd) && IsWindowVisible(hwnd) &&
        GetWindowTextLength(hwnd) > 0) {

        if (strstr(windowTitle, mingw)) { hwnd_mingw = hwnd; }
        if (strstr(windowTitle, chrome)) { hwnd_chrome = hwnd; }
        if (strstr(windowTitle, vs)) { hwnd_vs = hwnd; }
        if (strstr(windowTitle, explorer)) { hwnd_explorer = hwnd; }
        if (strstr(windowTitle, psh)) { hwnd_psh = hwnd; }
        if (strstr(windowTitle, npp)) { hwnd_npp = hwnd; }
        if (strstr(windowTitle, excel)) { hwnd_excel = hwnd; }
        if (strstr(windowTitle, adobe)) { hwnd_adobe = hwnd; }
    }

    return TRUE;
}

int main(void)
{
    MSG msg = { 0 };
    LPARAM nul = 0;

    RegisterHotKey(NULL, HOTK_A, MOD_ALT, KEY_A);
    RegisterHotKey(NULL, HOTK_S, MOD_ALT, KEY_S);
    RegisterHotKey(NULL, HOTK_D, MOD_ALT, KEY_D);
    RegisterHotKey(NULL, HOTK_F, MOD_ALT, KEY_F);
    RegisterHotKey(NULL, HOTK_J, MOD_ALT, KEY_J);
    RegisterHotKey(NULL, HOTK_K, MOD_ALT, KEY_K);
    RegisterHotKey(NULL, HOTK_L, MOD_ALT, KEY_L);
    RegisterHotKey(NULL, HOTK_SC, MOD_ALT, KEY_SC);


    EnumWindows(win_callbk_vb, nul);


    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        if (msg.message == WM_HOTKEY) {
            switch(msg.wParam) {
            case HOTK_A:
                /* SetWindowPos(hwnd_mingw, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED); */
                printf("alt + a\n");
                SetForegroundWindow(hwnd_mingw);
                break;
            case HOTK_S:
                printf("alt + s\n");
                SetForegroundWindow(hwnd_explorer);
                break;
            case HOTK_D:
                printf("alt + d\n");
                SetForegroundWindow(hwnd_vs);
                break;
            case HOTK_F:
                printf("alt + f\n");
                SetForegroundWindow(hwnd_chrome);
                break;
            case HOTK_J:
                printf("alt + j\n");
                SetForegroundWindow(hwnd_psh);
                break;
            case HOTK_K:
                printf("alt + k\n");
                SetForegroundWindow(hwnd_npp);
                break;
            case HOTK_L:
                printf("alt + l\n");
                SetForegroundWindow(hwnd_excel);
                break;
            case HOTK_SC:
                printf("alt + ;\n");
                SetForegroundWindow(hwnd_adobe);
                break;
            }

            /* 
             * Since we just jumped, do some processing to make sure we have
             * other windows correctly assigned.
             */
            EnumWindows(win_callbk, nul);
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
