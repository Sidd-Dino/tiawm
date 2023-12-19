// tiawm - This Is A Window Manager

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_keysyms.h>

#include "xcb.h"
#include "list.h"
#include "config.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))


typedef struct button button;
struct button{
        xcb_window_t win;
        unsigned int btn;
        int16_t      rtx, rty;
};


static void handle_button_press(xcb_generic_event_t *ev);
static void handle_button_release(xcb_generic_event_t *ev);
static void handle_map_request(xcb_generic_event_t *ev);
static void handle_destroy_notify(xcb_generic_event_t *ev);
static void handle_enter_notify(xcb_generic_event_t *ev);
static void handle_motion_notify(xcb_generic_event_t *ev);


static void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *) = {
    [XCB_MAP_REQUEST]       =   handle_map_request,
    [XCB_DESTROY_NOTIFY]    =   handle_destroy_notify,
    [XCB_ENTER_NOTIFY]      =   handle_enter_notify,
    [XCB_BUTTON_PRESS]      =   handle_button_press,
    [XCB_BUTTON_RELEASE]    =   handle_button_release,
    [XCB_MOTION_NOTIFY]     =   handle_motion_notify
};

static button btn;


static void
handle_map_request(xcb_generic_event_t *ev) {
        xcb_map_request_event_t *e = (xcb_map_request_event_t *) ev;

        set_subscription(e->window, XCB_EVENT_MASK_ENTER_WINDOW);
        set_mapped(e->window, 1);
}

static void
handle_destroy_notify(xcb_generic_event_t *ev) {
        xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *) ev;
	xcb_kill_client(dpy, e->window);
}

static void
handle_button_press(xcb_generic_event_t *ev) {
        xcb_button_press_event_t *e = (xcb_button_press_event_t *) ev;

        if (!e->child || e->child == scr->root)
                return;

        btn = (button){
                .win  = e->child,
                .btn  = e->detail,
                .rtx  = e->root_x,
                .rty  = e->root_y
        };

        raise(e->child);
}

static void
handle_button_release(xcb_generic_event_t *ev) {
        btn = (button){0};
}

static void
handle_enter_notify(xcb_generic_event_t *ev) {
        xcb_enter_notify_event_t *e = (xcb_enter_notify_event_t *) ev;
        if (!e->event || e->event == scr->root)
                return;
        set_focused(e->event);
}

static void
handle_motion_notify(xcb_generic_event_t *ev) {
        if (!btn.win) return;

        xcb_query_pointer_reply_t *poin;
        uint16_t mask;
        int dx, dy;

        poin = get_pntr(scr->root);
        mask = mod_clean(poin->mask);
        dx   = poin->root_x - btn.rtx;
        dy   = poin->root_y - btn.rty;

        if (mask == MOVE_MOD && btn.btn == MOVE_BUTTON) {
                set_x(btn.win, get_x(btn.win) + dx);
                set_y(btn.win, get_y(btn.win) + dy);
        }

        if (mask == RESIZE_MOD && btn.btn == RESIZE_BUTTON) {
                set_width(btn.win,
                        MAX((int)(get_width(btn.win) + dx), MINIMUM_WIDTH)
                );
                set_height(btn.win,
                        MAX((int)(get_height(btn.win) + dy), MINIMUM_HEIGHT)
                );
        }

        btn.rtx = poin->root_x;
        btn.rty = poin->root_y;
}

int main(void) {
        xcb_generic_event_t *ev;
        unsigned int ev_type;

        signal(SIGCHLD, SIG_IGN);

        if (!xcb_init()) exit(1); 

        while ((ev = xcb_wait_for_event(dpy))) {
                ev_type = ev->response_type & ~0x80;

                if (events[ev_type]) {
                        events[ev_type](ev);
                        xcb_flush(dpy);
                }

                free(ev);
        }
}

// vim:cc=81 ts=8 sts=8 sw=8:
