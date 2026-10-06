/*
 * Zenith OS — Panel
 *
 * main.c — Entry point for the Zenith top panel
 *
 * The panel is a GTK4 application that uses gtk4-layer-shell
 * to anchor itself to the top of the screen.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include "panel.h"

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
        "org.zenith.Panel",
        G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(zenith_panel_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
