/*
 * Zenith OS — Application Launcher
 *
 * launcher.h — Launcher definitions
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ZENITH_LAUNCHER_H
#define ZENITH_LAUNCHER_H

#include <gtk/gtk.h>

void zenith_launcher_activate(GtkApplication *app, gpointer user_data);

#endif /* ZENITH_LAUNCHER_H */
