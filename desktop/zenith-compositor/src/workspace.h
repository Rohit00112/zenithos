/*
 * Zenith OS — Compositor
 *
 * workspace.h — Virtual workspace management
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ZENITH_WORKSPACE_H
#define ZENITH_WORKSPACE_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_scene.h>

/**
 * A workspace contains a list of views and a scene tree node
 * that can be shown/hidden to switch workspaces.
 */
struct zenith_workspace {
    int index;
    char name[64];
    struct wl_list views;            /* zenith_view.link */
    struct wlr_scene_tree *scene_tree;
};

/* Forward declaration */
struct zenith_compositor;

void zenith_workspace_init(struct zenith_workspace *ws, int index,
                           const char *name, struct wlr_scene_tree *parent);
void zenith_workspace_switch(struct zenith_compositor *compositor, int index);
void zenith_workspace_move_view(struct zenith_compositor *compositor,
                                struct zenith_view *view, int target_ws);

#endif /* ZENITH_WORKSPACE_H */
