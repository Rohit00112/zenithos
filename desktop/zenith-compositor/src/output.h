/*
 * Zenith OS — Compositor
 *
 * output.h — Output (monitor/display) management
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ZENITH_OUTPUT_H
#define ZENITH_OUTPUT_H

#include <wayland-server-core.h>

/* Forward declaration */
struct zenith_compositor;

/**
 * Called when a new output (monitor) is detected by the backend.
 */
void zenith_output_new(struct wl_listener *listener, void *data);

#endif /* ZENITH_OUTPUT_H */
