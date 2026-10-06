/*
 * Zenith OS — Panel
 *
 * clock.c — Clock widget for the panel
 *
 * Displays current time in HH:MM format, updates every second.
 * Shows date as tooltip.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include <time.h>

#include "panel.h"

/**
 * Update the clock label with the current time.
 */
static gboolean update_clock(gpointer user_data) {
    GtkLabel *label = GTK_LABEL(user_data);

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char time_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M", tm_info);
    gtk_label_set_text(label, time_str);

    /* Update tooltip with full date */
    char date_str[128];
    strftime(date_str, sizeof(date_str), "%A, %B %d, %Y", tm_info);
    gtk_widget_set_tooltip_text(GTK_WIDGET(label), date_str);

    return G_SOURCE_CONTINUE;
}

GtkWidget *zenith_clock_create(void) {
    GtkWidget *label = gtk_label_new("--:--");
    gtk_widget_add_css_class(label, "panel-clock");

    /* Update immediately */
    update_clock(label);

    /* Schedule updates every second */
    g_timeout_add_seconds(1, update_clock, label);

    return label;
}
