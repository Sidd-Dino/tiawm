#include <sys/types.h>
#include <xcb/xcb.h>

struct key {
    uint32_t mod;
    xcb_keysym_t keysym;
    void (*function)(const int n);
    int n;
};

typedef struct client {
    struct client *next, *prev;
    int f, wx,wy;
    unsigned int ww, wh;
    xcb_drawable_t w;
} client;

void button_press (xcb_generic_event_t *ev);
void button_release (xcb_generic_event_t *ev);
void configure_request (xcb_generic_event_t *ev);
void key_press (xcb_generic_event_t *ev);
void map_request (xcb_generic_event_t *ev);
void mapping_notify (xcb_generic_event_t *ev);
void destroy_notify (xcb_generic_event_t *ev);
void enter_notify (xcb_generic_event_t *ev);
void motion_notify (xcb_generic_event_t *ev);

void focus_next(const int n);
void focus_prev(const int n);
void quit_wm(const int n);

void grab_input(xcb_drawable_t root);