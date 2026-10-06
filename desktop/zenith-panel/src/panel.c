/*
 * Zenith OS — Panel
 *
 * panel.c — Panel layout and layer-shell configuration
 *
 * The panel is 32px tall, anchored to the top edge of the screen.
 * Layout: [workspace-indicator] ... [clock]
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

#include "panel.h"

#define PANEL_HEIGHT 32

/**
 * Apply the Zenith panel CSS styling.
 */
static void apply_panel_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();

    const char *css =
        "window.zenith-panel {"
        "  background-color: rgba(13, 17, 23, 0.92);"
        "  color: #e6edf3;"
        "  font-family: 'Inter', 'Cantarell', sans-serif;"
        "  font-size: 13px;"
        "  font-weight: 500;"
        "}"
        ".workspace-button {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 4px;"
        "  color: #8b949e;"
        "  padding: 2px 10px;"
        "  margin: 2px 1px;"
        "  min-height: 24px;"
        "  font-weight: 500;"
        "  transition: all 150ms ease;"
        "}"
        ".workspace-button:hover {"
        "  background-color: rgba(88, 166, 255, 0.15);"
        "  color: #e6edf3;"
        "}"
        ".workspace-button.active {"
        "  background-color: rgba(88, 166, 255, 0.25);"
        "  color: #58a6ff;"
        "}"
        ".panel-clock {"
        "  color: #e6edf3;"
        "  font-weight: 600;"
        "  padding: 0 12px;"
        "  font-variant-numeric: tabular-nums;"
        "}"
        ".panel-section {"
        "  padding: 0 8px;"
        "}"
        ".panel-separator {"
        "  background-color: #30363d;"
        "  min-width: 1px;"
        "  margin: 6px 4px;"
        "}";

    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

void zenith_panel_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    apply_panel_css();

    /* Create the panel window */
    GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_widget_add_css_class(GTK_WIDGET(window), "zenith-panel");

    /* Configure as a layer-shell surface */
    gtk_layer_init_for_window(window);
    gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_namespace(window, "zenith-panel");

    /* Anchor to top edge, left and right (full width) */
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    /* Set exclusive zone so windows don't overlap the panel */
    gtk_layer_set_exclusive_zone(window, PANEL_HEIGHT);

    /* Set panel size */
    gtk_widget_set_size_request(GTK_WIDGET(window), -1, PANEL_HEIGHT);

    /* Create main horizontal layout */
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_window_set_child(window, box);

    /* Left section: workspace indicator */
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(left_box, "panel-section");
    gtk_widget_set_halign(left_box, GTK_ALIGN_START);
    gtk_widget_set_hexpand(left_box, FALSE);
    gtk_box_append(GTK_BOX(box), left_box);

    GtkWidget *workspace_indicator = zenith_workspace_indicator_create();
    gtk_box_append(GTK_BOX(left_box), workspace_indicator);

    /* Separator */
    GtkWidget *sep1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class(sep1, "panel-separator");
    gtk_box_append(GTK_BOX(box), sep1);

    /* Center section (spacer for now) */
    GtkWidget *center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(center_box, TRUE);
    gtk_box_append(GTK_BOX(box), center_box);

    /* Right section: clock */
    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(right_box, "panel-section");
    gtk_widget_set_halign(right_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(box), right_box);

    GtkWidget *clock = zenith_clock_create();
    gtk_box_append(GTK_BOX(right_box), clock);

    /* Show the panel */
    gtk_window_present(window);
}
