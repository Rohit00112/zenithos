#include "notification.h"
#include <string.h>

ZenithNotification *notification_new(guint32 id, const char *app_name,
                                      const char *summary, const char *body,
                                      const char *icon, gint32 expire_timeout) {
    ZenithNotification *notif = g_new0(ZenithNotification, 1);
    notif->id = id;
    notif->app_name = g_strdup(app_name ? app_name : "");
    notif->summary = g_strdup(summary ? summary : "");
    notif->body = g_strdup(body ? body : "");
    notif->icon = g_strdup(icon ? icon : "");
    notif->expire_timeout = expire_timeout;
    notif->timestamp = g_get_monotonic_time();
    return notif;
}

void notification_free(ZenithNotification *notif) {
    if (!notif) return;
    g_free(notif->app_name);
    g_free(notif->summary);
    g_free(notif->body);
    g_free(notif->icon);
    g_free(notif);
}
