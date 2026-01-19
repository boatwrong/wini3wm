#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <wingdi.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>
#include <crtdbg.h>

#include "wini3wm.h"

HWND hwnd_top;

HWND hwnd_mingw;
HWND hwnd_vs;
HWND hwnd_chrome;
HWND hwnd_explorer;
HWND hwnd_psh;
HWND hwnd_npp;
HWND hwnd_excel;
HWND hwnd_adobe;


int bdb_SetForegroundWindow(HWND hwnd)
{
    int fullscreenWidth;
    int fullscreenHeight;

    /*
     * Source - https://stackoverflow.com/a/5299718
     * Posted by Czarek Tomczak, modified by community. See post 'Timeline' for change history
     * Retrieved 2026-01-19, License - CC BY-SA 3.0
     */
    HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };

    if (!GetMonitorInfo(hmon, &mi)) return -1;

    /*
     * Source - https://stackoverflow.com/a/2416613
     * Posted by Warpspace, modified by community. See post 'Timeline' for change history
     * Retrieved 2026-01-19, License - CC BY-SA 4.0
     */
    fullscreenWidth  = mi.rcWork.right - mi.rcWork.left;
    fullscreenHeight = mi.rcWork.bottom - mi.rcWork.top;

    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    SetWindowPos(hwnd, HWND_TOP, 0, 0, fullscreenWidth, fullscreenHeight, SWP_SHOWWINDOW);

    SetForegroundWindow(hwnd);

    return 0;
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int show_status(HINSTANCE hInstance, int nCmdShow)
{
    HWND hwnd;
    const char className[] = "MyWindowClass";

    /* Register class */
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.lpfnWndProc = WindowProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassEx(&wc))
    {
        MessageBoxA(NULL, "RegisterClassEx failed!", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    /* Create window */
    hwnd = CreateWindow(className, "Window Title", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if (hwnd == NULL)
	{
		MessageBoxA(NULL, "CreateWindow failed!", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}

    /* TODO: What to put as second arg */
	ShowWindow(hwnd, nCmdShow);

    return 0;
}




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
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

    /* if (show_status(hInstance, nCmdShow)) return -1; */

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_HOTKEY) {
            switch(msg.wParam) {
            case HOTK_A:
                /* SetWindowPos(hwnd_mingw, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED); */
                printf("jump a\n");
                bdb_SetForegroundWindow(hwnd_mingw);
                break;
            case HOTK_S:
                printf("jump s\n");
                bdb_SetForegroundWindow(hwnd_explorer);
                break;
            case HOTK_D:
                printf("jump d\n");
                bdb_SetForegroundWindow(hwnd_vs);
                break;
            case HOTK_F:
                printf("jump f\n");
                bdb_SetForegroundWindow(hwnd_chrome);
                break;
            case HOTK_J:
                printf("jump j\n");
                bdb_SetForegroundWindow(hwnd_psh);
                break;
            case HOTK_K:
                printf("jump k\n");
                bdb_SetForegroundWindow(hwnd_npp);
                break;
            case HOTK_L:
                printf("jump l\n");
                bdb_SetForegroundWindow(hwnd_excel);
                break;
            case HOTK_SC:
                printf("jump ;\n");
                bdb_SetForegroundWindow(hwnd_adobe);
                break;
            }

            /* 
             * Since we just jumped, do some processing to make sure we have
             * other windows correctly assigned.
             */
            EnumWindows(win_callbk_vb, nul);
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
