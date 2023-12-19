#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_keysyms.h>

#include "xcb.h"
#include "config.h"

xcb_connection_t * dpy;
xcb_screen_t     * scr;
unsigned int       numlockmask;
uint32_t           values[4];

// Borrowed from mcpcpc/xwm
// Wrapper to get xcb keycodes from keysymbol
xcb_keycode_t *
get_keycode(xcb_keysym_t ks) {
        xcb_key_symbols_t *ksyms = xcb_key_symbols_alloc(dpy);
        xcb_keycode_t     *kc    = NULL;
        if (ksyms) kc = xcb_key_symbols_get_keycode(ksyms, ks);
        xcb_key_symbols_free(ksyms);
        return kc;
}

xcb_query_pointer_reply_t *
get_pntr(xcb_window_t w) {
        return xcb_query_pointer_reply(
                        dpy,
                        xcb_query_pointer(dpy, w),
                        NULL
        );
}

xcb_get_geometry_reply_t *
get_geom(xcb_window_t w) {
        return xcb_get_geometry_reply(dpy, xcb_get_geometry(dpy, w), NULL);
}

xcb_get_window_attributes_reply_t *
get_attr(xcb_window_t w) {
        return xcb_get_window_attributes_reply(
                        dpy,
                        xcb_get_window_attributes(dpy, w),
                        NULL
        );
}

int
setup_numlockmask(void) {
        xcb_get_modifier_mapping_reply_t *reply;
        xcb_keycode_t *modmap, *numlock;

        reply = xcb_get_modifier_mapping_reply(dpy,
                        xcb_get_modifier_mapping_unchecked(dpy), NULL);
        if (!reply) return 0;

        modmap = xcb_get_modifier_mapping_keycodes(reply);
        if (!modmap) return 0;

        numlock = get_keycode(0xff7f); // XK_Num_Lock 0xff7f

        for (int i=4; numlock && i<8; i++) {
                for (int j=0; j<reply->keycodes_per_modifier; j++) {
                        xcb_keycode_t keycode = modmap[ j +
                                (i * reply->keycodes_per_modifier) ];

                        if (keycode == XCB_NO_SYMBOL)
                                continue;

                        for (int n=0; numlock[n] != XCB_NO_SYMBOL; n++)
                                if (numlock[n] == keycode) {
                                        numlockmask = 1 << i;
                                        break;
                                }
                }
        }

        free(reply);
        free(numlock);

        xcb_flush(dpy);
        return 1;
}

void
setup_buttons(void) {
        unsigned int modifiers[] = {
                0,
                XCB_MOD_MASK_LOCK,
                numlockmask,
                numlockmask | XCB_MOD_MASK_LOCK
        };

        for (int m=0; m<4; m++) {
                xcb_grab_button(dpy, 0, scr->root,
                        XCB_EVENT_MASK_BUTTON_MOTION          |
                                XCB_EVENT_MASK_BUTTON_PRESS   |
                                XCB_EVENT_MASK_BUTTON_RELEASE ,
                        XCB_GRAB_MODE_ASYNC,
                        XCB_GRAB_MODE_ASYNC,
                        scr->root,
                        XCB_NONE,
                        MOVE_BUTTON,
                        MOVE_MOD | modifiers[m]
                );
                xcb_grab_button(dpy, 0, scr->root,
                        XCB_EVENT_MASK_BUTTON_MOTION          |
                                XCB_EVENT_MASK_BUTTON_PRESS   |
                                XCB_EVENT_MASK_BUTTON_RELEASE ,
                        XCB_GRAB_MODE_ASYNC,
                        XCB_GRAB_MODE_ASYNC,
                        scr->root,
                        XCB_NONE,
                        RESIZE_BUTTON,
                        RESIZE_MOD | modifiers[m]
                );
        }

        xcb_flush(dpy);
}

int
xcb_init(void) {
        dpy = xcb_connect(NULL, NULL);
        if (xcb_connection_has_error(dpy)) return 0;

        scr = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;
        if (!scr) return 0;

        values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                  | XCB_EVENT_MASK_STRUCTURE_NOTIFY
                  | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
                  | XCB_EVENT_MASK_PROPERTY_CHANGE;

        xcb_generic_error_t *error = xcb_request_check(dpy,
                        xcb_change_window_attributes_checked(dpy, scr->root,
                                XCB_CW_EVENT_MASK, values));
        xcb_flush(dpy);

        if (error) {
                free(error);
                return 0;
        }

        if (!setup_numlockmask()) return 0;

        setup_buttons();
        return 1;
}

void
xcb_cleanup(void) {
        if (dpy) xcb_disconnect(dpy);
}

int
exists(xcb_window_t w) {
        xcb_get_window_attributes_reply_t *attr_r = get_attr(w);

        if (!attr_r)
                return 0;

        free(attr_r);
        return 1;
}

// Borrowed from sdhand/x11fs
// Bunch of functions to get and set window properties etc.
// All should be fairly self explanatory

#define DEFINE_NORM_SETTER(name, fn, prop) \
void set_##name(xcb_window_t wid, int arg) {\
        uint32_t values[] = {arg};\
        fn(dpy, wid, prop, values);\
        xcb_flush(dpy);\
}

DEFINE_NORM_SETTER(width,        xcb_configure_window,         XCB_CONFIG_WINDOW_WIDTH)
DEFINE_NORM_SETTER(height,       xcb_configure_window,         XCB_CONFIG_WINDOW_HEIGHT)
DEFINE_NORM_SETTER(x,            xcb_configure_window,         XCB_CONFIG_WINDOW_X)
DEFINE_NORM_SETTER(y,            xcb_configure_window,         XCB_CONFIG_WINDOW_Y)
DEFINE_NORM_SETTER(stack_mode,   xcb_configure_window,         XCB_CONFIG_WINDOW_STACK_MODE)
DEFINE_NORM_SETTER(ignored,      xcb_change_window_attributes, XCB_CW_OVERRIDE_REDIRECT)
DEFINE_NORM_SETTER(subscription, xcb_change_window_attributes, XCB_CW_EVENT_MASK)

#define DEFINE_GEOM_GETTER(name) \
int get_##name(xcb_window_t wid)\
{\
        xcb_get_geometry_reply_t *geom_r = get_geom(wid);\
        if(!geom_r)\
                return -1;\
        \
        int name = geom_r->name;\
        free(geom_r);\
        return name;\
}

DEFINE_GEOM_GETTER(width)
DEFINE_GEOM_GETTER(height)
DEFINE_GEOM_GETTER(x)
DEFINE_GEOM_GETTER(y)

int
get_mapped(xcb_window_t w) {
        xcb_get_window_attributes_reply_t *attr_r = get_attr(w);

        if (!attr_r)
                return -1;

        int map_state = attr_r->map_state;
        free(attr_r);
        return map_state == XCB_MAP_STATE_VIEWABLE;
}

void
set_mapped(xcb_window_t w, int map_state) {
        if (map_state)
                xcb_map_window(dpy, w);
        else
                xcb_unmap_window(dpy, w);
        xcb_flush(dpy);
}

int
get_focused(void) {
        xcb_get_input_focus_reply_t *reply;
        xcb_window_t focused;
        reply = xcb_get_input_focus_reply(
                        dpy,
                        xcb_get_input_focus(dpy),
                        NULL
        );

        if (!reply) return -1;

        focused = reply->focus;
        if (focused == scr->root) focused = 0;

        free(reply);
        return focused;
}

void
set_focused(xcb_window_t w) {
        xcb_set_input_focus(
                dpy,
                XCB_INPUT_FOCUS_POINTER_ROOT,
                w,
                XCB_CURRENT_TIME
        );
}

// vim:cc=81 ts=8 sts=8 sw=8:
