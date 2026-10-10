#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <gtk/gtk.h>

typedef struct {
    guint32 id;
    char *app_name;
    char *summary;
    char *body;
    char *icon;
    gint64 expire_timeout;
    gint64 timestamp;
} ZenithNotification;

ZenithNotification *notification_new(guint32 id, const char *app_name,
                                      const char *summary, const char *body,
                                      const char *icon, gint32 expire_timeout);
void notification_free(ZenithNotification *notif);

#endif
