/*
 * Zenith OS — Session Manager
 *
 * main.c — Entry point for zenith-session
 *
 * This is the top-level process that starts the Zenith desktop.
 * It can be invoked by a display manager or from a TTY.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include "session.h"

int main(void) {
    printf("Zenith OS — Starting desktop session...\n");

    int ret = zenith_session_start();

    printf("Zenith OS — Session ended (exit code: %d)\n", ret);
    return ret;
}
