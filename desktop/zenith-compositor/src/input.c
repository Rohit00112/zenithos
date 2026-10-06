/*
 * Zenith OS — Compositor
 *
 * input.c — Input device management (keyboard, pointer)
 *
 * Handles cursor motion, button clicks, scroll, keyboard input,
 * and interactive window operations (move/resize).
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

#include "compositor.h"
#include "input.h"
#include "view.h"
#include "keybindings.h"

/* ========================================================================= */
/* Cursor (pointer) handlers                                                  */
/* ========================================================================= */

/**
 * Process cursor motion — either pass through to the surface under the
 * cursor, or handle an interactive move/resize operation.
 */
static void process_cursor_motion(struct zenith_compositor *compositor,
                                   uint32_t time) {
    if (compositor->cursor_mode == ZENITH_CURSOR_MOVE) {
        /* Move the grabbed window */
        struct zenith_view *view = compositor->grabbed_view;
        if (view) {
            wlr_scene_node_set_position(&view->scene_tree->node,
                compositor->cursor->x - compositor->grab_x,
                compositor->cursor->y - compositor->grab_y);
        }
        return;
    }

    if (compositor->cursor_mode == ZENITH_CURSOR_RESIZE) {
        /* Resize the grabbed window */
        struct zenith_view *view = compositor->grabbed_view;
        if (view) {
            double border_x = compositor->cursor->x - compositor->grab_x;
            double border_y = compositor->cursor->y - compositor->grab_y;

            int new_left = compositor->grab_geobox.x;
            int new_right = compositor->grab_geobox.x +
                            compositor->grab_geobox.width;
            int new_top = compositor->grab_geobox.y;
            int new_bottom = compositor->grab_geobox.y +
                             compositor->grab_geobox.height;

            if (compositor->resize_edges & WLR_EDGE_TOP) {
                new_top = border_y;
                if (new_top >= new_bottom) new_top = new_bottom - 1;
            }
            if (compositor->resize_edges & WLR_EDGE_BOTTOM) {
                new_bottom = border_y;
                if (new_bottom <= new_top) new_bottom = new_top + 1;
            }
            if (compositor->resize_edges & WLR_EDGE_LEFT) {
                new_left = border_x;
                if (new_left >= new_right) new_left = new_right - 1;
            }
            if (compositor->resize_edges & WLR_EDGE_RIGHT) {
                new_right = border_x;
                if (new_right <= new_left) new_right = new_left + 1;
            }

            struct wlr_box geo_box;
            wlr_xdg_surface_get_geometry(
                view->xdg_toplevel->base, &geo_box);

            wlr_scene_node_set_position(&view->scene_tree->node,
                new_left - geo_box.x, new_top - geo_box.y);

            int new_width = new_right - new_left;
            int new_height = new_bottom - new_top;
            wlr_xdg_toplevel_set_size(view->xdg_toplevel,
                                       new_width, new_height);
        }
        return;
    }

    /* Normal mode — find the surface under the cursor */
    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct zenith_view *view = zenith_view_at(compositor,
        compositor->cursor->x, compositor->cursor->y, &surface, &sx, &sy);

    if (!view) {
        /* No window under cursor — set default cursor image */
        wlr_cursor_set_xcursor(compositor->cursor,
                                compositor->cursor_mgr, "default");
    }

    if (surface) {
        /* Send pointer enter/motion to the surface */
        wlr_seat_pointer_notify_enter(compositor->seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(compositor->seat, time, sx, sy);
    } else {
        wlr_seat_pointer_clear_focus(compositor->seat);
    }
}

static void handle_cursor_motion(struct wl_listener *listener, void *data) {
    struct zenith_compositor *compositor =
        wl_container_of(listener, compositor, cursor_motion);
    struct wlr_pointer_motion_event *event = data;

    wlr_cursor_move(compositor->cursor, &event->pointer->base,
                    event->delta_x, event->delta_y);
    process_cursor_motion(compositor, event->time_msec);
}

static void handle_cursor_motion_absolute(struct wl_listener *listener,
                                           void *data) {
    struct zenith_compositor *compositor =
        wl_container_of(listener, compositor, cursor_motion_absolute);
    struct wlr_pointer_motion_absolute_event *event = data;

    wlr_cursor_warp_absolute(compositor->cursor, &event->pointer->base,
                              event->x, event->y);
    process_cursor_motion(compositor, event->time_msec);
}

static void handle_cursor_button(struct wl_listener *listener, void *data) {
    struct zenith_compositor *compositor =
        wl_container_of(listener, compositor, cursor_button);
    struct wlr_pointer_button_event *event = data;

    wlr_seat_pointer_notify_button(compositor->seat,
        event->time_msec, event->button, event->state);

    if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
        /* End any interactive move/resize */
        if (compositor->cursor_mode != ZENITH_CURSOR_PASSTHROUGH) {
            compositor->cursor_mode = ZENITH_CURSOR_PASSTHROUGH;
            compositor->grabbed_view = NULL;
        }
    } else {
        /* Click to focus */
        double sx, sy;
        struct wlr_surface *surface = NULL;
        struct zenith_view *view = zenith_view_at(compositor,
            compositor->cursor->x, compositor->cursor->y,
            &surface, &sx, &sy);
        if (view) {
            zenith_compositor_focus_view(compositor, view, surface);
        }
    }
}

static void handle_cursor_axis(struct wl_listener *listener, void *data) {
    struct zenith_compositor *compositor =
        wl_container_of(listener, compositor, cursor_axis);
    struct wlr_pointer_axis_event *event = data;

    wlr_seat_pointer_notify_axis(compositor->seat,
        event->time_msec, event->orientation, event->delta,
        event->delta_discrete, event->source, event->relative_direction);
}

static void handle_cursor_frame(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_compositor *compositor =
        wl_container_of(listener, compositor, cursor_frame);
    wlr_seat_pointer_notify_frame(compositor->seat);
}

/* ========================================================================= */
/* Seat handlers                                                              */
/* ========================================================================= */

static void handle_request_cursor(struct wl_listener *listener, void *data) {
    struct zenith_compositor *compositor =
        wl_container_of(listener, compositor, request_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;

    struct wlr_seat_client *focused_client =
        compositor->seat->pointer_state.focused_client;
    if (focused_client == event->seat_client) {
        wlr_cursor_set_surface(compositor->cursor, event->surface,
                                event->hotspot_x, event->hotspot_y);
    }
}

static void handle_request_set_selection(struct wl_listener *listener,
                                          void *data) {
    struct zenith_compositor *compositor =
        wl_container_of(listener, compositor, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(compositor->seat, event->source, event->serial);
}

/* ========================================================================= */
/* Keyboard handling                                                          */
/* ========================================================================= */

static void handle_keyboard_modifiers(struct wl_listener *listener,
                                       void *data) {
    (void)data;
    struct zenith_keyboard *keyboard =
        wl_container_of(listener, keyboard, modifiers);

    wlr_seat_set_keyboard(keyboard->compositor->seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(keyboard->compositor->seat,
                                        &keyboard->wlr_keyboard->modifiers);
}

static void handle_keyboard_key(struct wl_listener *listener, void *data) {
    struct zenith_keyboard *keyboard =
        wl_container_of(listener, keyboard, key);
    struct wlr_keyboard_key_event *event = data;
    struct zenith_compositor *compositor = keyboard->compositor;

    /* Translate libinput keycode to xkbcommon */
    uint32_t keycode = event->keycode + 8;

    /* Get keysyms from the keyboard layout */
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(
        keyboard->wlr_keyboard->xkb_state, keycode, &syms);

    bool handled = false;

    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        /* Check for compositor keybindings */
        uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);
        for (int i = 0; i < nsyms; i++) {
            handled = zenith_keybinding_handle(compositor, modifiers, syms[i]);
            if (handled) break;
        }
    }

    if (!handled) {
        /* Pass the key to the focused client */
        wlr_seat_set_keyboard(compositor->seat, keyboard->wlr_keyboard);
        wlr_seat_keyboard_notify_key(compositor->seat, event->time_msec,
                                      event->keycode, event->state);
    }
}

static void handle_keyboard_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_keyboard *keyboard =
        wl_container_of(listener, keyboard, destroy);

    wl_list_remove(&keyboard->modifiers.link);
    wl_list_remove(&keyboard->key.link);
    wl_list_remove(&keyboard->destroy.link);
    wl_list_remove(&keyboard->link);
    free(keyboard);
}

static void setup_keyboard(struct zenith_compositor *compositor,
                            struct wlr_input_device *device) {
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

    struct zenith_keyboard *keyboard = calloc(1, sizeof(*keyboard));
    if (!keyboard) {
        wlr_log(WLR_ERROR, "Failed to allocate keyboard");
        return;
    }
    keyboard->compositor = compositor;
    keyboard->wlr_keyboard = wlr_keyboard;

    /* Set up keymap — use the user's default XKB layout */
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(
        context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    /* Listen for keyboard events */
    keyboard->modifiers.notify = handle_keyboard_modifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);

    keyboard->key.notify = handle_keyboard_key;
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);

    keyboard->destroy.notify = handle_keyboard_destroy;
    wl_signal_add(&device->events.destroy, &keyboard->destroy);

    wl_list_insert(&compositor->keyboards, &keyboard->link);

    wlr_seat_set_keyboard(compositor->seat, wlr_keyboard);
    wlr_log(WLR_INFO, "Keyboard configured");
}

/* ========================================================================= */
/* New input device handler                                                   */
/* ========================================================================= */

static void handle_new_input(struct wl_listener *listener, void *data) {
    struct zenith_compositor *compositor =
        wl_container_of(listener, compositor, new_input);
    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        wlr_log(WLR_INFO, "New keyboard: %s", device->name);
        setup_keyboard(compositor, device);
        break;

    case WLR_INPUT_DEVICE_POINTER:
        wlr_log(WLR_INFO, "New pointer: %s", device->name);
        wlr_cursor_attach_input_device(compositor->cursor, device);
        break;

    default:
        wlr_log(WLR_INFO, "New input device: %s (type %d)",
                device->name, device->type);
        break;
    }

    /* Update seat capabilities */
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&compositor->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(compositor->seat, caps);
}

/* ========================================================================= */
/* Input initialization                                                       */
/* ========================================================================= */

void zenith_input_init(struct zenith_compositor *compositor) {
    /* Create cursor */
    compositor->cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(compositor->cursor,
                                     compositor->output_layout);

    /* Create xcursor manager (cursor themes) */
    compositor->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);

    /* Cursor event listeners */
    compositor->cursor_motion.notify = handle_cursor_motion;
    wl_signal_add(&compositor->cursor->events.motion,
                  &compositor->cursor_motion);

    compositor->cursor_motion_absolute.notify = handle_cursor_motion_absolute;
    wl_signal_add(&compositor->cursor->events.motion_absolute,
                  &compositor->cursor_motion_absolute);

    compositor->cursor_button.notify = handle_cursor_button;
    wl_signal_add(&compositor->cursor->events.button,
                  &compositor->cursor_button);

    compositor->cursor_axis.notify = handle_cursor_axis;
    wl_signal_add(&compositor->cursor->events.axis,
                  &compositor->cursor_axis);

    compositor->cursor_frame.notify = handle_cursor_frame;
    wl_signal_add(&compositor->cursor->events.frame,
                  &compositor->cursor_frame);

    /* Create seat (represents a collection of input devices) */
    wl_list_init(&compositor->keyboards);
    compositor->seat = wlr_seat_create(compositor->wl_display, "seat0");

    compositor->request_cursor.notify = handle_request_cursor;
    wl_signal_add(&compositor->seat->events.request_set_cursor,
                  &compositor->request_cursor);

    compositor->request_set_selection.notify = handle_request_set_selection;
    wl_signal_add(&compositor->seat->events.request_set_selection,
                  &compositor->request_set_selection);

    /* Listen for new input devices */
    compositor->new_input.notify = handle_new_input;
    wl_signal_add(&compositor->backend->events.new_input,
                  &compositor->new_input);

    compositor->cursor_mode = ZENITH_CURSOR_PASSTHROUGH;

    wlr_log(WLR_INFO, "Input subsystem initialized");
}
