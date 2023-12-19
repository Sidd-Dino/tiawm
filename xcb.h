#include <xcb/xcb.h>
#include <xcb/xproto.h>

#define mod_clean(mask)\
        (mask & ~(numlockmask|XCB_MOD_MASK_LOCK) &     \
        (XCB_MOD_MASK_SHIFT|XCB_MOD_MASK_CONTROL|      \
        XCB_MOD_MASK_1|XCB_MOD_MASK_2|XCB_MOD_MASK_3|  \
        XCB_MOD_MASK_4|XCB_MOD_MASK_5))

extern xcb_connection_t * dpy;
extern xcb_screen_t     * scr;
extern unsigned int       numlockmask;

xcb_keycode_t                     * get_keycode(xcb_keysym_t sym);
xcb_query_pointer_reply_t         * get_pntr(xcb_window_t w);
xcb_get_geometry_reply_t          * get_geom(xcb_window_t w);
xcb_get_window_attributes_reply_t * get_attr(xcb_window_t w);

int  setup_numlockmask(void);
void setup_buttons(void);

int  xcb_init(void);
void xcb_cleanup(void);

int  exists(xcb_window_t w);

int  get_x(xcb_window_t w);
void set_x(xcb_window_t w, int x);

int  get_y(xcb_window_t w);
void set_y(xcb_window_t w, int y);

int  get_width(xcb_window_t w);
void set_width(xcb_window_t w, int width);

int  get_height(xcb_window_t w);
void set_height(xcb_window_t w, int height);

void set_stack_mode(xcb_window_t w, int pos);
#define raise(w) set_stack_mode(w, XCB_STACK_MODE_ABOVE);
#define lower(w) set_stack_mode(w, XCB_STACK_MODE_BELOW);

void set_ignored(xcb_window_t w, int ignored);

void set_subscription(xcb_window_t w, int mask);

int  get_mapped(xcb_window_t w);
void set_mapped(xcb_window_t w, int map_state);

int  get_focused(void);
void set_focused(xcb_window_t w);

// vim:cc=81 ts=8 sts=8 sw=8:
