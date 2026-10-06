/*
 * Zenith OS — Panel
 *
 * panel.h — Top panel data structures
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ZENITH_PANEL_H
#define ZENITH_PANEL_H

#include <gtk/gtk.h>

/**
 * The panel is a layer-shell surface anchored to the top of the screen.
 * It contains a workspace indicator (left), center area, and
 * system indicators (right: clock, network, etc.)
 */
struct zenith_panel {
    GtkApplication *app;
    GtkWindow *window;
    GtkWidget *box;          /* Main horizontal box */
    GtkWidget *left_box;     /* Left section (workspace indicator) */
    GtkWidget *center_box;   /* Center section */
    GtkWidget *right_box;    /* Right section (clock, indicators) */
};

/**
 * Create and configure the panel window as a layer-shell surface.
 */
void zenith_panel_activate(GtkApplication *app, gpointer user_data);

/**
 * Create the clock widget.
 * Returns a GtkLabel that updates every second.
 */
GtkWidget *zenith_clock_create(void);

/**
 * Create the workspace indicator widget.
 * Returns a GtkBox with workspace buttons.
 */
GtkWidget *zenith_workspace_indicator_create(void);

#endif /* ZENITH_PANEL_H */
