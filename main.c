#include <windows.h>
#include <stdio.h>

HWND hwnd_mingw;
HWND hwnd_ide;
HWND hwnd_chrome;

const static char *str_mingw = "LT-DBD";
const static char *str_ide = "Visual Studio;
const static char *str_chrome = "Google Chrome";

#define HOTK_A 'A'
#define HOTK_S 'S'
#define HOTK_D 'D'
#define HOTK_F 'F'

#define ID_HOTK_A 1000
#define ID_HOTK_S 1001
#define ID_HOTK_D 1002
#define ID_HOTK_F 1003

int IsAltTabWindow(HWND hwnd)
{
	/* Start at the root owner */
	HWND hwndWalk = GetAncestor(hwnd, GA_ROOTOWNER);
	/* See if we are the last active visible popup */
	HWND hwndTry;
	while ((hwndTry = GetLastActivePopup(hwndWalk)) != hwndTry) {
		if (IsWindowVisible(hwndTry)) break;
		hwndWalk = hwndTry;
	}
	return hwndWalk == hwnd;
}

int CALLBACK WindowCallback(HWND hwnd, LPARAM lParam)
{
    const DWORD TITLE_SIZE = 1024;
    TCHAR windowTitle[TITLE_SIZE];
    GetWindowText(hwnd, windowTitle, TITLE_SIZE);

    if (IsAltTabWindow(hwnd) && IsWindowVisible(hwnd))
        printf("Window (%d): %s\n", IsAltTabWindow(hwnd), windowTitle);

    return 1;
}

int main(void)
{
	RegisterHotKey(NULL, ID_HOTK_A, MOD_ALT, HOTK_A);
	RegisterHotKey(NULL, ID_HOTK_S, MOD_ALT, HOTK_S);
	RegisterHotKey(NULL, ID_HOTK_D, MOD_ALT, HOTK_D);
	RegisterHotKey(NULL, ID_HOTK_F, MOD_ALT, HOTK_F);

	EnumWindows(WindowCallback, NULL);

	return 0;
}
