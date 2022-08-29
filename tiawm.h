#include <xcb/xproto.h>

#define mod_clean(mask) (mask & ~(numlockmask|XCB_MOD_MASK_LOCK) &              \
        (XCB_MOD_MASK_SHIFT|XCB_MOD_MASK_CONTROL|                               \
        XCB_MOD_MASK_1|XCB_MOD_MASK_2|XCB_MOD_MASK_3|                           \
        XCB_MOD_MASK_4|XCB_MOD_MASK_5))

#define MAX(a,b) ((a) > (b) ? (a) : (b))

typedef struct mouse {
        xcb_window_t window;
        unsigned int button;
        int16_t      root_x, root_y;
} mouse;

xcb_keycode_t* xcb_get_keycode(xcb_keysym_t keysym);
xcb_keysym_t xcb_get_keysym(xcb_keycode_t keycode);


void handle_button_press (xcb_generic_event_t *ev);
void handle_button_release (xcb_generic_event_t *ev);
void handle_map_request (xcb_generic_event_t *ev);
void handle_destroy_notify (xcb_generic_event_t *ev);
void handle_enter_notify (xcb_generic_event_t *ev);
void handle_motion_notify (xcb_generic_event_t *ev);

int grab_input(void);
int setup(void);