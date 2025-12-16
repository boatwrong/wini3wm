#ifndef _WIN_I3_WM_H
#define _WIN_I3_WM_H

/* search strings for windows */
static const char *mingw = "User@LT-DBD";
static const char *chrome = "Google Chrome";
static const char *vs = "Visual Studio";
static const char *explorer = "File Explorer";
static const char *psh = "PowerShell";
static const char *npp = "Notepad++";
static const char *excel = "Excel";
static const char *adobe = "Adobe";

/* unique id's for hotkeys, arbitrary numbers */
#define HOTK_A 1010
#define HOTK_S 1011
#define HOTK_D 1012
#define HOTK_F 1013
#define HOTK_J 1014
#define HOTK_K 1015
#define HOTK_L 1016
#define HOTK_SC 1017

/* char values for hotkeys */
#define KEY_A 'A'
#define KEY_S 'S'
#define KEY_D 'D'
#define KEY_F 'F'
#define KEY_J 'J'
#define KEY_K 'K'
#define KEY_L 'L'
#define KEY_SC VK_OEM_1

/* misc macros */
#define TITLE_SIZE 1024

#endif /* _WIN_I3_WM_H */
