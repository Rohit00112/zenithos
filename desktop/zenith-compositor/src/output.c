/*
 * Zenith OS — Compositor
 *
 * output.c — Output (monitor/display) management
 *
 * Handles new outputs, mode configuration, and frame rendering
 * through the wlr_scene graph.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

#include "compositor.h"
#include "output.h"

/**
 * Called each time an output is ready to present a frame.
 * The scene graph handles all rendering; we just commit.
 */
static void handle_output_frame(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_output *output =
        wl_container_of(listener, output, frame);
    struct wlr_scene *scene = output->compositor->scene;

    struct wlr_scene_output *scene_output =
        wlr_scene_get_scene_output(scene, output->wlr_output);
    if (scene_output == NULL) {
        return;
    }

    /* Render the scene for this output and commit the frame */
    wlr_scene_output_commit(scene_output, NULL);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

/**
 * Called when the output requests a state change (e.g., mode, enabled).
 */
static void handle_output_request_state(struct wl_listener *listener,
                                         void *data) {
    struct zenith_output *output =
        wl_container_of(listener, output, request_state);
    const struct wlr_output_event_request_state *event = data;

    wlr_output_commit_state(output->wlr_output, event->state);
}

/**
 * Called when an output is destroyed (disconnected).
 */
static void handle_output_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct zenith_output *output =
        wl_container_of(listener, output, destroy);

    wlr_log(WLR_INFO, "Output '%s' destroyed", output->wlr_output->name);

    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->request_state.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->link);
    free(output);
}

/**
 * Called when a new output (monitor) is detected by the backend.
 *
 * This configures the output with its preferred mode and adds it
 * to the output layout.
 */
void zenith_output_new(struct wl_listener *listener, void *data) {
    struct zenith_compositor *compositor =
        wl_container_of(listener, compositor, new_output);
    struct wlr_output *wlr_output = data;

    wlr_log(WLR_INFO, "New output: %s (%s %s)",
            wlr_output->name,
            wlr_output->make ? wlr_output->make : "unknown",
            wlr_output->model ? wlr_output->model : "unknown");

    /* Initialize the output with the renderer */
    wlr_output_init_render(wlr_output, compositor->allocator,
                           compositor->renderer);

    /* Set the preferred mode */
    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);

    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode != NULL) {
        wlr_log(WLR_INFO, "Setting output mode: %dx%d@%dmHz",
                mode->width, mode->height, mode->refresh);
        wlr_output_state_set_mode(&state, mode);
    }

    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);

    /* Create our output wrapper */
    struct zenith_output *output = calloc(1, sizeof(*output));
    if (!output) {
        wlr_log(WLR_ERROR, "Failed to allocate output");
        return;
    }
    output->compositor = compositor;
    output->wlr_output = wlr_output;

    /* Listen for events */
    output->frame.notify = handle_output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    output->request_state.notify = handle_output_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);

    output->destroy.notify = handle_output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    wl_list_insert(&compositor->outputs, &output->link);

    /* Add output to the layout — auto-arranged */
    struct wlr_output_layout_output *lo =
        wlr_output_layout_add_auto(compositor->output_layout, wlr_output);

    /* Create a scene output for rendering */
    output->scene_output = wlr_scene_output_create(compositor->scene, wlr_output);
    wlr_scene_output_layout_add_output(compositor->scene_layout, lo,
                                       output->scene_output);
}
