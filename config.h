
#include <signal.h>
#include <xcb/xproto.h>
#include <X11/keysym.h>

#define MOD1  XCB_MOD_MASK_1
#define MOD4  XCB_MOD_MASK_4


static struct key keys[] = {
    {MOD4|XCB_MOD_MASK_SHIFT,  XK_q,   quit_wm,     -1 },
    {MOD1,                     XK_Tab, focus_next,  -1 },
    {MOD1|XCB_MOD_MASK_SHIFT,  XK_Tab, focus_prev,  -1 }
};