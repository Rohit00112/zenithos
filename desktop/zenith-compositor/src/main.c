/*
 * Zenith OS — Compositor
 *
 * main.c — Entry point for the Zenith Wayland compositor
 *
 * This starts the compositor, initializes the Wayland display,
 * sets up the backend, and enters the event loop.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define _POSIX_C_SOURCE 200112L
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wlr/util/log.h>

#include "compositor.h"

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -s <command>   Startup command to execute\n"
        "  -d             Enable debug logging\n"
        "  -h             Show this help\n"
        "\n"
        "Zenith OS Wayland Compositor v0.1.0\n",
        prog);
}

int main(int argc, char *argv[]) {
    char *startup_cmd = NULL;
    enum wlr_log_importance log_level = WLR_INFO;

    /* Parse command-line arguments */
    int opt;
    while ((opt = getopt(argc, argv, "s:dh")) != -1) {
        switch (opt) {
        case 's':
            startup_cmd = optarg;
            break;
        case 'd':
            log_level = WLR_DEBUG;
            break;
        case 'h':
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        default:
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    wlr_log_init(log_level, NULL);
    wlr_log(WLR_INFO, "Starting Zenith compositor v0.1.0");

    /* Initialize the compositor */
    struct zenith_compositor compositor = {0};
    if (!zenith_compositor_init(&compositor)) {
        wlr_log(WLR_ERROR, "Failed to initialize compositor");
        return EXIT_FAILURE;
    }

    /* Launch startup command if specified */
    if (startup_cmd) {
        wlr_log(WLR_INFO, "Launching startup command: %s", startup_cmd);
        if (fork() == 0) {
            execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (char *)NULL);
            _exit(EXIT_FAILURE);
        }
    }

    /* Enter the event loop */
    wlr_log(WLR_INFO, "Compositor running on Wayland display: %s",
             compositor.socket);
    zenith_compositor_run(&compositor);

    /* Cleanup */
    wlr_log(WLR_INFO, "Shutting down compositor");
    zenith_compositor_destroy(&compositor);

    return EXIT_SUCCESS;
}
