#ifndef WSM_POINTER_H
#define WSM_POINTER_H

#include <stdbool.h>

struct wlr_pointer;

struct wsm_seat;
struct wsm_seat_device;

enum wsm_pointer_type {
	WSM_POINTER_TYPE_UNKNOWN,
	WSM_POINTER_TYPE_MOUSE,
	WSM_POINTER_TYPE_TOUCHPAD,
	WSM_POINTER_TYPE_TRACKBALL,
	WSM_POINTER_TYPE_JOYSTICK,
	WSM_POINTER_TYPE_POINTING_STICK,
};

struct wsm_pointer {
	struct wlr_pointer *pointer_wlr;
	struct wsm_seat_device *seat_device_wsm;

	struct {
		// mouse basic capability
		bool supports_left_handed;
		bool supports_middle_emulation;
		bool supports_pointer_speed;
		bool supports_pointer_acceleration;
		bool support_pointer_acceleration_profiles;
		bool supports_pointer_scroll_factor;
		bool supports_natural_scrol;

		bool supports_disable_while_typing;
		bool supports_gesture;
		bool supports_calibration_matrix;
		bool supports_disable_events;
		bool supports_disable_events_on_external_mouse;
		bool supports_scroll_methods;
		bool supports_click_methods;
	} capability;

	enum wsm_pointer_type pointer_type;
};

struct wsm_pointer *wsm_pointer_create(struct wsm_seat *seat,
	struct wsm_seat_device *device);
void wsm_pointer_destroy(struct wsm_pointer *pointer);

#endif
