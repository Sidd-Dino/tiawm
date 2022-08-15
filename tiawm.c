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

static client            *list = {0}, *cur;//, *ws_list[10] = {0}, *cur;
static xcb_connection_t  *d;
static xcb_screen_t      *scr;
static unsigned int      numlockmask = 0;//, ws;

uint32_t values[4];

void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *) = {
    [XCB_BUTTON_PRESS]      =   event_button_press,
    [XCB_BUTTON_RELEASE]    =   event_button_release,
    [XCB_CONFIGURE_REQUEST] =   event_configure_request,
    [XCB_MAP_REQUEST]       =   event_map_request,
    [XCB_MAPPING_NOTIFY]    =   event_mapping_notify,
    [XCB_DESTROY_NOTIFY]    =   event_destroy_notify,
    [XCB_ENTER_NOTIFY]      =   event_enter_notify,
    [XCB_MOTION_NOTIFY]     =   event_motion_notify
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


void event_button_press(xcb_generic_event_t *ev) {
    xcb_button_press_event_t *e = (xcb_button_press_event_t *) ev;

    printf("%d\n", e->event);
}

void event_button_release(xcb_generic_event_t *ev) {
    ;
}

void event_configure_request(xcb_generic_event_t *ev) {
    ;
}

void event_map_request(xcb_generic_event_t *ev) {
    xcb_map_request_event_t *e = (xcb_map_request_event_t *) ev;

    values[0] = XCB_EVENT_MASK_ENTER_WINDOW;
    xcb_change_window_attributes_checked(d, e->window,
		XCB_CW_EVENT_MASK, values);

    win_add(e->window);
    cur=list->prev;

    xcb_map_window(d, e->window);

}

void event_mapping_notify(xcb_generic_event_t *ev) {
    ;
}

void event_destroy_notify(xcb_generic_event_t *ev) {
    ;
}

void event_enter_notify(xcb_generic_event_t *ev) {
    xcb_enter_notify_event_t *e = (xcb_enter_notify_event_t *) ev;

    printf("%d %d\n", e->root, e->event);
}

void event_motion_notify(xcb_generic_event_t *ev) {
    ;
}


void win_add(xcb_window_t w) {
    client *c;

    if (!(c = (client*) calloc(1, sizeof(client))))
        exit(1);
    
    c->w = w;

    if (list) {
        list->prev->next = c;
        c->prev          = list->prev;
        list->prev       = c;
        c->next          = list;    
    } else {
        list = c;
        c->prev = c->next = c;
    }
}

void win_del(xcb_window_t w) {
    ;
}

void win_focus(client *c) {
    cur = c;
    xcb_set_input_focus(d, XCB_INPUT_FOCUS_POINTER_ROOT, cur->w,
			XCB_CURRENT_TIME);
}

void win_focus_next(const int n) {
    ;
}

void win_focus_prev(const int n) {
    ;
}


void quit_wm(const int n) {
    ;
}

int grab_input(void) {
    unsigned int modifiers[] = {
		0,
		XCB_MOD_MASK_LOCK,
		numlockmask,
		numlockmask | XCB_MOD_MASK_LOCK
	};

    for (int m=0; m<4; m++) {
        xcb_grab_button(d, 0, scr->root,
            XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
            scr->root, XCB_NONE, MOVE_BUTTON, MOVE_MOD | modifiers[m]);
        xcb_grab_button(d, 0, scr->root,
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
