/*
 * Zenith OS — Network Center
 *
 * network.c — Network interfaces and connections UI
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "network.h"

void zenith_network_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(window, "Network Settings");
    gtk_window_set_default_size(window, 800, 600);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_top(vbox, 24);
    gtk_widget_set_margin_bottom(vbox, 24);
    gtk_widget_set_margin_start(vbox, 24);
    gtk_widget_set_margin_end(vbox, 24);
    gtk_window_set_child(window, vbox);

    GtkWidget *label = gtk_label_new("Network management requires NetworkManager D-Bus API.");
    gtk_box_append(GTK_BOX(vbox), label);

    GtkWidget *stub = gtk_label_new("Phase 3 Stub.");
    gtk_widget_add_css_class(stub, "dim-label");
    gtk_box_append(GTK_BOX(vbox), stub);

    gtk_window_present(window);
}
