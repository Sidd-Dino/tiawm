#include <sys/types.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

typedef struct key {
    uint32_t mod;
    xcb_keysym_t keysym;
    void (*function)(const int n);
    int n;
} key;

typedef struct client {
    struct client *next, *prev;
    int f, wx,wy;
    unsigned int ww, wh;
    xcb_window_t w;
} client;


xcb_keycode_t* xcb_get_keycode(xcb_keysym_t keysym);
xcb_keysym_t xcb_get_keysym(xcb_keycode_t keycode);


void event_button_press (xcb_generic_event_t *ev);
void event_button_release (xcb_generic_event_t *ev);
void event_configure_request (xcb_generic_event_t *ev);
void event_key_press (xcb_generic_event_t *ev);
void event_map_request (xcb_generic_event_t *ev);
void event_mapping_notify (xcb_generic_event_t *ev);
void event_destroy_notify (xcb_generic_event_t *ev);
void event_enter_notify (xcb_generic_event_t *ev);
void event_motion_notify (xcb_generic_event_t *ev);


void win_add(xcb_window_t w);
void win_del(xcb_window_t w);
void win_center(client *c);
void win_focus(client *c);
void win_focus_next(const int n);
void win_focus_prev(const int n);


void quit_wm(const int n);
int grab_input(void);
int setup(void);