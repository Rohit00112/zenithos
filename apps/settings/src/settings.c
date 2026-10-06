/*
 * Zenith OS — Settings
 *
 * settings.c — Main settings UI
 *
 * Provides a UI for basic system configuration.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "settings.h"

static GtkWidget* create_settings_row(const char *title, const char *subtitle, GtkWidget *control) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_top(row, 12);
    gtk_widget_set_margin_bottom(row, 12);
    gtk_widget_set_margin_start(row, 16);
    gtk_widget_set_margin_end(row, 16);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *title_label = gtk_label_new(title);
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    gtk_widget_add_css_class(title_label, "settings-title");

    GtkWidget *subtitle_label = gtk_label_new(subtitle);
    gtk_widget_set_halign(subtitle_label, GTK_ALIGN_START);
    gtk_widget_add_css_class(subtitle_label, "settings-subtitle");

    gtk_box_append(GTK_BOX(vbox), title_label);
    gtk_box_append(GTK_BOX(vbox), subtitle_label);

    gtk_box_append(GTK_BOX(row), vbox);

    if (control) {
        GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_hexpand(spacer, TRUE);
        gtk_box_append(GTK_BOX(row), spacer);

        gtk_widget_set_valign(control, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), control);
    }

    return row;
}

static GtkWidget* create_category_box(const char *title) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_top(vbox, 16);
    gtk_widget_set_margin_bottom(vbox, 16);

    GtkWidget *label = gtk_label_new(title);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 16);
    gtk_widget_set_margin_bottom(label, 8);
    gtk_widget_add_css_class(label, "category-title");

    gtk_box_append(GTK_BOX(vbox), label);

    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_set_margin_start(frame, 16);
    gtk_widget_set_margin_end(frame, 16);
    gtk_box_append(GTK_BOX(vbox), frame);

    GtkWidget *list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_frame_set_child(GTK_FRAME(frame), list);

    return vbox; /* Actually return the outer box, inner list holds items. Wait, we need to return the list too. */
}

/* Hardcode settings tabs for now */
static GtkWidget *create_appearance_page(void) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *cat = create_category_box("Theme");
    GtkWidget *list = gtk_frame_get_child(GTK_FRAME(gtk_widget_get_last_child(cat)));

    GtkWidget *theme_switch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(theme_switch), TRUE);

    gtk_box_append(GTK_BOX(list), create_settings_row("Dark Mode", "Use dark colors systematically", theme_switch));
    gtk_box_append(GTK_BOX(page), cat);

    return page;
}

static GtkWidget *create_about_page(void) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *cat = create_category_box("System Information");
    GtkWidget *list = gtk_frame_get_child(GTK_FRAME(gtk_widget_get_last_child(cat)));

    gtk_box_append(GTK_BOX(list), create_settings_row("Zenith OS", "Version 0.1.0 (Summit)", NULL));
    gtk_box_append(GTK_BOX(list), create_settings_row("Windowing System", "Wayland (wlroots)", NULL));
    gtk_box_append(GTK_BOX(list), create_settings_row("Updates", "Up to date", NULL));

    gtk_box_append(GTK_BOX(page), cat);

    return page;
}

static void apply_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        ".settings-title { font-weight: bold; font-size: 14pt; }"
        ".settings-subtitle { color: #888888; font-size: 11pt; }"
        ".category-title { color: #58a6ff; font-weight: bold; font-size: 12pt; }"
        "window.zenith-settings { background-color: #0d1117; }";

    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

void zenith_settings_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    apply_css();

    GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(window, "Settings");
    gtk_window_set_default_size(window, 900, 650);
    gtk_widget_add_css_class(GTK_WIDGET(window), "zenith-settings");

    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_vexpand(paned, TRUE);
    gtk_window_set_child(window, paned);

    /* Sidebar stack sidebar */
    GtkWidget *sidebar = gtk_stack_sidebar_new();
    gtk_widget_set_size_request(sidebar, 250, -1);
    gtk_paned_set_start_child(GTK_PANED(paned), sidebar);

    /* Stack for pages */
    GtkWidget *stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_paned_set_end_child(GTK_PANED(paned), stack);

    gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(sidebar), GTK_STACK(stack));

    /* Add pages */
    gtk_stack_add_titled(GTK_STACK(stack), create_appearance_page(), "appearance", "Appearance");
    gtk_stack_add_titled(GTK_STACK(stack), create_about_page(), "about", "About System");

    gtk_window_present(window);
}
