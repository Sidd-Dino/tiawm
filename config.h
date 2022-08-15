#ifndef CONFIG_H
#define CONFIG_H

#define MOVE_BUTTON     1
#define RESIZE_BUTTON   1
#define MOVE_MOD        XCB_MOD_MASK_4
#define RESIZE_MOD      XCB_MOD_MASK_1

#define MOD1            XCB_MOD_MASK_1
#define MOD4            XCB_MOD_MASK_4
#define SHFT            XCB_MOD_MASK_SHIFT

struct key keys[] = {
    {MOD4|SHFT,  0x0071, quit_wm,     -1 }, // XK_q   0x0071
    {MOD1,       0xff09, win_focus_next,  -1 }, // XK_Tab 0xff09
    {MOD1|SHFT,  0xff09, win_focus_prev,  -1 }
};

#endif