// tiawm - This Is A Window Manager

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "tiawm.h"

xcb_connection_t *d;
xcb_screen_t *scr;
xcb_drawable_t win;
xcb_get_geometry_reply_t *geom;

uint32_t values[3];

void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *) = {
    [XCB_BUTTON_PRESS]      =   button_press,
    [XCB_BUTTON_RELEASE]    =   button_release,
    [XCB_CONFIGURE_REQUEST] =   configure_request,
    [XCB_KEY_PRESS]         =   key_press,
    [XCB_MAP_REQUEST]       =   map_request,
    [XCB_MAPPING_NOTIFY]    =   mapping_notify,
    [XCB_DESTROY_NOTIFY]    =   destroy_notify,
    [XCB_ENTER_NOTIFY]      =   enter_notify,
    [XCB_MOTION_NOTIFY]     =   motion_notify
};

#include "config.h"


void button_press (xcb_generic_event_t *ev) {
    xcb_button_press_event_t *e;
    e = (xcb_button_press_event_t *) ev;
    win = e->child;

    values[0] = XCB_STACK_MODE_ABOVE;
    xcb_configure_window(d, win, XCB_CONFIG_WINDOW_STACK_MODE, values);

}

void button_release (xcb_generic_event_t *ev) {
    printf("but release\n");
}

void configure_request (xcb_generic_event_t *ev) {
    printf("config request\n");
}

void key_press (xcb_generic_event_t *ev) {
    ;
}

void map_request (xcb_generic_event_t *ev) {
    xcb_map_request_event_t *e;
    e = (xcb_map_request_event_t *) ev;

    xcb_map_window(d, e->window);
}

void mapping_notify (xcb_generic_event_t *ev) {
    printf("mapping notify\n");
}

void destroy_notify (xcb_generic_event_t *ev) {
    ;
}

void enter_notify (xcb_generic_event_t *ev) {
    ;
}

void motion_notify (xcb_generic_event_t *ev) {
    ;
}


void focus_next(const int n) {
    ;
}

void focus_prev(const int n) {
    ;
}

void quit_wm(const int n) {
    ;
}


void grab_input(xcb_drawable_t root) {
    values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;
    xcb_change_window_attributes_checked(d, root, XCB_CW_EVENT_MASK, values);

    xcb_flush(d);
    
    xcb_ungrab_key(d, XCB_GRAB_ANY, root, XCB_MOD_MASK_ANY);

    xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(d);
    xcb_keycode_t *keycode;

    for (uint8_t i = 0; i < sizeof(keys)/sizeof(*keys); i++)
        if ((keycode = xcb_key_symbols_get_keycode(keysyms, keys[i].keysym)))
            xcb_grab_key(d, 1, root, keys[i].mod, *keycode,
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    xcb_grab_button(d, 0, root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, root,
        XCB_NONE, MOVE_BUTTON, MOVE_MOD );
    
    xcb_grab_button(d, 0, root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION,
        XCB_GRAB_MODE_ASYNC,XCB_GRAB_MODE_ASYNC, root,
        XCB_NONE, RESIZE_BUTTON, RESIZE_MOD );

    xcb_key_symbols_free(keysyms);

    xcb_flush(d);
}

int main(void) {
    xcb_generic_event_t *ev;
    unsigned int ev_type;

    signal(SIGCHLD, SIG_IGN);

    d = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(d)) exit(1);

    scr = xcb_setup_roots_iterator(xcb_get_setup(d)).data;

    if (!scr) exit(1);

    grab_input(scr->root);

    while ((ev = xcb_wait_for_event(d))) {
        ev_type = ev->response_type & ~0x80;

        if (events[ev_type]) {
            events[ev_type](ev);
            xcb_flush(d);
        }

        free(ev);
    }
}