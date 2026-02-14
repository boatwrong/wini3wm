#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "layout.h"

static int hotk_code_default[] = { 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017 };
static int assign_code_default[] = { 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025 };

static int hotk_chr_default[] = { 'A', 'S', 'D', 'F', 'J', 'K', 'L', VK_OEM_1 };

struct i3_layout g_layout;

void init_layout(void)
{

	/* 
	 * todo: take a config file to determine workspaces instead of
	 * 	 defaults. 
	 */

	int i;
	size_t ws_size = sizeof(*(g_layout.wsps));
	g_layout.wsps_count = 8;
	g_layout.wsps = malloc(ws_size * g_layout.wsps_count);

	if (!g_layout.wsps)
		goto init_error;

	for (i = 0; i < g_layout.wsps_count; i++) {
		printf("setup ws: %d, key: %c, code: %d assign: %d\n", i, hotk_chr_default[i], hotk_code_default[i], assign_code_default[i]);

		if (!RegisterHotKey(NULL, hotk_code_default[i], MOD_ALT, hotk_chr_default[i])) 
			goto init_error;

		if (!RegisterHotKey(NULL, assign_code_default[i], MOD_ALT | MOD_SHIFT, hotk_chr_default[i])) 
			goto init_error;

		g_layout.wsps[i].hwnd = NULL;
		g_layout.wsps[i].hotk_chr = hotk_chr_default[i];
		g_layout.wsps[i].hotk_code = hotk_code_default[i];
		g_layout.wsps[i].assign_code = assign_code_default[i];
	}



	return;

init_error:
	fprintf(stderr, "Error, initialization failed\n");
	exit(-1);
}
