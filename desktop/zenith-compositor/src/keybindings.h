/*
 * Zenith OS — Compositor
 *
 * keybindings.h — Keyboard shortcut handling
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ZENITH_KEYBINDINGS_H
#define ZENITH_KEYBINDINGS_H

#include <stdbool.h>
#include <stdint.h>
#include <xkbcommon/xkbcommon.h>

struct zenith_compositor;

/**
 * Handle a key combination.
 * Returns true if the key was consumed by a compositor keybinding.
 */
bool zenith_keybinding_handle(struct zenith_compositor *compositor,
                               uint32_t modifiers, xkb_keysym_t sym);

#endif /* ZENITH_KEYBINDINGS_H */
