/*
 * Zenith OS — Dock
 *
 * dock.c — Dock widget and layout
 *
 * Sets up the dock at the bottom of the screen.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

#include "dock.h"

#define DOCK_HEIGHT 48

static void apply_dock_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();

    const char *css =
        "window.zenith-dock {"
        "  background-color: rgba(13, 17, 23, 0.85);"
        "  border: 1px solid rgba(48, 54, 61, 0.8);"
        "  border-radius: 12px 12px 0 0;"
        "  margin-bottom: 0;"
        "}"
        ".dock-icon {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 6px;"
        "  margin: 4px;"
        "  min-width: 36px;"
        "  min-height: 36px;"
        "  transition: all 200ms ease;"
        "}"
        ".dock-icon:hover {"
        "  background-color: rgba(255, 255, 255, 0.1);"
        "}";

    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

static GtkWidget* create_dock_icon(const char* icon_name, const char* tooltip) {
    GtkWidget *button = gtk_button_new();
    GtkWidget *icon = gtk_image_new_from_icon_name(icon_name);

    gtk_image_set_pixel_size(GTK_IMAGE(icon), 24);
    gtk_button_set_child(GTK_BUTTON(button), icon);

    gtk_widget_add_css_class(button, "dock-icon");
    gtk_widget_set_tooltip_text(button, tooltip);
    gtk_widget_set_focusable(button, FALSE);

    return button;
}

void zenith_dock_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    apply_dock_css();

    GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_widget_add_css_class(GTK_WIDGET(window), "zenith-dock");

    gtk_layer_init_for_window(window);
    gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_namespace(window, "zenith-dock");

    /* Anchor to bottom and center it */
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);

    /* Margin gives it a floating look if we wanted it, but let's stick to bottom initially */
    gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_BOTTOM, 0);

    /* Set exclusive zone to prevent windows overlapping */
    gtk_layer_set_exclusive_zone(window, DOCK_HEIGHT);

    gtk_widget_set_size_request(GTK_WIDGET(window), -1, DOCK_HEIGHT);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_window_set_child(window, box);

    /* For Phase 1, just hardcode some stub launchers */
    gtk_box_append(GTK_BOX(box), create_dock_icon("utilities-terminal-symbolic", "Terminal"));
    gtk_box_append(GTK_BOX(box), create_dock_icon("system-file-manager-symbolic", "Files"));
    gtk_box_append(GTK_BOX(box), create_dock_icon("web-browser-symbolic", "Browser"));
    gtk_box_append(GTK_BOX(box), create_dock_icon("preferences-system-symbolic", "Settings"));

    gtk_window_present(window);
}
