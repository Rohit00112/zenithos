#include "popup.h"
#include <gtk4-layer-shell.h>

struct _NotificationPopup {
    GtkWindow *window;
    ZenithNotification *notif;
    guint timeout_id;
};

static gboolean on_timeout(gpointer data) {
    NotificationPopup *popup = data;
    notification_popup_close(popup);
    return G_SOURCE_REMOVE;
}

NotificationPopup *notification_popup_new(ZenithNotification *notif) {
    NotificationPopup *popup = g_new0(NotificationPopup, 1);
    popup->notif = notif;

    // Create window
    popup->window = GTK_WINDOW(gtk_window_new());
    gtk_window_set_decorated(popup->window, FALSE);
    gtk_window_set_default_size(popup->window, 350, 80);

    // Layer shell
    gtk_layer_init_for_window(popup->window);
    gtk_layer_set_layer(popup->window, GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_anchor(popup->window, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(popup->window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_margin(popup->window, GTK_LAYER_SHELL_EDGE_TOP, 10);
    gtk_layer_set_margin(popup->window, GTK_LAYER_SHELL_EDGE_RIGHT, 10);

    // Build UI
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(box, 12);
    gtk_widget_set_margin_end(box, 12);
    gtk_widget_set_margin_top(box, 12);
    gtk_widget_set_margin_bottom(box, 12);

    if (notif->summary && notif->summary[0]) {
        GtkWidget *summary = gtk_label_new(notif->summary);
        gtk_label_set_wrap(GTK_LABEL(summary), TRUE);
        gtk_label_set_xalign(GTK_LABEL(summary), 0.0);
        gtk_widget_add_css_class(summary, "title-3");
        gtk_box_append(GTK_BOX(box), summary);
    }

    if (notif->body && notif->body[0]) {
        GtkWidget *body = gtk_label_new(notif->body);
        gtk_label_set_wrap(GTK_LABEL(body), TRUE);
        gtk_label_set_xalign(GTK_LABEL(body), 0.0);
        gtk_box_append(GTK_BOX(box), body);
    }

    gtk_window_set_child(popup->window, box);

    // CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        "window { "
        "  background: rgba(30, 30, 30, 0.95); "
        "  border-radius: 8px; "
        "  border: 1px solid rgba(255, 255, 255, 0.1); "
        "  color: #ffffff; "
        "}"
    );
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);

    return popup;
}

void notification_popup_show(NotificationPopup *popup) {
    if (!popup) return;

    gtk_window_present(popup->window);

    // Auto-close timeout
    gint64 timeout_ms = popup->notif->expire_timeout;
    if (timeout_ms <= 0) timeout_ms = 5000; // 5 seconds default
    if (timeout_ms > 0) {
        popup->timeout_id = g_timeout_add(timeout_ms, on_timeout, popup);
    }
}

void notification_popup_close(NotificationPopup *popup) {
    if (!popup) return;
    if (popup->timeout_id > 0) {
        g_source_remove(popup->timeout_id);
        popup->timeout_id = 0;
    }
    gtk_window_destroy(popup->window);
    g_free(popup);
}
