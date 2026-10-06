/*
 * Zenith OS — Compositor
 *
 * keybindings.c — Keyboard shortcut handling
 *
 * Default keybindings:
 *   Super + Enter         → Launch terminal (foot)
 *   Super + Q             → Close focused window
 *   Super + 1-4           → Switch to workspace 1-4
 *   Super + Shift + 1-4   → Move focused window to workspace 1-4
 *   Super + Shift + E     → Exit compositor
 *   Super + F             → Toggle maximize
 *   Alt + F4              → Close focused window
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include <unistd.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

#include "compositor.h"
#include "keybindings.h"
#include "workspace.h"

#define MOD_SUPER WLR_MODIFIER_LOGO
#define MOD_SHIFT WLR_MODIFIER_SHIFT
#define MOD_ALT   WLR_MODIFIER_ALT

/**
 * Launch a program in a child process.
 */
static void launch_program(const char *cmd) {
    if (fork() == 0) {
        setsid();
        execl("/bin/sh", "/bin/sh", "-c", cmd, (char *)NULL);
        _exit(EXIT_FAILURE);
    }
}

/**
 * Get the currently focused view, if any.
 */
static struct zenith_view *get_focused_view(
        struct zenith_compositor *compositor) {
    int ws = compositor->active_workspace;
    if (wl_list_empty(&compositor->workspaces[ws].views)) {
        return NULL;
    }
    struct zenith_view *view;
    wl_list_for_each(view, &compositor->workspaces[ws].views, link) {
        return view;  /* First in list = focused */
    }
    return NULL;
}

bool zenith_keybinding_handle(struct zenith_compositor *compositor,
                               uint32_t modifiers, xkb_keysym_t sym) {

    /*
     * Super + D or Super + Space → Launch application launcher
     */
    if ((modifiers & MOD_SUPER) && (sym == XKB_KEY_d || sym == XKB_KEY_space)) {
        launch_program("zenith-launcher");
        return true;
    }

    /*
     * Super + Enter → Launch terminal
     */
    if ((modifiers & MOD_SUPER) && sym == XKB_KEY_Return) {
        launch_program("foot");
        return true;
    }

    /*
     * Super + Q → Close focused window
     */
    if ((modifiers & MOD_SUPER) && sym == XKB_KEY_q) {
        struct zenith_view *view = get_focused_view(compositor);
        if (view) {
            wlr_xdg_toplevel_send_close(view->xdg_toplevel);
        }
        return true;
    }

    /*
     * Alt + F4 → Close focused window
     */
    if ((modifiers & MOD_ALT) && sym == XKB_KEY_F4) {
        struct zenith_view *view = get_focused_view(compositor);
        if (view) {
            wlr_xdg_toplevel_send_close(view->xdg_toplevel);
        }
        return true;
    }

    /*
     * Super + F → Toggle maximize
     */
    if ((modifiers & MOD_SUPER) && sym == XKB_KEY_f) {
        struct zenith_view *view = get_focused_view(compositor);
        if (view) {
            /* Trigger the maximize request handler */
            view->request_maximize.notify(&view->request_maximize, NULL);
        }
        return true;
    }

    /*
     * Super + 1-4 → Switch workspace
     */
    if (modifiers == MOD_SUPER) {
        int ws = -1;
        switch (sym) {
        case XKB_KEY_1: ws = 0; break;
        case XKB_KEY_2: ws = 1; break;
        case XKB_KEY_3: ws = 2; break;
        case XKB_KEY_4: ws = 3; break;
        }
        if (ws >= 0) {
            zenith_workspace_switch(compositor, ws);
            return true;
        }
    }

    /*
     * Super + Shift + 1-4 → Move focused window to workspace
     */
    if ((modifiers & MOD_SUPER) && (modifiers & MOD_SHIFT)) {
        int ws = -1;
        switch (sym) {
        case XKB_KEY_exclam:      ws = 0; break;  /* Shift+1 */
        case XKB_KEY_at:          ws = 1; break;  /* Shift+2 */
        case XKB_KEY_numbersign:  ws = 2; break;  /* Shift+3 */
        case XKB_KEY_dollar:      ws = 3; break;  /* Shift+4 */
        }
        if (ws >= 0) {
            struct zenith_view *view = get_focused_view(compositor);
            if (view) {
                zenith_workspace_move_view(compositor, view, ws);
            }
            return true;
        }
    }

    /*
     * Super + Shift + E → Exit compositor
     */
    if ((modifiers & MOD_SUPER) && (modifiers & MOD_SHIFT) &&
        sym == XKB_KEY_E) {
        wlr_log(WLR_INFO, "Exit keybinding pressed");
        wl_display_terminate(compositor->wl_display);
        return true;
    }

    return false;
}
