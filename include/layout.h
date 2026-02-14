#ifndef _WINI3_LAYOUT_H
#define _WINI3_LAYOUT_H

struct i3_workspace {
	char title[16];
	int hotk_chr;
	int hotk_code;
	int assign_code;
	HWND hwnd;
};

struct i3_layout {
	struct i3_workspace *wsps;
	size_t wsps_count;
};

void init_layout(void);

extern struct i3_layout g_layout;

#endif /* _WINI3_LAYOUT_H */
