/*
 * Zenith OS — Wallpaper
 *
 * main.c — Layer shell background layer
 *
 * Loads an image or color and places it on the BACKGROUND layer.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

static void apply_wallpaper_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "window.zenith-wallpaper {"
        "  background-color: #0d1117;"
        "}";

    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    apply_wallpaper_css();

    GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_widget_add_css_class(GTK_WIDGET(window), "zenith-wallpaper");

    gtk_layer_init_for_window(window);
    gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_BACKGROUND);
    gtk_layer_set_namespace(window, "zenith-wallpaper");

    /* Cover entire screen */
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    gtk_window_present(window);
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
        "org.zenith.Wallpaper",
        G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
