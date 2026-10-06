/*
 * Zenith OS — Compositor
 *
 * compositor.h — Core compositor data structures and API
 *
 * The compositor manages the Wayland display, wlroots backend, renderer,
 * outputs, input devices, views (windows), and workspaces.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ZENITH_COMPOSITOR_H
#define ZENITH_COMPOSITOR_H

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>

#include "workspace.h"

#define ZENITH_NUM_WORKSPACES 4

/**
 * Cursor mode determines how pointer motion is interpreted.
 */
enum zenith_cursor_mode {
    ZENITH_CURSOR_PASSTHROUGH = 0,  /* Normal — events go to surface */
    ZENITH_CURSOR_MOVE,             /* Moving a window */
    ZENITH_CURSOR_RESIZE,           /* Resizing a window */
};

/**
 * A Zenith output represents a physical display/monitor.
 */
struct zenith_output {
    struct wl_list link;             /* zenith_compositor.outputs */
    struct zenith_compositor *compositor;
    struct wlr_output *wlr_output;
    struct wlr_scene_output *scene_output;

    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;
};

/**
 * A Zenith view represents a toplevel window (xdg-shell surface).
 */
struct zenith_view {
    struct wl_list link;             /* zenith_workspace.views */
    struct zenith_compositor *compositor;
    struct wlr_xdg_toplevel *xdg_toplevel;
    struct wlr_scene_tree *scene_tree;

    /* Which workspace this view belongs to */
    int workspace_index;

    /* Saved geometry for maximize/restore */
    bool maximized;
    struct wlr_box saved_geometry;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
};

/**
 * A Zenith keyboard wraps a wlr_keyboard with its listeners.
 */
struct zenith_keyboard {
    struct wl_list link;             /* zenith_compositor.keyboards */
    struct zenith_compositor *compositor;
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

/**
 * The main compositor server. Owns all state.
 */
struct zenith_compositor {
    /* Wayland core */
    struct wl_display *wl_display;
    struct wl_event_loop *wl_event_loop;

    /* wlroots backend and rendering */
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_compositor *wlr_compositor;

    /* Scene graph — handles rendering order automatically */
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;

    /* Output layout — manages multi-monitor geometry */
    struct wlr_output_layout *output_layout;
    struct wl_list outputs;          /* zenith_output.link */
    struct wl_listener new_output;

    /* Input */
    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;
    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;

    struct wlr_seat *seat;
    struct wl_list keyboards;       /* zenith_keyboard.link */
    struct wl_listener new_input;
    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;

    /* XDG shell — manages toplevel windows */
    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_toplevel;
    struct wl_listener new_xdg_popup;

    /* Layer shell — manages panels, docks, overlays */
    struct wlr_layer_shell_v1 *layer_shell;
    struct wl_listener new_layer_surface;

    /* Workspaces */
    struct zenith_workspace workspaces[ZENITH_NUM_WORKSPACES];
    int active_workspace;

    /* Cursor interactive state */
    enum zenith_cursor_mode cursor_mode;
    struct zenith_view *grabbed_view;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;

    /* Socket name for clients */
    const char *socket;
};

/* compositor.c */
bool zenith_compositor_init(struct zenith_compositor *compositor);
void zenith_compositor_run(struct zenith_compositor *compositor);
void zenith_compositor_destroy(struct zenith_compositor *compositor);
void zenith_compositor_focus_view(struct zenith_compositor *compositor,
                                  struct zenith_view *view,
                                  struct wlr_surface *surface);

/* view.c */
struct zenith_view *zenith_view_at(struct zenith_compositor *compositor,
                                    double lx, double ly,
                                    struct wlr_surface **surface,
                                    double *sx, double *sy);

#endif /* ZENITH_COMPOSITOR_H */
