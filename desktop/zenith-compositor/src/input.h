/*
 * Zenith OS — Compositor
 *
 * input.h — Input device management
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ZENITH_INPUT_H
#define ZENITH_INPUT_H

struct zenith_compositor;

/**
 * Initialize input handling: cursor, seat, and new-input listener.
 */
void zenith_input_init(struct zenith_compositor *compositor);

#endif /* ZENITH_INPUT_H */
