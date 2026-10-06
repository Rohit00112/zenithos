/*
 * Zenith OS — System Monitor
 *
 * monitor.c — Process list and resource usage viewer
 *
 * Reads /proc to display CPU, memory, and process list.
 * Refreshes every 2 seconds via GLib timer.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define REFRESH_INTERVAL_MS 2000

/* Column indices for the process list model */
enum {
    COL_PID,
    COL_NAME,
    COL_CPU,
    COL_MEM_MB,
    COL_STATUS,
    NUM_COLS
};

static GtkListStore *process_store = NULL;
static GtkLabel *cpu_label = NULL;
static GtkLabel *mem_label = NULL;
static GtkLabel *load_label = NULL;

typedef struct {
    long pid;
    char name[256];
    char status;
    long vmrss_kb;
    unsigned long utime;
    unsigned long stime;
} ProcEntry;

static double read_cpu_usage(void) {
    static unsigned long long prev_idle = 0, prev_total = 0;

    FILE *f = fopen("/proc/stat", "r");
    if (!f) return 0.0;

    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    fscanf(f, "cpu %llu %llu %llu %llu %llu %llu %llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    fclose(f);

    unsigned long long total = user + nice + system + idle + iowait + irq + softirq;
    unsigned long long diff_idle = idle - prev_idle;
    unsigned long long diff_total = total - prev_total;

    double usage = 0.0;
    if (diff_total > 0)
        usage = 100.0 * (1.0 - (double)diff_idle / diff_total);

    prev_idle = idle;
    prev_total = total;
    return usage;
}

static void read_mem_info(long *total_mb, long *used_mb) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) { *total_mb = *used_mb = 0; return; }

    long total_kb = 0, free_kb = 0, buffers_kb = 0, cached_kb = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        long val;
        if (sscanf(line, "MemTotal: %ld kB", &val) == 1) total_kb = val;
        else if (sscanf(line, "MemFree: %ld kB", &val) == 1) free_kb = val;
        else if (sscanf(line, "Buffers: %ld kB", &val) == 1) buffers_kb = val;
        else if (sscanf(line, "Cached: %ld kB", &val) == 1) cached_kb = val;
    }
    fclose(f);

    *total_mb = total_kb / 1024;
    *used_mb = (total_kb - free_kb - buffers_kb - cached_kb) / 1024;
}

static void refresh_processes(void) {
    gtk_list_store_clear(process_store);

    DIR *dir = opendir("/proc");
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        /* Only numeric entries are PIDs */
        char *endp;
        long pid = strtol(entry->d_name, &endp, 10);
        if (*endp != '\0' || pid <= 0) continue;

        char stat_path[64];
        snprintf(stat_path, sizeof(stat_path), "/proc/%ld/stat", pid);
        FILE *sf = fopen(stat_path, "r");
        if (!sf) continue;

        ProcEntry p = {0};
        p.pid = pid;
        unsigned long utime = 0, stime = 0;
        char state = 'R';
        char comm[256] = {0};
        fscanf(sf, "%*d (%255[^)]) %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu",
               comm, &state, &utime, &stime);
        fclose(sf);

        strncpy(p.name, comm, sizeof(p.name) - 1);
        p.status = state;
        p.utime = utime;
        p.stime = stime;

        /* Memory from /proc/PID/status */
        char status_path[64];
        snprintf(status_path, sizeof(status_path), "/proc/%ld/status", pid);
        FILE *mf = fopen(status_path, "r");
        if (mf) {
            char mline[128];
            while (fgets(mline, sizeof(mline), mf)) {
                if (sscanf(mline, "VmRSS: %ld kB", &p.vmrss_kb) == 1) break;
            }
            fclose(mf);
        }

        char pid_str[16];
        snprintf(pid_str, sizeof(pid_str), "%ld", pid);
        char mem_str[16];
        snprintf(mem_str, sizeof(mem_str), "%.1f", p.vmrss_kb / 1024.0);
        char state_str[2] = {p.status, 0};

        GtkTreeIter iter;
        gtk_list_store_append(process_store, &iter);
        gtk_list_store_set(process_store, &iter,
            COL_PID, pid_str,
            COL_NAME, p.name,
            COL_CPU, "—",
            COL_MEM_MB, mem_str,
            COL_STATUS, state_str,
            -1);
    }
    closedir(dir);
}

static gboolean on_refresh_tick(gpointer user_data) {
    (void)user_data;

    /* Update CPU */
    double cpu = read_cpu_usage();
    char cpu_str[32];
    snprintf(cpu_str, sizeof(cpu_str), "CPU: %.1f%%", cpu);
    gtk_label_set_text(cpu_label, cpu_str);

    /* Update memory */
    long total_mb, used_mb;
    read_mem_info(&total_mb, &used_mb);
    char mem_str[64];
    snprintf(mem_str, sizeof(mem_str), "Memory: %ld / %ld MB (%ld%%)",
             used_mb, total_mb, total_mb > 0 ? used_mb * 100 / total_mb : 0);
    gtk_label_set_text(mem_label, mem_str);

    /* Load average */
    double load1, load5, load15;
    FILE *lf = fopen("/proc/loadavg", "r");
    if (lf) {
        fscanf(lf, "%lf %lf %lf", &load1, &load5, &load15);
        fclose(lf);
        char load_str[64];
        snprintf(load_str, sizeof(load_str), "Load: %.2f  %.2f  %.2f", load1, load5, load15);
        gtk_label_set_text(load_label, load_str);
    }

    refresh_processes();
    return G_SOURCE_CONTINUE;
}

void zenith_monitor_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(window, "System Monitor");
    gtk_window_set_default_size(window, 850, 600);

    /* Top stat bar */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(window, vbox);

    GtkWidget *stat_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 24);
    gtk_widget_set_margin_start(stat_bar, 12);
    gtk_widget_set_margin_end(stat_bar, 12);
    gtk_widget_set_margin_top(stat_bar, 8);
    gtk_widget_set_margin_bottom(stat_bar, 8);
    gtk_box_append(GTK_BOX(vbox), stat_bar);

    cpu_label = GTK_LABEL(gtk_label_new("CPU: —"));
    mem_label = GTK_LABEL(gtk_label_new("Memory: —"));
    load_label = GTK_LABEL(gtk_label_new("Load: —"));

    gtk_box_append(GTK_BOX(stat_bar), GTK_WIDGET(cpu_label));
    gtk_box_append(GTK_BOX(stat_bar), GTK_WIDGET(mem_label));
    gtk_box_append(GTK_BOX(stat_bar), GTK_WIDGET(load_label));

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(vbox), sep);

    /* Process list */
    process_store = gtk_list_store_new(NUM_COLS,
        G_TYPE_STRING,  /* PID */
        G_TYPE_STRING,  /* NAME */
        G_TYPE_STRING,  /* CPU */
        G_TYPE_STRING,  /* MEM */
        G_TYPE_STRING); /* STATUS */

    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(process_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), TRUE);

    const char *col_titles[] = {"PID", "Name", "CPU", "Mem (MB)", "State"};
    for (int i = 0; i < NUM_COLS; i++) {
        GtkCellRenderer *r = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
            col_titles[i], r, "text", i, NULL);
        gtk_tree_view_column_set_resizable(col, TRUE);
        if (i == COL_NAME) gtk_tree_view_column_set_expand(col, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
    }

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), tree);
    gtk_box_append(GTK_BOX(vbox), scroll);

    /* Initial refresh + timer */
    on_refresh_tick(NULL);
    g_timeout_add(REFRESH_INTERVAL_MS, on_refresh_tick, NULL);

    gtk_window_present(window);
}
