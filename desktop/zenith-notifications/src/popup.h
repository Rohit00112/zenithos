#ifndef POPUP_H
#define POPUP_H

#include <gtk/gtk.h>
#include "notification.h"

typedef struct _NotificationPopup NotificationPopup;

NotificationPopup *notification_popup_new(ZenithNotification *notif);
void notification_popup_show(NotificationPopup *popup);
void notification_popup_close(NotificationPopup *popup);

#endif
