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

#ifndef WSM_INPUT_H
#define WSM_INPUT_H

#include <stdbool.h>

#include <libinput.h>

#include <wayland-server-core.h>

struct wlr_input_device;
struct libinput_device;

struct wsm_input_device {
    char *identifier;
    struct wlr_input_device *wlr_device;
    struct wl_list link;
    struct wl_listener device_destroy;
    bool is_virtual;
    const struct wsm_input_device_impl *input_device_impl;
};

struct wsm_input_device_impl {
    enum libinput_config_status (*set_send_events)(struct libinput_device *device,
                                                   uint32_t mode);
    enum libinput_config_status (*set_tap)(struct libinput_device *device,
                                           enum libinput_config_tap_state tap);
    enum libinput_config_status (*set_tap_button_map)(struct libinput_device *device,
                                                      enum libinput_config_tap_button_map map);
    enum libinput_config_status (*set_tap_drag)(struct libinput_device *device,
                                                enum libinput_config_drag_state drag);
    enum libinput_config_status (*set_tap_drag_lock)(struct libinput_device *device,
                                                     enum libinput_config_drag_lock_state lock);
    enum libinput_config_status (*set_accel_speed)(struct libinput_device *device, double speed);
    enum libinput_config_status (*set_rotation_angle)(struct libinput_device *device, double angle);
    enum libinput_config_status (*set_accel_profile)(struct libinput_device *device,
                                                     enum libinput_config_accel_profile profile);
    enum libinput_config_status (*set_natural_scroll)(struct libinput_device *d, bool n);
    enum libinput_config_status (*set_left_handed)(struct libinput_device *device, bool left);
    enum libinput_config_status (*set_click_method)(struct libinput_device *device,
                                                    enum libinput_config_click_method method);
    enum libinput_config_status (*set_middle_emulation)(struct libinput_device *dev,
                                                        enum libinput_config_middle_emulation_state mid);
    enum libinput_config_status (*set_scroll_method)(struct libinput_device *device,
                                                     enum libinput_config_scroll_method method);
    enum libinput_config_status (*set_scroll_button)(struct libinput_device *dev, uint32_t button);
    enum libinput_config_status (*set_scroll_button_lock)(struct libinput_device *dev,
                                                          enum libinput_config_scroll_button_lock_state lock);
    enum libinput_config_status (*set_dwt)(struct libinput_device *device, bool dwt);
    enum libinput_config_status (*set_dwtp)(struct libinput_device *device, bool dwtp);
    enum libinput_config_status (*set_calibration_matrix)(struct libinput_device *dev, float mat[6]);
};

struct wsm_input_device* wsm_input_device_create();
struct wsm_input_device *input_wsm_device_from_wlr(
    struct wlr_input_device *device);
void wsm_input_device_destroy(struct wlr_input_device *wlr_device);
void wsm_input_configure_libinput_device_send_events(
    struct wsm_input_device *input_device);

#endif
