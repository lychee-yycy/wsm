#ifndef WSM_TABLET_H
#define WSM_TABLET_H

#include <wayland-server-core.h>

struct wlr_surface;
struct wlr_tablet_pad;
struct wlr_tablet_tool;
struct wlr_tablet_v2_tablet;
struct wlr_tablet_v2_tablet_tool;
struct wlr_tablet_v2_tablet_pad;
struct wlr_tablet_manager_v2;

struct wsm_seat;
struct wsm_seat_device;

struct wsm_tablet {
	struct wl_list link;
	struct wsm_seat_device *seat_device;
	struct wlr_tablet_v2_tablet *tablet_v2;
	struct wlr_tablet_manager_v2 *tablet_manager_v2;
};

enum wsm_tablet_tool_mode {
	WSM_TABLET_TOOL_MODE_ABSOLUTE,
	WSM_TABLET_TOOL_MODE_RELATIVE,
};

struct wsm_tablet_tool {
	struct wl_listener set_cursor;
	struct wl_listener tool_destroy;

	struct wsm_seat *seat;
	struct wsm_tablet *tablet;
	struct wlr_tablet_v2_tablet_tool *tablet_v2_tool;

	double tilt_x, tilt_y;
	enum wsm_tablet_tool_mode mode;
};

struct wsm_tablet_pad {
	struct wl_listener attach;
	struct wl_listener button;
	struct wl_listener ring;
	struct wl_listener strip;

	struct wl_listener surface_destroy;

	struct wl_listener tablet_destroy;

	struct wl_list link;
	struct wsm_seat_device *seat_device;
	struct wsm_tablet *tablet;
	struct wlr_tablet_pad *tablet_pad_wlr;
	struct wlr_tablet_v2_tablet_pad *tablet_v2_pad;

	struct wlr_surface *current_surface;
};

struct wsm_tablet *wsm_tablet_create(struct wsm_seat *seat,
	struct wsm_seat_device *device);

void wsm_configure_tablet(struct wsm_tablet *tablet);
void wsm_tablet_destroy(struct wsm_tablet *tablet);

void wsm_tablet_tool_configure(struct wsm_tablet *tablet,
	struct wlr_tablet_tool *wlr_tool);

struct wsm_tablet_pad *wsm_tablet_pad_create(struct wsm_seat *seat,
	struct wsm_seat_device *device);

void wsm_configure_tablet_pad(struct wsm_tablet_pad *tablet_pad);

void wsm_tablet_pad_destroy(struct wsm_tablet_pad *tablet_pad);

void wsm_tablet_pad_set_focus(struct wsm_tablet_pad *tablet_pad,
	struct wlr_surface *surface);

#endif
