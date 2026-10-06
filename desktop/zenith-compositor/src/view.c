/*
 * Zenith OS — Compositor
 *
 * view.c — View (window) management
 *
 * A "view" wraps an XDG toplevel surface and manages its lifecycle,
 * scene graph node, workspace membership, and interactive operations.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "compositor.h"
#include "view.h"

/* ========================================================================= */
/* XDG toplevel event handlers                                                */
/* ========================================================================= */

/**
 * Called when the surface is mapped (ready to be displayed).
 */
static void handle_xdg_toplevel_map(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, map);

    /* Add to the active workspace's view list */
    wl_list_insert(
        &view->compositor->workspaces[view->workspace_index].views,
        &view->link);

    /* Focus the new window */
    zenith_compositor_focus_view(view->compositor, view,
                                 view->xdg_toplevel->base->surface);

    wlr_log(WLR_INFO, "View mapped: %s",
            view->xdg_toplevel->title ? view->xdg_toplevel->title : "(untitled)");
}

/**
 * Called when the surface is unmapped (hidden/closed).
 */
static void handle_xdg_toplevel_unmap(struct wl_listener *listener,
                                       void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, unmap);

    /* Remove from workspace view list */
    wl_list_remove(&view->link);

    /* If this was the grabbed view, release it */
    if (view->compositor->grabbed_view == view) {
        view->compositor->cursor_mode = ZENITH_CURSOR_PASSTHROUGH;
        view->compositor->grabbed_view = NULL;
    }

    wlr_log(WLR_INFO, "View unmapped: %s",
            view->xdg_toplevel->title ? view->xdg_toplevel->title : "(untitled)");
}

/**
 * Called when the surface commits a new frame.
 */
static void handle_xdg_toplevel_commit(struct wl_listener *listener,
                                        void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, commit);

    if (view->xdg_toplevel->base->initial_commit) {
        /* First commit — configure the toplevel */
        wlr_xdg_toplevel_set_size(view->xdg_toplevel, 0, 0);
    }
}

/**
 * Called when the toplevel is destroyed.
 */
static void handle_xdg_toplevel_destroy(struct wl_listener *listener,
                                         void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, destroy);

    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->commit.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->request_move.link);
    wl_list_remove(&view->request_resize.link);
    wl_list_remove(&view->request_maximize.link);
    wl_list_remove(&view->request_fullscreen.link);

    free(view);
}

/**
 * Called when the client requests an interactive move.
 */
static void handle_xdg_toplevel_request_move(struct wl_listener *listener,
                                              void *data) {
    (void)data;
    struct zenith_view *view = wl_container_of(listener, view, request_move);
    zenith_view_begin_move(view->compositor, view);
}

/**
 * Called when the client requests an interactive resize.
 */
static void handle_xdg_toplevel_request_resize(struct wl_listener *listener,
                                                void *data) {
    struct zenith_view *view = wl_container_of(listener, view, request_resize);
    struct wlr_xdg_toplevel_resize_event *event = data;
    zenith_view_begin_resize(view->compositor, view, event->edges);
}

/**
 * Called when the client requests maximize/unmaximize.
 */
static void handle_xdg_toplevel_request_maximize(struct wl_listener *listener,
                                                   void *data) {
    (void)data;
    struct zenith_view *view =
        wl_container_of(listener, view, request_maximize);

    if (!view->xdg_toplevel->base->surface->mapped) {
        return;
    }

    if (!view->maximized) {
        /* Save current geometry for later restore */
        struct wlr_box geo;
        wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo);
        view->saved_geometry.x = view->scene_tree->node.x;
        view->saved_geometry.y = view->scene_tree->node.y;
        view->saved_geometry.width = geo.width;
        view->saved_geometry.height = geo.height;

        /* Get the output under the view */
        struct wlr_output *output = wlr_output_layout_output_at(
            view->compositor->output_layout,
            view->scene_tree->node.x + geo.width / 2.0,
            view->scene_tree->node.y + geo.height / 2.0);

        if (output) {
            struct wlr_box output_box;
            wlr_output_layout_get_box(view->compositor->output_layout,
                                       output, &output_box);
            /* Reserve space for panel (32px top) */
            wlr_scene_node_set_position(&view->scene_tree->node,
                                         output_box.x, output_box.y + 32);
            wlr_xdg_toplevel_set_size(view->xdg_toplevel,
                                       output_box.width,
                                       output_box.height - 32);
        }

        view->maximized = true;
    } else {
        /* Restore saved geometry */
        wlr_scene_node_set_position(&view->scene_tree->node,
                                     view->saved_geometry.x,
                                     view->saved_geometry.y);
        wlr_xdg_toplevel_set_size(view->xdg_toplevel,
                                   view->saved_geometry.width,
                                   view->saved_geometry.height);
        view->maximized = false;
    }

    wlr_xdg_toplevel_set_maximized(view->xdg_toplevel, view->maximized);
    wlr_xdg_surface_schedule_configure(view->xdg_toplevel->base);
}

/**
 * Called when the client requests fullscreen.
 */
static void handle_xdg_toplevel_request_fullscreen(
        struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_view *view =
        wl_container_of(listener, view, request_fullscreen);

    /* For now, just acknowledge the request. Full implementation later. */
    if (view->xdg_toplevel->base->surface->mapped) {
        wlr_xdg_toplevel_set_fullscreen(view->xdg_toplevel,
            view->xdg_toplevel->requested.fullscreen);
        wlr_xdg_surface_schedule_configure(view->xdg_toplevel->base);
    }
}

/* ========================================================================= */
/* View creation and lookup                                                   */
/* ========================================================================= */

void zenith_view_create(struct zenith_compositor *compositor,
                         struct wlr_xdg_toplevel *toplevel) {
    struct zenith_view *view = calloc(1, sizeof(*view));
    if (!view) {
        wlr_log(WLR_ERROR, "Failed to allocate view");
        return;
    }

    view->compositor = compositor;
    view->xdg_toplevel = toplevel;
    view->workspace_index = compositor->active_workspace;
    view->maximized = false;

    /* Create scene tree node for this view, parented to its workspace */
    view->scene_tree = wlr_scene_xdg_surface_create(
        compositor->workspaces[view->workspace_index].scene_tree,
        toplevel->base);
    view->scene_tree->node.data = view;
    toplevel->base->data = view->scene_tree;

    /* Listen for toplevel events */
    view->map.notify = handle_xdg_toplevel_map;
    wl_signal_add(&toplevel->base->surface->events.map, &view->map);

    view->unmap.notify = handle_xdg_toplevel_unmap;
    wl_signal_add(&toplevel->base->surface->events.unmap, &view->unmap);

    view->commit.notify = handle_xdg_toplevel_commit;
    wl_signal_add(&toplevel->base->surface->events.commit, &view->commit);

    view->destroy.notify = handle_xdg_toplevel_destroy;
    wl_signal_add(&toplevel->events.destroy, &view->destroy);

    view->request_move.notify = handle_xdg_toplevel_request_move;
    wl_signal_add(&toplevel->events.request_move, &view->request_move);

    view->request_resize.notify = handle_xdg_toplevel_request_resize;
    wl_signal_add(&toplevel->events.request_resize, &view->request_resize);

    view->request_maximize.notify = handle_xdg_toplevel_request_maximize;
    wl_signal_add(&toplevel->events.request_maximize,
                  &view->request_maximize);

    view->request_fullscreen.notify = handle_xdg_toplevel_request_fullscreen;
    wl_signal_add(&toplevel->events.request_fullscreen,
                  &view->request_fullscreen);
}

struct zenith_view *zenith_view_at(struct zenith_compositor *compositor,
                                    double lx, double ly,
                                    struct wlr_surface **surface,
                                    double *sx, double *sy) {
    /* Use the scene graph to find the node at these coordinates */
    struct wlr_scene_node *node =
        wlr_scene_node_at(&compositor->scene->tree.node, lx, ly, sx, sy);

    if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
        return NULL;
    }

    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
    struct wlr_scene_surface *scene_surface =
        wlr_scene_surface_try_from_buffer(scene_buffer);
    if (!scene_surface) {
        return NULL;
    }
    *surface = scene_surface->surface;

    /* Walk up the scene tree to find the view's root node */
    struct wlr_scene_tree *tree = node->parent;
    while (tree != NULL && tree->node.data == NULL) {
        tree = tree->node.parent;
    }

    if (tree == NULL) {
        return NULL;
    }
    return tree->node.data;
}

void zenith_view_begin_move(struct zenith_compositor *compositor,
                             struct zenith_view *view) {
    if (view->maximized) {
        return; /* Don't move maximized windows */
    }
    compositor->grabbed_view = view;
    compositor->cursor_mode = ZENITH_CURSOR_MOVE;
    compositor->grab_x = compositor->cursor->x - view->scene_tree->node.x;
    compositor->grab_y = compositor->cursor->y - view->scene_tree->node.y;
}

void zenith_view_begin_resize(struct zenith_compositor *compositor,
                               struct zenith_view *view, uint32_t edges) {
    if (view->maximized) {
        return; /* Don't resize maximized windows */
    }
    compositor->grabbed_view = view;
    compositor->cursor_mode = ZENITH_CURSOR_RESIZE;
    compositor->resize_edges = edges;

    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);

    double border_x = (view->scene_tree->node.x + geo_box.x) +
        ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
    double border_y = (view->scene_tree->node.y + geo_box.y) +
        ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
    compositor->grab_x = compositor->cursor->x - border_x;
    compositor->grab_y = compositor->cursor->y - border_y;

    compositor->grab_geobox = geo_box;
    compositor->grab_geobox.x += view->scene_tree->node.x;
    compositor->grab_geobox.y += view->scene_tree->node.y;
}
