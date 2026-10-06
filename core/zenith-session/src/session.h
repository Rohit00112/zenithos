/*
 * Zenith OS — Session Manager
 *
 * session.h — Session lifecycle management
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ZENITH_SESSION_H
#define ZENITH_SESSION_H

/**
 * Start the Zenith desktop session.
 *
 * This launches:
 *   1. zenith-compositor (Wayland compositor)
 *   2. zenith-panel (top panel)
 *   3. Sets up environment variables
 *
 * Returns the exit code of the compositor.
 */
int zenith_session_start(void);

#endif /* ZENITH_SESSION_H */
