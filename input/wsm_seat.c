/*
MIT License

Copyright (c) 2024 YaoBing Xiao

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#define _POSIX_C_SOURCE 200809L
#include "wsm_seat.h"
#include "wsm_server.h"
#include "wsm_scene.h"
#include "wsm_log.h"
#include "wsm_switch.h"
#include "wsm_cursor.h"
#include "wsm_keyboard.h"
#include "wsm_tablet.h"
#include "wsm_input_manager.h"
#include "wsm_seatop.h"
#include "wsm_node_descriptor.h"

#include <stdlib.h>
#include <assert.h>

#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_touch.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_idle_notify_v1.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_primary_selection.h>

static void handle_request_start_drag(struct wl_listener *listener,
                                      void *data) {
    struct wsm_seat *seat = wl_container_of(listener, seat, request_start_drag);
    struct wlr_seat_request_start_drag_event *event = data;

    if (wlr_seat_validate_pointer_grab_serial(seat->wlr_seat,
                                              event->origin, event->serial)) {
        wlr_seat_start_pointer_drag(seat->wlr_seat, event->drag, event->serial);
        return;
    }

    struct wlr_touch_point *point;
    if (wlr_seat_validate_touch_grab_serial(seat->wlr_seat,
                                            event->origin, event->serial, &point)) {
        wlr_seat_start_touch_drag(seat->wlr_seat,
                                  event->drag, event->serial, point);
        return;
    }

    wsm_log(WSM_DEBUG, "Ignoring start_drag request: "
                         "could not validate pointer or touch serial %" PRIu32, event->serial);
    wlr_data_source_destroy(event->drag->source);
}

static void drag_handle_destroy(struct wl_listener *listener, void *data) {
    struct wsm_drag *drag = wl_container_of(listener, drag, destroy);

    struct wsm_seat *seat = drag->seat;
    if (seat->focused_layer) {
        struct wlr_layer_surface_v1 *layer = seat->focused_layer;
        seat_set_focus_layer(seat, NULL);
        seat_set_focus_layer(seat, layer);
    } else {
        struct wlr_surface *surface = seat->wlr_seat->keyboard_state.focused_surface;
        seat_set_focus_surface(seat, NULL, false);
        seat_set_focus_surface(seat, surface, false);
    }

    drag->wlr_drag->data = NULL;
    wl_list_remove(&drag->destroy.link);
    free(drag);
}

static void drag_icon_update_position(struct wsm_seat *seat, struct wlr_scene_node *node) {
    struct wlr_drag_icon *wlr_icon = wsm_scene_descriptor_try_get(node, WSM_SCENE_DESC_DRAG_ICON);
    struct wlr_cursor *cursor = seat->wsm_cursor->wlr_cursor;

    switch (wlr_icon->drag->grab_type) {
    case WLR_DRAG_GRAB_KEYBOARD:
        return;
    case WLR_DRAG_GRAB_KEYBOARD_POINTER:
        wlr_scene_node_set_position(node, cursor->x, cursor->y);
        break;
    case WLR_DRAG_GRAB_KEYBOARD_TOUCH:;
        struct wlr_touch_point *point =
            wlr_seat_touch_get_point(seat->wlr_seat, wlr_icon->drag->touch_id);
        if (point == NULL) {
            return;
        }
        wlr_scene_node_set_position(node, seat->touch_x, seat->touch_y);
    }
}

static void handle_start_drag(struct wl_listener *listener, void *data) {
    struct wsm_seat *seat = wl_container_of(listener, seat, start_drag);
    struct wlr_drag *wlr_drag = data;

    struct wsm_drag *drag = calloc(1, sizeof(struct wsm_drag));
    if (!wsm_assert(drag, "Could not create wsm_drag: allocation failed!")) {
        return;
    }
    drag->seat = seat;
    drag->wlr_drag = wlr_drag;
    wlr_drag->data = drag;

    drag->destroy.notify = drag_handle_destroy;
    wl_signal_add(&wlr_drag->events.destroy, &drag->destroy);

    struct wlr_drag_icon *wlr_drag_icon = wlr_drag->icon;
    if (wlr_drag_icon != NULL) {
        struct wlr_scene_tree *tree = wlr_scene_drag_icon_create(seat->drag_icons, wlr_drag_icon);
        if (!tree) {
            wsm_log(WSM_ERROR, "Failed to allocate a drag icon scene tree");
            return;
        }

        if (!wsm_scene_descriptor_assign(&tree->node, WSM_SCENE_DESC_DRAG_ICON,
                                        wlr_drag_icon)) {
            wsm_log(WSM_ERROR, "Failed to allocate a drag icon scene descriptor");
            wlr_scene_node_destroy(&tree->node);
            return;
        }

        drag_icon_update_position(seat, &tree->node);
    }
    wsm_seatop_begin(seat);
}

static void handle_request_set_selection(struct wl_listener *listener,
                                         void *data) {
    struct wsm_seat *seat =
        wl_container_of(listener, seat, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(seat->wlr_seat, event->source, event->serial);
}

static void handle_request_set_primary_selection(struct wl_listener *listener,
                                                 void *data) {
    struct wsm_seat *seat =
        wl_container_of(listener, seat, request_set_primary_selection);
    struct wlr_seat_request_set_primary_selection_event *event = data;
    wlr_seat_set_primary_selection(seat->wlr_seat, event->source, event->serial);
}

struct wsm_seat *seat_create(const char *seat_name) {
    struct wsm_seat *seat = calloc(1, sizeof(struct wsm_seat));
    if (!wsm_assert(seat, "Could not create wsm_seat: allocation failed!")) {
        return NULL;
    }

    seat->wlr_seat = wlr_seat_create(global_server.wl_display, seat_name);
    seat->drag_icons = wlr_scene_tree_create(&global_server.wsm_scene->wlr_scene->tree);

    if (!wsm_assert(seat->wlr_seat, "Could not create wlr_seat: create failed!")) {
        return NULL;
    }
    wl_list_init(&seat->devices);
    wl_list_init(&seat->keyboard_groups);
    seat->wlr_seat->data = seat;

    seat->wsm_cursor = wsm_cursor_create(&global_server, seat);
    if (!wsm_assert(seat->wsm_cursor, "wsm_cursor is NULL!")) {
        wlr_seat_destroy(seat->wlr_seat);
        free(seat);
        return NULL;
    }

    wl_list_insert(&global_server.wsm_input_manager->seats, &seat->link);

    wl_list_init(&seat->focus_stack);
    wl_list_init(&seat->devices);

    seat->request_start_drag.notify = handle_request_start_drag;
    wl_signal_add(&seat->wlr_seat->events.request_start_drag,
                  &seat->request_start_drag);

    seat->start_drag.notify = handle_start_drag;
    wl_signal_add(&seat->wlr_seat->events.start_drag, &seat->start_drag);

    seat->request_set_selection.notify = handle_request_set_selection;
    wl_signal_add(&seat->wlr_seat->events.request_set_selection,
                  &seat->request_set_selection);

    seat->request_set_primary_selection.notify =
        handle_request_set_primary_selection;
    wl_signal_add(&seat->wlr_seat->events.request_set_primary_selection,
                  &seat->request_set_primary_selection);

    wl_list_init(&seat->keyboard_groups);
    wl_list_init(&seat->keyboard_shortcuts_inhibitors);

    wsm_seatop_begin(seat);

    return seat;
}

static void seat_device_destroy(struct wsm_seat_device *seat_device) {
    if (!seat_device) {
        wsm_log(WSM_ERROR, "wsm_seat_device is NULL!");
        return;
    }

    wsm_keyboard_destroy(seat_device->keyboard);
    wsm_tablet_destroy(seat_device->tablet);
    wsm_tablet_pad_destroy(seat_device->tablet_pad);
    wsm_switch_destroy(seat_device->switch_device);
    wlr_cursor_detach_input_device(seat_device->wsm_seat->wsm_cursor->wlr_cursor,
                                   seat_device->input_device->wlr_device);
    wl_list_remove(&seat_device->link);
    free(seat_device);
}

static struct wsm_seat_device *seat_get_device(struct wsm_seat *seat,
                                                struct wsm_input_device *input_device) {
    struct wsm_seat_device *seat_device = NULL;
    wl_list_for_each(seat_device, &seat->devices, link) {
        if (seat_device->input_device == input_device) {
            return seat_device;
        }
    }

    struct wsm_keyboard_group *group = NULL;
    wl_list_for_each(group, &seat->keyboard_groups, link) {
        if (group->seat_device->input_device == input_device) {
            return group->seat_device;
        }
    }

    return NULL;
}

static void seat_update_capabilities(struct wsm_seat *seat) {
    uint32_t caps = 0;
    uint32_t previous_caps = seat->wlr_seat->capabilities;
    struct wsm_seat_device *seat_device;
    wl_list_for_each(seat_device, &seat->devices, link) {
        switch (seat_device->input_device->wlr_device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            caps |= WL_SEAT_CAPABILITY_KEYBOARD;
            break;
        case WLR_INPUT_DEVICE_POINTER:
            caps |= WL_SEAT_CAPABILITY_POINTER;
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            caps |= WL_SEAT_CAPABILITY_TOUCH;
            break;
        case WLR_INPUT_DEVICE_TABLET_TOOL:
            caps |= WL_SEAT_CAPABILITY_POINTER;
            break;
        case WLR_INPUT_DEVICE_SWITCH:
        case WLR_INPUT_DEVICE_TABLET_PAD:
            break;
        }
    }

    if ((caps & WL_SEAT_CAPABILITY_POINTER) == 0) {
        cursor_set_image(seat->wsm_cursor, NULL, NULL);
        wlr_seat_set_capabilities(seat->wlr_seat, caps);
    } else {
        wlr_seat_set_capabilities(seat->wlr_seat, caps);
        if ((previous_caps & WL_SEAT_CAPABILITY_POINTER) == 0) {
            cursor_set_image(seat->wsm_cursor, "default", NULL);
        }
    }
}

void seat_remove_device(struct wsm_seat *seat,
                        struct wsm_input_device *input_device) {
    struct wsm_seat_device *seat_device = seat_get_device(seat, input_device);

    if (!seat_device) {
        wsm_log(WSM_ERROR, "wsm_seat_device is NULL in seat_remove_device!");
        return;
    }

    wsm_log(WSM_DEBUG, "removing device %s from seat %s",
             input_device->identifier, seat->wlr_seat->name);

    seat_device_destroy(seat_device);
    seat_update_capabilities(seat);
}

void seat_idle_notify_activity(struct wsm_seat *seat,
                               enum wlr_input_device_type source) {
    if ((source & seat->idle_inhibit_sources) == 0) {
        return;
    }
    wlr_idle_notifier_v1_notify_activity(global_server.idle_notifier_v1, seat->wlr_seat);
}

void seatop_tablet_tool_tip(struct wsm_seat *seat,
                            struct wsm_tablet_tool *tool, uint32_t time_msec,
                            enum wlr_tablet_tool_tip_state state) {
    if (seat->seatop_impl->tablet_tool_tip) {
        seat->seatop_impl->tablet_tool_tip(seat, tool, time_msec, state);
    }
}

void seatop_hold_begin(struct wsm_seat *seat,
                       struct wlr_pointer_hold_begin_event *event) {
    if (seat->seatop_impl->hold_begin) {
        seat->seatop_impl->hold_begin(seat, event);
    }
}

void seatop_hold_end(struct wsm_seat *seat,
                     struct wlr_pointer_hold_end_event *event) {
    if (seat->seatop_impl->hold_end) {
        seat->seatop_impl->hold_end(seat, event);
    }
}

void seatop_pinch_begin(struct wsm_seat *seat,
                        struct wlr_pointer_pinch_begin_event *event) {
    if (seat->seatop_impl->pinch_begin) {
        seat->seatop_impl->pinch_begin(seat, event);
    }
}

void seatop_pinch_update(struct wsm_seat *seat,
                         struct wlr_pointer_pinch_update_event *event) {
    if (seat->seatop_impl->pinch_update) {
        seat->seatop_impl->pinch_update(seat, event);
    }
}

void seatop_pinch_end(struct wsm_seat *seat,
                      struct wlr_pointer_pinch_end_event *event) {
    if (seat->seatop_impl->pinch_end) {
        seat->seatop_impl->pinch_end(seat, event);
    }
}

void seatop_swipe_begin(struct wsm_seat *seat,
                        struct wlr_pointer_swipe_begin_event *event) {
    if (seat->seatop_impl->swipe_begin) {
        seat->seatop_impl->swipe_begin(seat, event);
    }
}

void seatop_swipe_update(struct wsm_seat *seat,
                         struct wlr_pointer_swipe_update_event *event) {
    if (seat->seatop_impl->swipe_update) {
        seat->seatop_impl->swipe_update(seat, event);
    }
}

void seatop_swipe_end(struct wsm_seat *seat,
                      struct wlr_pointer_swipe_end_event *event) {
    if (seat->seatop_impl->swipe_end) {
        seat->seatop_impl->swipe_end(seat, event);
    }
}

void seatop_pointer_motion(struct wsm_seat *seat, uint32_t time_msec) {
    if (seat->seatop_impl->pointer_motion) {
        seat->seatop_impl->pointer_motion(seat, time_msec);
    }
}

void seatop_pointer_axis(struct wsm_seat *seat,
                         struct wlr_pointer_axis_event *event) {
    if (seat->seatop_impl->pointer_axis) {
        seat->seatop_impl->pointer_axis(seat, event);
    }
}

void seatop_button(struct wsm_seat *seat, uint32_t time_msec,
                   struct wlr_input_device *device, uint32_t button,
                   enum wlr_button_state state) {
    if (seat->seatop_impl->button) {
        seat->seatop_impl->button(seat, time_msec, device, button, state);
    }
}

void seatop_touch_motion(struct wsm_seat *seat, struct wlr_touch_motion_event *event,
                         double lx, double ly) {
    if (seat->seatop_impl->touch_motion) {
        seat->seatop_impl->touch_motion(seat, event, lx, ly);
    }
}

void seatop_touch_up(struct wsm_seat *seat, struct wlr_touch_up_event *event) {
    if (seat->seatop_impl->touch_up) {
        seat->seatop_impl->touch_up(seat, event);
    }
}

void seatop_touch_down(struct wsm_seat *seat, struct wlr_touch_down_event *event,
                       double lx, double ly) {
    if (seat->seatop_impl->touch_down) {
        seat->seatop_impl->touch_down(seat, event, lx, ly);
    }
}

void seatop_touch_cancel(struct wsm_seat *seat, struct wlr_touch_cancel_event *event) {
    if (seat->seatop_impl->touch_cancel) {
        seat->seatop_impl->touch_cancel(seat, event);
    }
}

void seatop_rebase(struct wsm_seat *seat, uint32_t time_msec) {
    if (seat->seatop_impl->rebase) {
        seat->seatop_impl->rebase(seat, time_msec);
    }
}

void seatop_end(struct wsm_seat *seat) {
    if (seat->seatop_impl && seat->seatop_impl->end) {
        seat->seatop_impl->end(seat);
    }
    free(seat->seatop_data);
    seat->seatop_data = NULL;
    seat->seatop_impl = NULL;
}

void seatop_tablet_tool_motion(struct wsm_seat *seat,
                               struct wsm_tablet_tool *tool, uint32_t time_msec) {
    if (seat->seatop_impl->tablet_tool_motion) {
        seat->seatop_impl->tablet_tool_motion(seat, tool, time_msec);
    } else {
        seatop_pointer_motion(seat, time_msec);
    }
}

bool seatop_allows_set_cursor(struct wsm_seat *seat) {
    return seat->seatop_impl->allow_set_cursor;
}

void seat_add_device(struct wsm_seat *seat,
                     struct wsm_input_device *input_device) {
    if (seat_get_device(seat, input_device)) {
        wsm_log(WSM_ERROR, "device %s already exists in seat %s",
                input_device->identifier, seat->wlr_seat->name);
        return;
    }

    struct wsm_seat_device *seat_device =
        calloc(1, sizeof(struct wsm_seat_device));
    if (!seat_device) {
        wsm_log(WSM_ERROR, "Could not create wsm_seat_device: allocation failed!");
        return;
    }

    wsm_log(WSM_DEBUG, "adding device %s to seat %s",
             input_device->identifier, seat->wlr_seat->name);

    seat_device->wsm_seat = seat;
    seat_device->input_device = input_device;
    wl_list_insert(&seat->devices, &seat_device->link);

    seat_configure_device(seat, input_device);
    seat_update_capabilities(seat);
}

static void seat_keyboard_notify_enter(struct wsm_seat *seat,
                                       struct wlr_surface *surface) {
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat->wlr_seat);
    if (!keyboard) {
        wlr_seat_keyboard_notify_enter(seat->wlr_seat, surface, NULL, 0, NULL);
        return;
    }

    struct wsm_keyboard *wsm_keyboard =
        wsm_keyboard_for_wlr_keyboard(seat, keyboard);
    wsm_assert(!wsm_keyboard, "Cannot find wsm_keyboard for seat keyboard");

    struct wsm_shortcut_state *state = &wsm_keyboard->state_pressed_sent;
    wlr_seat_keyboard_notify_enter(seat->wlr_seat, surface,
                                   state->pressed_keycodes, state->npressed, &keyboard->modifiers);
}

static void seat_configure_pointer(struct wsm_seat *seat,
                                   struct wsm_seat_device *wsm_device) {
    wlr_cursor_attach_input_device(seat->wsm_cursor->wlr_cursor,
                                   wsm_device->input_device->wlr_device);
}

static void seat_configure_keyboard(struct wsm_seat *seat,
                                    struct wsm_seat_device *seat_device) {
    if (!seat_device->keyboard) {
        wsm_keyboard_create(seat, seat_device);
    }
    wsm_keyboard_configure(seat_device->keyboard);

    // We only need to update the current keyboard, as the rest will be updated
    // as they are activated.
    struct wlr_keyboard *wlr_keyboard =
        wlr_keyboard_from_input_device(seat_device->input_device->wlr_device);
    struct wlr_keyboard *current_keyboard = seat->wlr_seat->keyboard_state.keyboard;
    if (wlr_keyboard != current_keyboard) {
        return;
    }

    // force notify reenter to pick up the new configuration.  This reuses
    // the current focused surface to avoid breaking input grabs.
    struct wlr_surface *surface = seat->wlr_seat->keyboard_state.focused_surface;
    if (surface) {
        wlr_seat_keyboard_notify_clear_focus(seat->wlr_seat);
        seat_keyboard_notify_enter(seat, surface);
    }
}

static void seat_configure_switch(struct wsm_seat *seat,
                                  struct wsm_seat_device *seat_device) {
    if (!seat_device->switch_device) {
        wsm_switch_create(seat, seat_device);
    }
    wsm_switch_configure(seat_device->switch_device);
}

static void seat_configure_touch(struct wsm_seat *seat,
                                 struct wsm_seat_device *wsm_device) {
    wlr_cursor_attach_input_device(seat->wsm_cursor->wlr_cursor,
                                   wsm_device->input_device->wlr_device);
}

static void seat_configure_tablet_tool(struct wsm_seat *seat,
                                       struct wsm_seat_device *wsm_device) {
    if (!wsm_device->tablet) {
        wsm_device->tablet = wsm_tablet_create(seat, wsm_device);
    }
    wsm_configure_tablet(wsm_device->tablet);
    wlr_cursor_attach_input_device(seat->wsm_cursor->wlr_cursor,
                                   wsm_device->input_device->wlr_device);
}

static void seat_configure_tablet_pad(struct wsm_seat *seat,
                                      struct wsm_seat_device *wsm_device) {
    if (!wsm_device->tablet_pad) {
        wsm_device->tablet_pad = wsm_tablet_pad_create(seat, wsm_device);
    }
    wsm_configure_tablet_pad(wsm_device->tablet_pad);
}

void seat_configure_device(struct wsm_seat *seat, struct wsm_input_device *device) {
    struct wsm_seat_device *seat_device = seat_get_device(seat, device);
    if (!seat_device) {
        _wsm_log(WSM_ERROR, "seat_get_device is NULL!");
        return;
    }

    switch (device->wlr_device->type) {
    case WLR_INPUT_DEVICE_POINTER:
        seat_configure_pointer(seat, seat_device);
        break;
    case WLR_INPUT_DEVICE_KEYBOARD:
        seat_configure_keyboard(seat, seat_device);
        break;
    case WLR_INPUT_DEVICE_SWITCH:
        seat_configure_switch(seat, seat_device);
        break;
    case WLR_INPUT_DEVICE_TOUCH:
        seat_configure_touch(seat, seat_device);
        break;
    case WLR_INPUT_DEVICE_TABLET_TOOL:
        seat_configure_tablet_tool(seat, seat_device);
        break;
    case WLR_INPUT_DEVICE_TABLET_PAD:
        seat_configure_tablet_pad(seat, seat_device);
        break;
    }
}

static void seat_tablet_pads_set_focus(struct wsm_seat *seat,
                                       struct wlr_surface *surface) {
    struct wsm_seat_device *seat_device;
    wl_list_for_each(seat_device, &seat->devices, link) {
        wsm_tablet_pad_set_focus(seat_device->tablet_pad, surface);
    }
}

void seat_set_focus_surface(struct wsm_seat *seat,
                            struct wlr_surface *surface, bool unfocus) {
    if (seat->has_focus && unfocus) {
        seat->has_focus = false;
    }

    if (surface) {
        seat_keyboard_notify_enter(seat, surface);
    } else {
        wlr_seat_keyboard_notify_clear_focus(seat->wlr_seat);
    }

    seat_tablet_pads_set_focus(seat, surface);
}

void seat_set_focus_layer(struct wsm_seat *seat,
                          struct wlr_layer_surface_v1 *layer) {
    if (!layer && seat->focused_layer) {
        return;
    } else if (!layer) {
        return;
    }
    assert(layer->surface->mapped);
    if (layer->current.layer >= ZWLR_LAYER_SHELL_V1_LAYER_TOP &&
        layer->current.keyboard_interactive
            == ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE) {
        seat->has_exclusive_layer = true;
    }
    if (seat->focused_layer == layer) {
        return;
    }
    seat_set_focus_surface(seat, layer->surface, true);
    seat->focused_layer = layer;
}

void drag_icons_update_position(struct wsm_seat *seat) {
    struct wlr_scene_node *node;
    wl_list_for_each(node, &seat->drag_icons->children, link) {
        drag_icon_update_position(seat, node);
    }
}

void seat_pointer_notify_button(struct wsm_seat *seat, uint32_t time_msec,
                                uint32_t button, enum wlr_button_state state) {
    seat->last_button_serial = wlr_seat_pointer_notify_button(seat->wlr_seat,
                                                              time_msec, button, state);
}
