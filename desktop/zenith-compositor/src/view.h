/*
 * Zenith OS — Compositor
 *
 * view.h — View (window) management
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ZENITH_VIEW_H
#define ZENITH_VIEW_H

#include <wlr/types/wlr_xdg_shell.h>

struct zenith_compositor;
struct zenith_view;

/**
 * Create a new view for an XDG toplevel surface.
 */
void zenith_view_create(struct zenith_compositor *compositor,
                         struct wlr_xdg_toplevel *toplevel);

/**
 * Find the view (and surface within it) at the given layout coordinates.
 * Returns the view, sets *surface and surface-local coordinates.
 */
struct zenith_view *zenith_view_at(struct zenith_compositor *compositor,
                                    double lx, double ly,
                                    struct wlr_surface **surface,
                                    double *sx, double *sy);

/**
 * Begin an interactive move operation on the view.
 */
void zenith_view_begin_move(struct zenith_compositor *compositor,
                             struct zenith_view *view);

/**
 * Begin an interactive resize operation on the view.
 */
void zenith_view_begin_resize(struct zenith_compositor *compositor,
                               struct zenith_view *view, uint32_t edges);

#endif /* ZENITH_VIEW_H */
