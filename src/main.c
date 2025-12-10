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

int IsAltTabWindow(HWND hwnd)
{
    TITLEBARINFO ti;
    HWND hwndTry, hwndWalk = NULL;

    if(!IsWindowVisible(hwnd))
        return 0;

    hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
    while(hwndTry != hwndWalk) 
    {
        hwndWalk = hwndTry;
        hwndTry = GetLastActivePopup(hwndWalk);
        if(IsWindowVisible(hwndTry)) 
            break;
    }
    if(hwndWalk != hwnd)
        return 0;

    /* the following removes some task tray programs and "Program Manager" */
    ti.cbSize = sizeof(ti);
    GetTitleBarInfo(hwnd, &ti);
    if(ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
        return 0;

    /*
     * Tool windows should not be displayed either, these do not appear in the
     * task bar.
     */
    if(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
        return 0;

    return 1;
}

int is_alt_tab_win(HWND hwnd)
{

    /* Start at the root owner */
    HWND hwnd_walk = GetAncestor(hwnd, GA_ROOTOWNER);

    /* See if we are the last active visible popup */
    HWND hwnd_try;

    /* Used later... */
    TITLEBARINFO ti;

    while ((hwnd_try = GetLastActivePopup(hwnd_walk)) != hwnd_try) {
        if (IsWindowVisible(hwnd_try))
            break;

        hwnd_walk = hwnd_try;
    }

    ti.cbSize = sizeof(ti);
    GetTitleBarInfo(hwnd, &ti);

    if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE) return 0;

    if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) return 0;

    return hwnd_walk == hwnd;
}

BOOL CALLBACK win_callbk(HWND hwnd, LPARAM lParam)
{
    const DWORD TITLE_SIZE = 1024;
    TCHAR windowTitle[TITLE_SIZE];
    GetWindowText(hwnd, windowTitle, TITLE_SIZE);

    if (is_alt_tab_win(hwnd) && IsWindowVisible(hwnd) &&
        GetWindowTextLength(hwnd) > 0) {

        if (strstr(windowTitle, mingw)) { hwnd_mingw = hwnd; }
        if (strstr(windowTitle, chrome)) { hwnd_chrome = hwnd; }
        if (strstr(windowTitle, vs)) { hwnd_vs = hwnd; }
        if (strstr(windowTitle, explorer)) { hwnd_explorer = hwnd; }
    }

    return TRUE;
}

HWND full_screen(HWND hwnd)
{
 HMONITOR hmon = MonitorFromWindow(hwnd,
                                   MONITOR_DEFAULTTONEAREST);
 MONITORINFO mi = { sizeof(mi) };
 if (!GetMonitorInfo(hmon, &mi)) return NULL;
 return CreateWindow(TEXT("static"),
       TEXT("something interesting might go here"),
       WS_POPUP | WS_VISIBLE,
       mi.rcMonitor.left,
       mi.rcMonitor.top,
       mi.rcMonitor.right - mi.rcMonitor.left,
       mi.rcMonitor.bottom - mi.rcMonitor.top,
       hwnd, NULL, NULL, 0);
}

int main(void)
{
    MSG msg = { };

    RegisterHotKey(NULL, HOTK_A, MOD_ALT, KEY_A);
    RegisterHotKey(NULL, HOTK_S, MOD_ALT, KEY_S);
    RegisterHotKey(NULL, HOTK_D, MOD_ALT, KEY_D);
    RegisterHotKey(NULL, HOTK_F, MOD_ALT, KEY_F);


    EnumWindows(win_callbk, NULL);


    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        if (msg.message == WM_HOTKEY) {
            switch(msg.wParam) {
            case HOTK_A:
                /* SetWindowPos(hwnd_mingw, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED); */
                SetForegroundWindow(hwnd_mingw);
                break;
            case HOTK_S:
                /* SetWindowPos(hwnd_explorer, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED); */
                SetForegroundWindow(hwnd_explorer);
                break;
            case HOTK_D:
                /* SetWindowPos(hwnd_vs, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED); */
                SetForegroundWindow(hwnd_vs);
                break;
            case HOTK_F:
                /* SetWindowPos(hwnd_chrome, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED); */
                SetForegroundWindow(hwnd_chrome);
                break;
            }

            /* 
             * Since we just jumped, do some processing to make sure we have
             * other windows correctly assigned.
             */
            EnumWindows(win_callbk, NULL);
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
