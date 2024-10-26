#include "wsm_pointer.h"
#include "wsm_seat.h"
#include "wsm_log.h"
#include "wsm_server.h"

#include <stdlib.h>

#include <libudev.h>

#include <wlr/types/wlr_pointer.h>
#include <wlr/backend/libinput.h>

struct wsm_pointer *wsm_pointer_create(struct wsm_seat *seat,
	struct wsm_seat_device *device) {
	struct wsm_pointer *pointer =
		calloc(1, sizeof(struct wsm_pointer));
	if (!pointer) {
		wsm_log(WSM_ERROR, "Could not create wsm_pointer: allocation failed!");
		return NULL;
	}

	struct wlr_input_device *wlr_device = device->input_device->input_device_wlr;
	pointer->pointer_wlr = wlr_pointer_from_input_device(wlr_device);
	pointer->seat_device_wsm = device;
	device->pointer = pointer;

	if (wlr_input_device_is_libinput(wlr_device)) {
		struct libinput_device *libinput_device = wlr_libinput_get_device_handle(wlr_device);
		struct udev_device *udev_device = libinput_device_get_udev_device(libinput_device);
		if (udev_device_get_property_value(udev_device, "ID_INPUT_MOUSE")) {
			pointer->pointer_type = WSM_POINTER_TYPE_MOUSE;
		} else if (udev_device_get_property_value(udev_device, "ID_INPUT_TOUCHPAD")) {
			pointer->pointer_type = WSM_POINTER_TYPE_TOUCHPAD;
		} else if (udev_device_get_property_value(udev_device, "ID_INPUT_TRACKBALL")) {
			pointer->pointer_type = WSM_POINTER_TYPE_TRACKBALL;
		} else if (udev_device_get_property_value(udev_device, "ID_INPUT_POINTINGSTICK")) {
			pointer->pointer_type = WSM_POINTER_TYPE_POINTING_STICK;
		} else if (udev_device_get_property_value(udev_device, "ID_INPUT_JOYSTICK")) {
			pointer->pointer_type = WSM_POINTER_TYPE_JOYSTICK;
		} else {
			pointer->pointer_type = WSM_POINTER_TYPE_UNKNOWN;
		}

		pointer->capability.supports_left_handed =
			libinput_device_config_left_handed_is_available(libinput_device);
		pointer->capability.supports_middle_emulation =
			libinput_device_config_middle_emulation_is_available(libinput_device);
		pointer->capability.supports_pointer_speed = true;
		pointer->capability.supports_pointer_acceleration =
			libinput_device_config_accel_is_available(libinput_device);
		pointer->capability.support_pointer_acceleration_profiles = libinput_device_config_accel_get_profiles(libinput_device);
		pointer->capability.supports_pointer_scroll_factor = true;
		pointer->capability.supports_natural_scrol =
			libinput_device_config_scroll_has_natural_scroll(libinput_device);

		pointer->capability.supports_disable_while_typing =
			libinput_device_config_dwt_is_available(libinput_device);
		pointer->capability.supports_gesture =
			libinput_device_has_capability(libinput_device, LIBINPUT_DEVICE_CAP_GESTURE);
		pointer->capability.supports_calibration_matrix =
			libinput_device_config_calibration_has_matrix(libinput_device);
		pointer->capability.supports_disable_events =
			libinput_device_config_send_events_get_modes(libinput_device) & LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;
		pointer->capability.supports_disable_events_on_external_mouse =
			libinput_device_config_send_events_get_modes(libinput_device) & LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE;
		pointer->capability.supports_scroll_methods =
			libinput_device_config_scroll_get_methods(libinput_device);
		pointer->capability.supports_click_methods =
			libinput_device_config_click_get_methods(libinput_device);
	}

	return pointer;
}

void wsm_pointer_destroy(struct wsm_pointer *pointer) {
	if (!pointer) {
		return;
	}

	free(pointer);
}
