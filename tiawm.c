// tiawm - This Is A Window Manager

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "tiawm.h"
#include "config.h"


static xcb_connection_t *d;
static xcb_screen_t     *scr;
static mouse            btn_press;
static unsigned int     numlockmask = 0;

uint32_t values[4];

void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *) = {
    [XCB_BUTTON_PRESS]      =   handle_button_press,
    [XCB_BUTTON_RELEASE]    =   handle_button_release,
    [XCB_MAP_REQUEST]       =   handle_map_request,
    [XCB_DESTROY_NOTIFY]    =   handle_destroy_notify,
    [XCB_ENTER_NOTIFY]      =   handle_enter_notify,
    [XCB_MOTION_NOTIFY]     =   handle_motion_notify
};


/* Borrowed from mcpcpc/xwm  */
/* Wrapper to get xcb keycodes from keysymbol */
xcb_keycode_t* xcb_get_keycode(xcb_keysym_t keysym) {
        xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(d);
        xcb_keycode_t *keycode;
        keycode = (!(keysyms) ? NULL : xcb_key_symbols_get_keycode(keysyms, keysym));
        xcb_key_symbols_free(keysyms);
        return keycode;
}

/* Borrowed from mcpcpc/xwm */
/* Wrapper to get xcb keysymbol from keycode */
xcb_keysym_t xcb_get_keysym(xcb_keycode_t keycode) {
        xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(d);
        xcb_keysym_t keysym;
        keysym = (!(keysyms) ? 0 : xcb_key_symbols_get_keysym(keysyms, keycode, 0));
        xcb_key_symbols_free(keysyms);
        return keysym;
}


void handle_button_press(xcb_generic_event_t *ev) {
        xcb_button_press_event_t *e = (xcb_button_press_event_t *) ev;

        if (!e->child || e->child == scr->root)
                return;

        btn_press.window  = e->child;
        btn_press.button  = e->detail;
        btn_press.root_x  = e->root_x;
        btn_press.root_y  = e->root_y;
	values[0] = XCB_STACK_MODE_ABOVE;
	xcb_configure_window(d, btn_press.window, XCB_CONFIG_WINDOW_STACK_MODE, values);
}

void handle_button_release(xcb_generic_event_t *ev) {
        btn_press.window = 0;
}

void handle_map_request(xcb_generic_event_t *ev) {
        xcb_map_request_event_t *e = (xcb_map_request_event_t *) ev;

        values[0] = XCB_EVENT_MASK_ENTER_WINDOW;
        xcb_change_window_attributes_checked(d, e->window,
                XCB_CW_EVENT_MASK, values);

        xcb_map_window(d, e->window);
        printf("MAP RQST %d\n", e->window);
}

void handle_destroy_notify(xcb_generic_event_t *ev) {
        xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *) ev;
	xcb_kill_client(d, e->window);
}

void handle_enter_notify(xcb_generic_event_t *ev) {
        xcb_enter_notify_event_t *e = (xcb_enter_notify_event_t *) ev;
        if (!e->event || e->event == scr->root)
                return;
        
        xcb_set_input_focus(d, XCB_INPUT_FOCUS_POINTER_ROOT, e->event,
	        XCB_CURRENT_TIME);
}

void handle_motion_notify(xcb_generic_event_t *ev) {
        if(!btn_press.window) return;

        xcb_get_geometry_reply_t  *geom;
        xcb_query_pointer_reply_t *poin;
        uint16_t                  mask;
        int                       dx, dy;

        geom = xcb_get_geometry_reply(d, xcb_get_geometry(d, btn_press.window), 0);
        poin = xcb_query_pointer_reply(d, xcb_query_pointer(d, scr->root), 0);
        mask = mod_clean(poin->mask);

        dx = poin->root_x - btn_press.root_x;
        dy = poin->root_y - btn_press.root_y;

        if ( mask == MOVE_MOD && btn_press.button == MOVE_BUTTON ) {
                values[0] = geom->x + dx;
                values[1] = geom->y + dy;
                xcb_configure_window(d, btn_press.window, XCB_CONFIG_WINDOW_X
                        | XCB_CONFIG_WINDOW_Y, values);
        }

        if ( mask == RESIZE_MOD && btn_press.button == RESIZE_BUTTON ) {
                values[0] = MAX((int)geom->width  + dx, MIN_WINDOW_WIDTH);
                values[1] = MAX((int)geom->height + dy, MIN_WINDOW_HEIGHT);
                xcb_configure_window(d, btn_press.window, XCB_CONFIG_WINDOW_WIDTH
                        | XCB_CONFIG_WINDOW_HEIGHT, values);
        }

        btn_press.root_x = poin->root_x;
        btn_press.root_y = poin->root_y;
}


int grab_input(void) {
        unsigned int modifiers[] = {
                0,
                XCB_MOD_MASK_LOCK,
                numlockmask,
                numlockmask | XCB_MOD_MASK_LOCK
        };

        for (int m=0; m<4; m++) {
        xcb_grab_button(d, 0, scr->root, XCB_EVENT_MASK_BUTTON_MOTION |
                XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                scr->root, XCB_NONE, MOVE_BUTTON, MOVE_MOD | modifiers[m]);
        xcb_grab_button(d, 0, scr->root, XCB_EVENT_MASK_BUTTON_MOTION |
                XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                scr->root, XCB_NONE, RESIZE_BUTTON, RESIZE_MOD | modifiers[m]);
    }

    xcb_flush(d);
    return 1;
}

/* Borrowed from 2bwm */
int setup(void) {
        values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                  | XCB_EVENT_MASK_STRUCTURE_NOTIFY
                  | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
                  | XCB_EVENT_MASK_PROPERTY_CHANGE;
    
    xcb_change_window_attributes_checked(d, scr->root,
                                        XCB_CW_EVENT_MASK, values);


        xcb_get_modifier_mapping_reply_t *reply;
        xcb_keycode_t *modmap, *numlock;

        reply = xcb_get_modifier_mapping_reply(d,
                        xcb_get_modifier_mapping_unchecked(d), NULL);
        if (!reply) return 0;

        modmap = xcb_get_modifier_mapping_keycodes(reply);
        if (!modmap) return 0;

        numlock = xcb_get_keycode(0xff7f); // XK_Num_Lock 0xff7f
        
        for (int i=4; i<8; i++) {
                for (int j=0; j<reply->keycodes_per_modifier; j++) {
                xcb_keycode_t keycode = modmap[i*reply->keycodes_per_modifier+j];
                if (keycode == XCB_NO_SYMBOL)
                        continue;
                
                if (numlock != NULL)
                        for (int n=0; numlock[n] != XCB_NO_SYMBOL; n++)
                        if (numlock[n] == keycode) {
                                numlockmask = 1 << i;
                                break;
                        }
                }
        }

        free(reply);
        free(numlock);

        xcb_flush(d);

        return 1;
}

int main(void) {
        xcb_generic_event_t *ev;
        unsigned int ev_type;

        signal(SIGCHLD, SIG_IGN);

        d   = xcb_connect(NULL, NULL);
        if (xcb_connection_has_error(d)) exit(1);

        scr = xcb_setup_roots_iterator(xcb_get_setup(d)).data;
        if (!scr) exit(1);

        if (!setup()) exit(1);
        if (!grab_input()) exit(1);

        while ((ev = xcb_wait_for_event(d))) {
                ev_type = ev->response_type & ~0x80;

                if (events[ev_type]) {
                        events[ev_type](ev);
                        xcb_flush(d);
                }

                free(ev);
        }
}
