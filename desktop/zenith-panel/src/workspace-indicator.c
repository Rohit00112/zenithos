/*
 * Zenith OS — Panel
 *
 * workspace-indicator.c — Workspace switcher widget
 *
 * Shows numbered buttons for each workspace. Clicking a button
 * switches to that workspace (via D-Bus or compositor IPC).
 *
 * For Phase 1, this is a visual indicator only. Full IPC with
 * the compositor will be implemented in Phase 2.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include "panel.h"

#define NUM_WORKSPACES 4

static GtkWidget *workspace_buttons[NUM_WORKSPACES];
static int current_workspace = 0;

/**
 * Update the visual state of workspace buttons.
 */
static void update_workspace_buttons(void) {
    for (int i = 0; i < NUM_WORKSPACES; i++) {
        if (i == current_workspace) {
            gtk_widget_add_css_class(workspace_buttons[i], "active");
        } else {
            gtk_widget_remove_css_class(workspace_buttons[i], "active");
        }
    }
}

/**
 * Called when a workspace button is clicked.
 */
static void on_workspace_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    int index = GPOINTER_TO_INT(user_data);

    current_workspace = index;
    update_workspace_buttons();

    /*
     * TODO (Phase 2): Send workspace switch command to compositor
     * via a Wayland protocol or D-Bus interface.
     *
     * For now, this only updates the visual indicator.
     */
}

GtkWidget *zenith_workspace_indicator_create(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

    for (int i = 0; i < NUM_WORKSPACES; i++) {
        char label[8];
        snprintf(label, sizeof(label), "%d", i + 1);

        workspace_buttons[i] = gtk_button_new_with_label(label);
        gtk_widget_add_css_class(workspace_buttons[i], "workspace-button");
        gtk_widget_set_focusable(workspace_buttons[i], FALSE);

        g_signal_connect(workspace_buttons[i], "clicked",
                         G_CALLBACK(on_workspace_clicked),
                         GINT_TO_POINTER(i));

        gtk_box_append(GTK_BOX(box), workspace_buttons[i]);
    }

    /* Set initial state */
    update_workspace_buttons();

    return box;
}
