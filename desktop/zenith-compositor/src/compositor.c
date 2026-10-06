/*
 * Zenith OS — Compositor
 *
 * compositor.c — Core compositor initialization and lifecycle
 *
 * This sets up the Wayland display, wlroots backend, renderer,
 * scene graph, XDG shell, layer shell, seat, and input/output handlers.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define _POSIX_C_SOURCE 200112L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "compositor.h"
#include "output.h"
#include "input.h"
#include "view.h"
#include "keybindings.h"

/* ========================================================================= */
/* XDG Shell handlers                                                        */
/* ========================================================================= */

/**
 * Called when a new XDG toplevel surface is created (a new window).
 */
static void handle_new_xdg_toplevel(struct wl_listener *listener, void *data) {
    struct zenith_compositor *compositor =
        wl_container_of(listener, compositor, new_xdg_toplevel);
    struct wlr_xdg_toplevel *toplevel = data;

    zenith_view_create(compositor, toplevel);
}

/**
 * Called when a new XDG popup surface is created.
 * We let the scene graph handle popups automatically.
 */
static void handle_new_xdg_popup(struct wl_listener *listener, void *data) {
    (void)listener;
    struct wlr_xdg_popup *popup = data;

    /* Create a scene tree for the popup, parented to the popup's parent */
    struct wlr_xdg_surface *parent =
        wlr_xdg_surface_try_from_wlr_surface(popup->parent);
    if (parent == NULL) {
        return;
    }
    struct wlr_scene_tree *parent_tree = parent->data;
    if (parent_tree == NULL) {
        return;
    }
    popup->base->data =
        wlr_scene_xdg_surface_create(parent_tree, popup->base);
}

/* ========================================================================= */
/* Compositor init / run / destroy                                            */
/* ========================================================================= */

bool zenith_compositor_init(struct zenith_compositor *compositor) {
    wlr_log(WLR_INFO, "Initializing Zenith compositor...");

    /* Create Wayland display */
    compositor->wl_display = wl_display_create();
    if (!compositor->wl_display) {
        wlr_log(WLR_ERROR, "Failed to create Wayland display");
        return false;
    }
    compositor->wl_event_loop = wl_display_get_event_loop(compositor->wl_display);

    /* Create wlroots backend — auto-detects DRM, headless, etc. */
    compositor->backend = wlr_backend_autocreate(
        compositor->wl_event_loop, NULL);
    if (!compositor->backend) {
        wlr_log(WLR_ERROR, "Failed to create wlroots backend");
        goto err_display;
    }

    /* Create renderer */
    compositor->renderer = wlr_renderer_autocreate(compositor->backend);
    if (!compositor->renderer) {
        wlr_log(WLR_ERROR, "Failed to create renderer");
        goto err_backend;
    }
    wlr_renderer_init_wl_display(compositor->renderer, compositor->wl_display);

    /* Create allocator */
    compositor->allocator = wlr_allocator_autocreate(
        compositor->backend, compositor->renderer);
    if (!compositor->allocator) {
        wlr_log(WLR_ERROR, "Failed to create allocator");
        goto err_renderer;
    }

    /* Create wlr_compositor — manages surfaces and regions */
    compositor->wlr_compositor = wlr_compositor_create(
        compositor->wl_display, 5, compositor->renderer);
    wlr_subcompositor_create(compositor->wl_display);
    wlr_data_device_manager_create(compositor->wl_display);

    /* ===== Scene graph ===== */
    compositor->scene = wlr_scene_create();
    compositor->output_layout = wlr_output_layout_create(compositor->wl_display);
    compositor->scene_layout = wlr_scene_attach_output_layout(
        compositor->scene, compositor->output_layout);

    /* Initialize workspaces — each gets a scene tree node */
    for (int i = 0; i < ZENITH_NUM_WORKSPACES; i++) {
        char name[64];
        snprintf(name, sizeof(name), "Workspace %d", i + 1);
        zenith_workspace_init(&compositor->workspaces[i], i, name,
                              &compositor->scene->tree);
    }
    compositor->active_workspace = 0;
    /* Show only the active workspace */
    for (int i = 0; i < ZENITH_NUM_WORKSPACES; i++) {
        wlr_scene_node_set_enabled(
            &compositor->workspaces[i].scene_tree->node,
            i == compositor->active_workspace);
    }

    /* ===== XDG Shell ===== */
    compositor->xdg_shell = wlr_xdg_shell_create(compositor->wl_display, 3);
    compositor->new_xdg_toplevel.notify = handle_new_xdg_toplevel;
    wl_signal_add(&compositor->xdg_shell->events.new_toplevel,
                  &compositor->new_xdg_toplevel);
    compositor->new_xdg_popup.notify = handle_new_xdg_popup;
    wl_signal_add(&compositor->xdg_shell->events.new_popup,
                  &compositor->new_xdg_popup);

    /* ===== Layer Shell (for panels, docks) ===== */
    compositor->layer_shell = wlr_layer_shell_v1_create(
        compositor->wl_display, 4);
    /* Layer shell handler will be implemented when panel/dock are ready */

    /* ===== Outputs ===== */
    wl_list_init(&compositor->outputs);
    compositor->new_output.notify = zenith_output_new;
    wl_signal_add(&compositor->backend->events.new_output,
                  &compositor->new_output);

    /* ===== Input (cursor + keyboard) ===== */
    zenith_input_init(compositor);

    /* ===== Add Wayland socket ===== */
    compositor->socket = wl_display_add_socket_auto(compositor->wl_display);
    if (!compositor->socket) {
        wlr_log(WLR_ERROR, "Failed to create Wayland socket");
        goto err_allocator;
    }

    /* Set WAYLAND_DISPLAY for child processes */
    setenv("WAYLAND_DISPLAY", compositor->socket, true);
    wlr_log(WLR_INFO, "Wayland socket: %s", compositor->socket);

    /* Start the backend — this begins output detection and input */
    if (!wlr_backend_start(compositor->backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        goto err_allocator;
    }

    wlr_log(WLR_INFO, "Compositor initialized successfully");
    return true;

err_allocator:
    wlr_allocator_destroy(compositor->allocator);
err_renderer:
    wlr_renderer_destroy(compositor->renderer);
err_backend:
    wlr_backend_destroy(compositor->backend);
err_display:
    wl_display_destroy(compositor->wl_display);
    return false;
}

void zenith_compositor_run(struct zenith_compositor *compositor) {
    wl_display_run(compositor->wl_display);
}

void zenith_compositor_destroy(struct zenith_compositor *compositor) {
    wl_display_destroy_clients(compositor->wl_display);

    wlr_scene_node_destroy(&compositor->scene->tree.node);
    wlr_xcursor_manager_destroy(compositor->cursor_mgr);
    wlr_cursor_destroy(compositor->cursor);
    wlr_allocator_destroy(compositor->allocator);
    wlr_renderer_destroy(compositor->renderer);
    wlr_backend_destroy(compositor->backend);
    wl_display_destroy(compositor->wl_display);
}

/**
 * Focus a view — bring it to the top, set keyboard focus.
 */
void zenith_compositor_focus_view(struct zenith_compositor *compositor,
                                  struct zenith_view *view,
                                  struct wlr_surface *surface) {
    if (view == NULL) {
        return;
    }

    struct wlr_seat *seat = compositor->seat;
    struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;

    if (prev_surface == surface) {
        /* Already focused */
        return;
    }

    /* Deactivate previous toplevel */
    if (prev_surface) {
        struct wlr_xdg_toplevel *prev_toplevel =
            wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
        if (prev_toplevel != NULL) {
            wlr_xdg_toplevel_set_activated(prev_toplevel, false);
        }
    }

    /* Move view to the front of the scene */
    wlr_scene_node_raise_to_top(&view->scene_tree->node);

    /* Move view to front of its workspace's view list */
    wl_list_remove(&view->link);
    wl_list_insert(
        &compositor->workspaces[view->workspace_index].views,
        &view->link);

    /* Activate the new toplevel */
    wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);

    /* Set keyboard focus */
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    if (keyboard != NULL) {
        wlr_seat_keyboard_notify_enter(seat, view->xdg_toplevel->base->surface,
            keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
    }
}
