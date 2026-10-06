/*
 * Zenith OS — Session Manager
 *
 * session.c — Session lifecycle implementation
 *
 * Starts the compositor and shell components, waits for
 * the compositor to exit, then cleans up.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "session.h"

#define MAX_CHILDREN 16

static pid_t children[MAX_CHILDREN];
static int num_children = 0;

/**
 * Launch a child process.
 * Returns the PID of the child, or -1 on failure.
 */
static pid_t launch_child(const char *name, const char *const argv[]) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        /* Child process */
        setsid();
        execvp(argv[0], (char *const *)argv);
        fprintf(stderr, "zenith-session: failed to exec %s: ", name);
        perror("");
        _exit(EXIT_FAILURE);
    }

    /* Parent */
    printf("zenith-session: started %s (PID %d)\n", name, pid);
    if (num_children < MAX_CHILDREN) {
        children[num_children++] = pid;
    }
    return pid;
}

/**
 * Kill all child processes.
 */
static void kill_children(void) {
    for (int i = 0; i < num_children; i++) {
        if (children[i] > 0) {
            printf("zenith-session: terminating PID %d\n", children[i]);
            kill(children[i], SIGTERM);
        }
    }

    /* Give children time to exit gracefully */
    usleep(500000);  /* 500ms */

    for (int i = 0; i < num_children; i++) {
        if (children[i] > 0) {
            int status;
            pid_t result = waitpid(children[i], &status, WNOHANG);
            if (result == 0) {
                /* Still alive — force kill */
                kill(children[i], SIGKILL);
                waitpid(children[i], &status, 0);
            }
        }
    }
}

int zenith_session_start(void) {
    /* Set environment for Wayland session */
    setenv("XDG_SESSION_TYPE", "wayland", 1);
    setenv("XDG_CURRENT_DESKTOP", "Zenith", 1);
    setenv("XDG_SESSION_DESKTOP", "zenith", 1);
    setenv("GDK_BACKEND", "wayland", 1);
    setenv("QT_QPA_PLATFORM", "wayland", 1);
    setenv("SDL_VIDEODRIVER", "wayland", 1);
    setenv("MOZ_ENABLE_WAYLAND", "1", 1);
    setenv("_JAVA_AWT_WM_NONREPARENTING", "1", 1);

    /*
     * Start the compositor with a startup script that launches
     * panel and dock after the compositor is ready.
     */
    const char *compositor_argv[] = {
        "zenith-compositor",
        "-s", "bash -c 'zenith-panel & zenith-dock &'",
        NULL
    };

    pid_t compositor_pid = launch_child("zenith-compositor", compositor_argv);
    if (compositor_pid < 0) {
        fprintf(stderr, "zenith-session: failed to start compositor\n");
        return EXIT_FAILURE;
    }

    /* Wait for the compositor to exit */
    int status;
    waitpid(compositor_pid, &status, 0);

    /* Compositor exited — clean up everything */
    printf("zenith-session: compositor exited, cleaning up...\n");
    kill_children();

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return EXIT_FAILURE;
}
