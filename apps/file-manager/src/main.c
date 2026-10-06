/*
 * Zenith OS — File Manager
 *
 * main.c — Entry point
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include "window.h"

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
        "org.zenith.FileManager",
        G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(zenith_file_manager_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
