/*
 * Zenith Notifications Daemon
 * Implements org.freedesktop.Notifications D-Bus interface
 */

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "notification.h"
#include "popup.h"

#define DBUS_NAME "org.freedesktop.Notifications"
#define DBUS_PATH "/org/freedesktop/Notifications"
#define DBUS_INTERFACE "org.freedesktop.Notifications"

static const char introspection_xml[] =
    "<node>"
    "  <interface name='org.freedesktop.Notifications'>"
    "    <method name='GetCapabilities'>"
    "      <arg direction='out' name='capabilities' type='as'/>"
    "    </method>"
    "    <method name='Notify'>"
    "      <arg direction='in' name='app_name' type='s'/>"
    "      <arg direction='in' name='replaces_id' type='u'/>"
    "      <arg direction='in' name='app_icon' type='s'/>"
    "      <arg direction='in' name='summary' type='s'/>"
    "      <arg direction='in' name='body' type='s'/>"
    "      <arg direction='in' name='actions' type='as'/>"
    "      <arg direction='in' name='hints' type='a{sv}'/>"
    "      <arg direction='in' name='expire_timeout' type='i'/>"
    "      <arg direction='out' name='id' type='u'/>"
    "    </method>"
    "    <method name='CloseNotification'>"
    "      <arg direction='in' name='id' type='u'/>"
    "    </method>"
    "    <method name='GetServerInformation'>"
    "      <arg direction='out' name='name' type='s'/>"
    "      <arg direction='out' name='vendor' type='s'/>"
    "      <arg direction='out' name='version' type='s'/>"
    "      <arg direction='out' name='spec_version' type='s'/>"
    "    </method>"
    "    <signal name='NotificationClosed'>"
    "      <arg name='id' type='u'/>"
    "      <arg name='reason' type='u'/>"
    "    </signal>"
    "    <signal name='ActionInvoked'>"
    "      <arg name='id' type='u'/>"
    "      <arg name='action_key' type='s'/>"
    "    </signal>"
    "  </interface>"
    "</node>";

typedef struct {
    GDBusConnection *connection;
    guint owner_id;
    guint registration_id;
    GHashTable *active_notifications;
    guint32 next_id;
} NotificationServer;

static NotificationServer *server = NULL;

static void handle_get_capabilities(GDBusMethodInvocation *invocation) {
    const char *capabilities[] = {
        "body",
        "body-markup",
        "icon-static",
        NULL
    };

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));
    for (int i = 0; capabilities[i] != NULL; i++) {
        g_variant_builder_add(&builder, "s", capabilities[i]);
    }

    g_dbus_method_invocation_return_value(invocation,
        g_variant_new("(as)", &builder));
}

static void handle_notify(GDBusMethodInvocation *invocation,
                          GVariant *parameters) {
    const char *app_name, *icon, *summary, *body;
    guint32 replaces_id;
    gint32 expire_timeout;
    GVariant *actions, *hints;

    g_variant_get(parameters, "(&su&s&s&s@as@a{sv}i)",
                  &app_name, &replaces_id, &icon, &summary, &body,
                  &actions, &hints, &expire_timeout);

    // Generate new ID if not replacing
    guint32 id = replaces_id > 0 ? replaces_id : server->next_id++;

    // Close existing if replacing
    if (replaces_id > 0) {
        NotificationPopup *old = g_hash_table_lookup(server->active_notifications,
                                                       GUINT_TO_POINTER(replaces_id));
        if (old) {
            notification_popup_close(old);
            g_hash_table_remove(server->active_notifications,
                                GUINT_TO_POINTER(replaces_id));
        }
    }

    // Create notification
    ZenithNotification *notif = notification_new(id, app_name, summary, body,
                                                  icon, expire_timeout);
    NotificationPopup *popup = notification_popup_new(notif);

    // Track it
    g_hash_table_insert(server->active_notifications,
                        GUINT_TO_POINTER(id), popup);

    // Show it
    notification_popup_show(popup);

    // Return ID
    g_dbus_method_invocation_return_value(invocation,
        g_variant_new("(u)", id));

    g_variant_unref(actions);
    g_variant_unref(hints);
}

static void handle_close_notification(GDBusMethodInvocation *invocation,
                                       GVariant *parameters) {
    guint32 id;
    g_variant_get(parameters, "(u)", &id);

    NotificationPopup *popup = g_hash_table_lookup(server->active_notifications,
                                                     GUINT_TO_POINTER(id));
    if (popup) {
        notification_popup_close(popup);
        g_hash_table_remove(server->active_notifications, GUINT_TO_POINTER(id));

        // Emit NotificationClosed signal
        g_dbus_connection_emit_signal(server->connection,
            NULL, DBUS_PATH, DBUS_INTERFACE,
            "NotificationClosed",
            g_variant_new("(uu)", id, 3), // reason 3 = closed by call
            NULL);
    }

    g_dbus_method_invocation_return_value(invocation, NULL);
}

static void handle_get_server_information(GDBusMethodInvocation *invocation) {
    g_dbus_method_invocation_return_value(invocation,
        g_variant_new("(ssss)",
                      "Zenith Notifications",
                      "Zenith OS",
                      "0.1.0",
                      "1.2"));
}

static void handle_method_call(GDBusConnection *connection,
                                const gchar *sender,
                                const gchar *object_path,
                                const gchar *interface_name,
                                const gchar *method_name,
                                GVariant *parameters,
                                GDBusMethodInvocation *invocation,
                                gpointer user_data) {
    if (g_strcmp0(method_name, "GetCapabilities") == 0) {
        handle_get_capabilities(invocation);
    } else if (g_strcmp0(method_name, "Notify") == 0) {
        handle_notify(invocation, parameters);
    } else if (g_strcmp0(method_name, "CloseNotification") == 0) {
        handle_close_notification(invocation, parameters);
    } else if (g_strcmp0(method_name, "GetServerInformation") == 0) {
        handle_get_server_information(invocation);
    }
}

static const GDBusInterfaceVTable interface_vtable = {
    handle_method_call,
    NULL,
    NULL
};

static void on_bus_acquired(GDBusConnection *connection,
                             const gchar *name,
                             gpointer user_data) {
    GError *error = NULL;
    GDBusNodeInfo *introspection_data;

    introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, &error);
    if (!introspection_data) {
        g_critical("Failed to parse introspection XML: %s", error->message);
        g_error_free(error);
        return;
    }

    server->registration_id = g_dbus_connection_register_object(
        connection,
        DBUS_PATH,
        introspection_data->interfaces[0],
        &interface_vtable,
        NULL,
        NULL,
        &error);

    if (server->registration_id == 0) {
        g_critical("Failed to register object: %s", error->message);
        g_error_free(error);
    }

    g_dbus_node_info_unref(introspection_data);
    server->connection = g_object_ref(connection);
}

static void on_name_acquired(GDBusConnection *connection,
                              const gchar *name,
                              gpointer user_data) {
    g_message("Acquired D-Bus name: %s", name);
}

static void on_name_lost(GDBusConnection *connection,
                          const gchar *name,
                          gpointer user_data) {
    g_critical("Lost D-Bus name: %s", name);
    g_application_quit(G_APPLICATION(user_data));
}

int main(int argc, char *argv[]) {
    GtkApplication *app;

    app = gtk_application_new("org.zenith.Notifications",
                               G_APPLICATION_DEFAULT_FLAGS);

    // Initialize server
    server = g_new0(NotificationServer, 1);
    server->active_notifications = g_hash_table_new_full(
        g_direct_hash, g_direct_equal,
        NULL, NULL);
    server->next_id = 1;

    // Own D-Bus name
    server->owner_id = g_bus_own_name(
        G_BUS_TYPE_SESSION,
        DBUS_NAME,
        G_BUS_NAME_OWNER_FLAGS_NONE,
        on_bus_acquired,
        on_name_acquired,
        on_name_lost,
        app,
        NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup
    if (server->owner_id > 0) {
        g_bus_unown_name(server->owner_id);
    }
    if (server->registration_id > 0) {
        g_dbus_connection_unregister_object(server->connection,
                                             server->registration_id);
    }
    if (server->connection) {
        g_object_unref(server->connection);
    }
    g_hash_table_destroy(server->active_notifications);
    g_free(server);
    g_object_unref(app);

    return status;
}
