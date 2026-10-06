/*
 * Zenith OS — File Manager
 *
 * window.c — Main window and file view
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "window.h"

enum {
    COL_ICON,
    COL_NAME,
    COL_SIZE,
    COL_TYPE,
    COL_PATH,
    NUM_COLS
};

static GtkListStore *file_store = NULL;
static char current_path[1024];
static GtkEntry *path_entry = NULL;

static void load_directory(const char *path) {
    gtk_list_store_clear(file_store);

    DIR *dir = opendir(path);
    if (!dir) return;

    strncpy(current_path, path, sizeof(current_path) - 1);
    gtk_editable_set_text(GTK_EDITABLE(path_entry), current_path);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0) continue;

        char full_path[2048];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) != 0) continue;

        const char *icon_name = "text-x-generic";
        const char *type_name = "File";
        char size_str[32] = "";

        if (S_ISDIR(st.st_mode)) {
            icon_name = "folder";
            type_name = "Folder";
        } else {
            if (st.st_size > 1024 * 1024)
                snprintf(size_str, sizeof(size_str), "%.1f MB", (double)st.st_size / (1024 * 1024));
            else if (st.st_size > 1024)
                snprintf(size_str, sizeof(size_str), "%.1f KB", (double)st.st_size / 1024);
            else
                snprintf(size_str, sizeof(size_str), "%lld B", (long long)st.st_size);

            /* Very basic mime sniffing by extension */
            if (strstr(entry->d_name, ".png") || strstr(entry->d_name, ".jpg"))
                icon_name = "image-x-generic";
            else if (strstr(entry->d_name, ".txt") || strstr(entry->d_name, ".md"))
                icon_name = "text-x-generic";
            else if (st.st_mode & S_IXUSR)
                icon_name = "application-x-executable";
        }

        GtkTreeIter iter;
        gtk_list_store_append(file_store, &iter);
        gtk_list_store_set(file_store, &iter,
            COL_ICON, icon_name,
            COL_NAME, entry->d_name,
            COL_SIZE, size_str,
            COL_TYPE, type_name,
            COL_PATH, full_path,
            -1);
    }
    closedir(dir);
}

static void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    (void)column;
    (void)user_data;

    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, path)) {
        char *full_path = NULL;
        char *type = NULL;
        gtk_tree_model_get(GTK_TREE_MODEL(file_store), &iter, COL_PATH, &full_path, COL_TYPE, &type, -1);

        if (strcmp(type, "Folder") == 0) {
            load_directory(full_path);
        } else {
            /* Open file with xdg-open normally, but here we can just use foot */
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), "foot -e vi \"%s\" &", full_path);
            int ret = system(cmd);
            (void)ret;
        }

        g_free(full_path);
        g_free(type);
    }
}

static void on_up_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    if (strcmp(current_path, "/") == 0) return;

    char *last_slash = strrchr(current_path, '/');
    if (last_slash && last_slash != current_path) {
        *last_slash = '\0';
        load_directory(current_path);
    } else if (last_slash == current_path) {
        load_directory("/");
    }
}

static void on_path_activated(GtkEntry *entry, gpointer user_data) {
    (void)user_data;
    const char *path = gtk_editable_get_text(GTK_EDITABLE(entry));
    load_directory(path);
}

void zenith_file_manager_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(window, "Files");
    gtk_window_set_default_size(window, 900, 600);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(window, vbox);

    /* Toolbar */
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(toolbar, 8);
    gtk_widget_set_margin_end(toolbar, 8);
    gtk_widget_set_margin_top(toolbar, 8);
    gtk_widget_set_margin_bottom(toolbar, 8);
    gtk_box_append(GTK_BOX(vbox), toolbar);

    GtkWidget *up_btn = gtk_button_new_from_icon_name("go-up-symbolic");
    g_signal_connect(up_btn, "clicked", G_CALLBACK(on_up_clicked), NULL);
    gtk_box_append(GTK_BOX(toolbar), up_btn);

    path_entry = GTK_ENTRY(gtk_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(path_entry), TRUE);
    g_signal_connect(path_entry, "activate", G_CALLBACK(on_path_activated), NULL);
    gtk_box_append(GTK_BOX(toolbar), GTK_WIDGET(path_entry));

    /* Main view area */
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_vexpand(paned, TRUE);
    gtk_box_append(GTK_BOX(vbox), paned);

    /* Sidebar */
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_size_request(sidebar, 200, -1);
    gtk_widget_set_margin_top(sidebar, 8);
    gtk_widget_set_margin_start(sidebar, 8);
    gtk_paned_set_start_child(GTK_PANED(paned), sidebar);

    const char *places[] = {"Home", "/home/zenith", "Root", "/", "Temp", "/tmp", NULL};
    for (int i = 0; places[i]; i += 2) {
        GtkWidget *btn = gtk_button_new_with_label(places[i]);
        gtk_widget_set_halign(btn, GTK_ALIGN_FILL);
        // Simple hack: store path string directly (leaks on exit but ok for mockup)
        g_signal_connect(btn, "clicked", G_CALLBACK(on_path_activated), NULL);
        gtk_box_append(GTK_BOX(sidebar), btn);
    }

    /* File list */
    file_store = gtk_list_store_new(NUM_COLS,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(file_store));
    g_signal_connect(tree, "row-activated", G_CALLBACK(on_row_activated), NULL);

    GtkCellRenderer *icon_rend = gtk_cell_renderer_pixbuf_new();
    GtkTreeViewColumn *icon_col = gtk_tree_view_column_new_with_attributes(
        "", icon_rend, "icon-name", COL_ICON, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), icon_col);

    GtkCellRenderer *text_rend = gtk_cell_renderer_text_new();

    GtkTreeViewColumn *name_col = gtk_tree_view_column_new_with_attributes(
        "Name", text_rend, "text", COL_NAME, NULL);
    gtk_tree_view_column_set_expand(name_col, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), name_col);

    GtkTreeViewColumn *size_col = gtk_tree_view_column_new_with_attributes(
        "Size", text_rend, "text", COL_SIZE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), size_col);

    GtkTreeViewColumn *type_col = gtk_tree_view_column_new_with_attributes(
        "Type", text_rend, "text", COL_TYPE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), type_col);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), tree);
    gtk_paned_set_end_child(GTK_PANED(paned), scroll);

    /* Load initial path */
    const char *home = g_getenv("HOME");
    load_directory(home ? home : "/");

    gtk_window_present(window);
}
