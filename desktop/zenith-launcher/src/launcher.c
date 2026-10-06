/*
 * Zenith OS — Application Launcher
 *
 * launcher.c — Fullscreen overlay app launcher with search
 *
 * Triggered by Super key or dock button. Shows a search bar
 * and grid of .desktop entries.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "launcher.h"

#define MAX_APPS 256

typedef struct {
    char name[128];
    char exec[256];
    char icon[128];
} AppEntry;

static AppEntry apps[MAX_APPS];
static int num_apps = 0;
static GtkWidget *flow_box = NULL;
static GtkWidget *search_entry = NULL;

static void apply_launcher_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();

    const char *css =
        "window.zenith-launcher {"
        "  background-color: rgba(13, 17, 23, 0.95);"
        "}"
        ".launcher-search {"
        "  background-color: rgba(22, 27, 34, 0.9);"
        "  color: #e6edf3;"
        "  border: 1px solid #30363d;"
        "  border-radius: 8px;"
        "  padding: 10px 16px;"
        "  font-size: 16px;"
        "  min-width: 400px;"
        "  margin: 40px;"
        "}"
        ".app-button {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 12px;"
        "  min-width: 80px;"
        "  min-height: 80px;"
        "  transition: all 150ms ease;"
        "}"
        ".app-button:hover {"
        "  background-color: rgba(255, 255, 255, 0.08);"
        "}"
        ".app-label {"
        "  color: #e6edf3;"
        "  font-size: 11px;"
        "}";

    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

static void scan_desktop_files(void) {
    const char *dirs[] = {
        "/usr/share/applications",
        "/usr/local/share/applications",
        NULL
    };

    num_apps = 0;

    for (int d = 0; dirs[d] != NULL && num_apps < MAX_APPS; d++) {
        DIR *dir = opendir(dirs[d]);
        if (!dir) continue;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL && num_apps < MAX_APPS) {
            if (!strstr(entry->d_name, ".desktop")) continue;

            char path[512];
            snprintf(path, sizeof(path), "%s/%s", dirs[d], entry->d_name);

            FILE *f = fopen(path, "r");
            if (!f) continue;

            AppEntry *app = &apps[num_apps];
            memset(app, 0, sizeof(*app));

            char line[512];
            int in_entry = 0;
            while (fgets(line, sizeof(line), f)) {
                line[strcspn(line, "\n")] = 0;

                if (strcmp(line, "[Desktop Entry]") == 0) {
                    in_entry = 1;
                    continue;
                }
                if (line[0] == '[') break;
                if (!in_entry) continue;

                if (strncmp(line, "Name=", 5) == 0 && app->name[0] == 0) {
                    strncpy(app->name, line + 5, sizeof(app->name) - 1);
                } else if (strncmp(line, "Exec=", 5) == 0) {
                    strncpy(app->exec, line + 5, sizeof(app->exec) - 1);
                } else if (strncmp(line, "Icon=", 5) == 0) {
                    strncpy(app->icon, line + 5, sizeof(app->icon) - 1);
                } else if (strncmp(line, "NoDisplay=true", 14) == 0) {
                    app->name[0] = 0;
                    break;
                }
            }
            fclose(f);

            if (app->name[0] && app->exec[0]) {
                num_apps++;
            }
        }
        closedir(dir);
    }
}

static void on_app_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppEntry *app = user_data;

    /* Strip field codes (%f, %u, etc.) from exec */
    char cmd[512];
    strncpy(cmd, app->exec, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = 0;
    char *pct = strchr(cmd, '%');
    if (pct) *pct = 0;

    /* Launch in background */
    char full_cmd[600];
    snprintf(full_cmd, sizeof(full_cmd), "%s &", cmd);
    int ret = system(full_cmd);
    (void)ret;

    /* Close launcher after launching */
    GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(button));
    if (root && GTK_IS_WINDOW(root)) {
        gtk_window_close(GTK_WINDOW(root));
    }
}

static GtkWidget* create_app_widget(AppEntry *app) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);

    GtkWidget *button = gtk_button_new();
    gtk_widget_add_css_class(button, "app-button");
    gtk_widget_set_focusable(button, FALSE);

    GtkWidget *icon;
    if (app->icon[0]) {
        icon = gtk_image_new_from_icon_name(app->icon);
    } else {
        icon = gtk_image_new_from_icon_name("application-x-executable-symbolic");
    }
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 48);
    gtk_button_set_child(GTK_BUTTON(button), icon);

    g_signal_connect(button, "clicked", G_CALLBACK(on_app_clicked), app);
    gtk_box_append(GTK_BOX(box), button);

    GtkWidget *label = gtk_label_new(app->name);
    gtk_widget_add_css_class(label, "app-label");
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 12);
    gtk_box_append(GTK_BOX(box), label);

    return box;
}

static void rebuild_grid(const char *filter) {
    /* Remove all children */
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(GTK_WIDGET(flow_box))) != NULL) {
        gtk_flow_box_remove(GTK_FLOW_BOX(flow_box), child);
    }

    for (int i = 0; i < num_apps; i++) {
        if (filter && filter[0]) {
            char lower_name[128];
            char lower_filter[128];
            strncpy(lower_name, apps[i].name, sizeof(lower_name) - 1);
            lower_name[sizeof(lower_name) - 1] = 0;
            strncpy(lower_filter, filter, sizeof(lower_filter) - 1);
            lower_filter[sizeof(lower_filter) - 1] = 0;

            for (char *p = lower_name; *p; p++) *p = (*p >= 'A' && *p <= 'Z') ? *p + 32 : *p;
            for (char *p = lower_filter; *p; p++) *p = (*p >= 'A' && *p <= 'Z') ? *p + 32 : *p;

            if (!strstr(lower_name, lower_filter)) continue;
        }
        gtk_flow_box_append(GTK_FLOW_BOX(flow_box), create_app_widget(&apps[i]));
    }
}

static void on_search_changed(GtkEditable *editable, gpointer user_data) {
    (void)user_data;
    const char *text = gtk_editable_get_text(editable);
    rebuild_grid(text);
}

static gboolean on_key_pressed(GtkEventControllerKey *controller,
                                guint keyval, guint keycode,
                                GdkModifierType state, gpointer user_data) {
    (void)controller;
    (void)keycode;
    (void)state;

    if (keyval == GDK_KEY_Escape) {
        GtkWidget *window = GTK_WIDGET(user_data);
        gtk_window_close(GTK_WINDOW(window));
        return TRUE;
    }
    return FALSE;
}

void zenith_launcher_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    apply_launcher_css();
    scan_desktop_files();

    GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_widget_add_css_class(GTK_WIDGET(window), "zenith-launcher");

    gtk_layer_init_for_window(window);
    gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_namespace(window, "zenith-launcher");
    gtk_layer_set_keyboard_mode(window, GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);

    /* Cover entire screen */
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    /* Escape to close */
    GtkEventController *key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed",
                     G_CALLBACK(on_key_pressed), window);
    gtk_widget_add_controller(GTK_WIDGET(window), key_controller);

    /* Main layout */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_START);
    gtk_window_set_child(window, vbox);

    /* Search entry */
    search_entry = gtk_entry_new();
    gtk_widget_add_css_class(search_entry, "launcher-search");
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Search applications...");
    g_signal_connect(search_entry, "changed", G_CALLBACK(on_search_changed), NULL);
    gtk_box_append(GTK_BOX(vbox), search_entry);

    /* App grid */
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_size_request(scroll, 600, 400);
    gtk_box_append(GTK_BOX(vbox), scroll);

    flow_box = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow_box), 6);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flow_box), 4);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flow_box), GTK_SELECTION_NONE);
    gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(flow_box), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), flow_box);

    rebuild_grid(NULL);

    gtk_window_present(window);
}
