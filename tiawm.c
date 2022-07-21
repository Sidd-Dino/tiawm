// tiawm - This Is A Window Manager

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include <X11/keysym.h>

#include "tiawm.h"

xcb_connection_t *d;
xcb_screen_t *scr;
xcb_drawable_t win;

uint32_t values[3];

#include "config.h"

void focus_next(const int n) {
    ;
}

void focus_prev(const int n) {
    ;
}

void grab_input(xcb_drawable_t root) {
    values[0] = XCB_EVENT_MASK_ENTER_WINDOW |
            XCB_EVENT_MASK_FOCUS_CHANGE |
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;

    xcb_change_window_attributes_checked(d, root, XCB_CW_EVENT_MASK, values);

    // xcb_ungrab_key(d, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);

    xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(d);
    xcb_keycode_t *keycode;

    for (int i = 0; i < sizeof(keys)/sizeof(*keys); i++)
        if ((keycode = xcb_key_symbols_get_keycode(keysyms, keys[i].keysym)))
            xcb_grab_key(d, 1, root, keys[i].mod, *keycode,
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    xcb_key_symbols_free(keysyms);
}

void quit_wm(const int n) {
    ;
}

int main(void) {
    d = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(d)) return 1;

    scr = xcb_setup_roots_iterator(xcb_get_setup(d)).data;

    grab_input(scr->root);
}