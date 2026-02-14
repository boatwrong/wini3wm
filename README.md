# Wini3wm

Windows i3 "window manager"

_Very_ hacky attempt at minimal `i3` style window manager for Windows. Haven't
found a great way to do window management on Windows and am stuck there for work
so might as well make it enjoyable. Alt-tabbing is awful and I want a similar
experience that I get on home desktop.

## Dependencies:

* [w64devkit](https://github.com/skeeto/w64devkit)

## Install:

Must build from source - not packaging an executable yet.

Build steps:

1. `make`
2. Executable (`i3wm.exe`) is in `./build/bin`

Usage:

Run the executable and off you go.

There currently are only 8 predefined workspaces which can be navigated to using
`ALT + <workspace>`. The workspaces are the home row keys on QWERTY layout
(a, s, d, f, j, k, l, ;). To assign a window to a workspace use `ALT + SHIFT +
<workspace>`. Currently only one window is allowed per workspace.

Features missing from `i3` I'd like to add:

* Config file (user defined:  workspaces, hotkeys, etc.)
* A status bar of sorts to list active workspaces and workspaces w/ an assigned
window.
* Tiling? This is not a majorly important feature to me and would be more
complicated so will likely not happend.
