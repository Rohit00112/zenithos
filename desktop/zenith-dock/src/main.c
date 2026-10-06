/*
 * Zenith OS — Dock
 *
 * main.c — Entry point for the Zenith dock
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include "dock.h"

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
        "org.zenith.Dock",
        G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(zenith_dock_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
