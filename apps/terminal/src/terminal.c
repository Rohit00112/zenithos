/*
 * Zenith OS — Terminal
 *
 * terminal.c — VTE terminal widget setup
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include <vte/vte.h>
#include "terminal.h"

static void on_child_exited(VteTerminal *vte, int status, gpointer user_data) {
    (void)vte;
    (void)status;
    GtkWindow *window = GTK_WINDOW(user_data);
    gtk_window_close(window);
}

static void apply_terminal_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "window.zenith-terminal {"
        "  background-color: #0d1117;"
        "}"
        "vte-terminal {"
        "  padding: 12px;"
        "}";

    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

void zenith_terminal_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    apply_terminal_css();

    GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(window, "Terminal");
    gtk_window_set_default_size(window, 800, 600);
    gtk_widget_add_css_class(GTK_WIDGET(window), "zenith-terminal");

    /* Create VTE widget */
    GtkWidget *terminal = vte_terminal_new();
    gtk_widget_set_hexpand(terminal, TRUE);
    gtk_widget_set_vexpand(terminal, TRUE);

    /* Config styles */
    GdkRGBA bg = {0.05, 0.066, 0.09, 1.0};  /* #0d1117 */
    GdkRGBA fg = {0.9, 0.93, 0.95, 1.0};    /* #e6edf3 */

    /* Zenith theme palette: black, red, green, yellow, blue, magenta, cyan, white */
    GdkRGBA palette[16] = {
        {0.1, 0.1, 0.1, 1.0}, {0.9, 0.3, 0.3, 1.0}, {0.3, 0.8, 0.3, 1.0}, {0.9, 0.8, 0.2, 1.0},
        {0.3, 0.5, 0.9, 1.0}, {0.8, 0.3, 0.8, 1.0}, {0.2, 0.8, 0.8, 1.0}, {0.8, 0.8, 0.8, 1.0},
        {0.3, 0.3, 0.3, 1.0}, {1.0, 0.4, 0.4, 1.0}, {0.4, 1.0, 0.4, 1.0}, {1.0, 1.0, 0.3, 1.0},
        {0.4, 0.6, 1.0, 1.0}, {1.0, 0.4, 1.0, 1.0}, {0.3, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}
    };

    vte_terminal_set_colors(VTE_TERMINAL(terminal), &fg, &bg, palette, 16);

    PangoFontDescription *font_desc = pango_font_description_from_string("JetBrains Mono, Fira Code, Monospace 11");
    vte_terminal_set_font(VTE_TERMINAL(terminal), font_desc);
    pango_font_description_free(font_desc);

    /* Scrollback and cursor */
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal), 10000);
    vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(terminal), VTE_CURSOR_BLINK_ON);

    /* Launch shell */
    char *shell = g_strdup(g_getenv("SHELL"));
    if (!shell) shell = g_strdup("/bin/bash");
    char **envp = g_get_environ();
    char *const spawn_argv[] = {shell, NULL};

    g_signal_connect(terminal, "child-exited", G_CALLBACK(on_child_exited), window);

    vte_terminal_spawn_async(VTE_TERMINAL(terminal),
                             VTE_PTY_DEFAULT,
                             NULL,       /* working directory */
                             spawn_argv, /* argv */
                             envp,       /* envv */
                             G_SPAWN_DEFAULT,
                             NULL, NULL, /* child setup */
                             NULL,       /* child pid */
                             -1,         /* timeout */
                             NULL,       /* cancellable */
                             NULL, NULL);/* callback */

    g_strfreev(envp);
    g_free(shell);

    /* Embed into window */
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), terminal);
    gtk_window_set_child(window, scroll);

    gtk_window_present(window);
}
