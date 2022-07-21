#include <xcb/xcb.h>

struct key {
    uint32_t mod;
    xcb_keysym_t keysym;
    void (*function)(const int n);
    int n;
};

void grab_input(xcb_drawable_t root);
void focus_next(const int n);
void focus_prev(const int n);
void quit_wm(const int n);