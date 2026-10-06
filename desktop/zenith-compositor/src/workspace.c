/*
 * Zenith OS — Compositor
 *
 * workspace.c — Virtual workspace management
 *
 * Each workspace has its own scene tree node. Switching workspaces
 * enables/disables the corresponding scene trees, making views
 * on inactive workspaces invisible.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <string.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

#include "compositor.h"
#include "workspace.h"

void zenith_workspace_init(struct zenith_workspace *ws, int index,
                           const char *name, struct wlr_scene_tree *parent) {
    ws->index = index;
    strncpy(ws->name, name, sizeof(ws->name) - 1);
    ws->name[sizeof(ws->name) - 1] = '\0';
    wl_list_init(&ws->views);

    /* Create a scene tree node for this workspace */
    ws->scene_tree = wlr_scene_tree_create(parent);
}

void zenith_workspace_switch(struct zenith_compositor *compositor, int index) {
    if (index < 0 || index >= ZENITH_NUM_WORKSPACES) {
        return;
    }
    if (index == compositor->active_workspace) {
        return;
    }

    wlr_log(WLR_INFO, "Switching to workspace %d (%s)",
            index + 1, compositor->workspaces[index].name);

    /* Hide the old workspace */
    wlr_scene_node_set_enabled(
        &compositor->workspaces[compositor->active_workspace].scene_tree->node,
        false);

    /* Show the new workspace */
    compositor->active_workspace = index;
    wlr_scene_node_set_enabled(
        &compositor->workspaces[index].scene_tree->node, true);

    /* Focus the top view in the new workspace, if any */
    struct zenith_view *view;
    wl_list_for_each(view, &compositor->workspaces[index].views, link) {
        zenith_compositor_focus_view(compositor, view,
                                     view->xdg_toplevel->base->surface);
        break;  /* Focus only the first (topmost) view */
    }

    /* If no views, clear keyboard focus */
    if (wl_list_empty(&compositor->workspaces[index].views)) {
        wlr_seat_keyboard_clear_focus(compositor->seat);
    }
}

void zenith_workspace_move_view(struct zenith_compositor *compositor,
                                struct zenith_view *view, int target_ws) {
    if (target_ws < 0 || target_ws >= ZENITH_NUM_WORKSPACES) {
        return;
    }
    if (target_ws == view->workspace_index) {
        return;
    }

    wlr_log(WLR_INFO, "Moving view '%s' to workspace %d",
            view->xdg_toplevel->title ? view->xdg_toplevel->title : "(untitled)",
            target_ws + 1);

    /* Remove from current workspace view list */
    wl_list_remove(&view->link);

    /* Re-parent the scene tree node to the target workspace */
    wlr_scene_node_reparent(&view->scene_tree->node,
                             compositor->workspaces[target_ws].scene_tree);

    /* Add to target workspace view list */
    view->workspace_index = target_ws;
    wl_list_insert(&compositor->workspaces[target_ws].views, &view->link);

    /* If the target workspace is not active, the view will be hidden
       automatically because the target workspace's scene tree is disabled */
}
