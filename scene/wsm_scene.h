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

#ifndef WSM_SCENE_H
#define WSM_SCENE_H

#include "../config.h"
#include "node/wsm_node.h"

#include <stdbool.h>

#include <wlr/types/wlr_scene.h>

struct wlr_scene;
struct wlr_scene_tree;
struct wlr_scene_output;
struct wlr_output_state;
struct wlr_output_layout;
struct wlr_scene_output_layout;
struct wlr_scene_output_state_options;

struct wsm_server;

/**
 * @brief scene render control.
 *
 * @details The order in which the scene-trees below are created determines the
 * z-order for nodes which cover the whole work-area.  For per-output scene-trees.
 *
 * | Type              | isWindow          | Scene Tree       | Per Output | Example
 * | ----------------- | ----------------- | ---------------- | ---------- | -------------------------------
 * | server-node       | No(display node)  | black-screen     | Yes        | black screen transition
 * | server-node       | No(display node)  | screen-water-mark| Yes        | show employee number information
 * | server-node       | No(display node)  | drag display node| No         | show drag icon
 * | layer-shell       | No(display node)  | osd_overlay_layer| No         | screenshot preview window
 * | xdg-popups        | Yes               | xdg-popups(layer)| Yes        | server decides layer surface popup(wlr_xdg_popup)
 * | layer-shell       | Yes               | notify-osd       | Yes        | notify bubble or osd
 * | layer-shell       | Yes               | overlay-layer    | Yes        | lock-screen
 * | xdg-popups        | Yes               | xdg-popups       | No         | server decides(wlr_xdg_popup)
 * | xwayland-OR       | Yes               | unmanaged        | No         | like Qt::X11BypassWindowManagerHint,override_redirect appear above
 * | layer-shell       | Yes               | top-layer        | Yes        | launcher、dock、fullscreen window
 * | toplevels windows | Yes               | always-on-top    | No         | X or Wayland.like Qt::WindowStaysOnTopHint
 * | toplevels windows | Yes               | normal           | No         | X or Wayland.normal window
 * | toplevels windows | Yes               | always-on-bottom | No         | X or Waylandlike Qt::WindowStaysOnBottomHint
 * | layer-shell       | Yes               | bottom-layer     | Yes        | placeholder for custom desktop widget
 * | layer-shell       | Yes               | background-layer | Yes        | desktop
 *
 */
struct wsm_scene {
	struct {
		struct wlr_scene_tree *shell_background;
		struct wlr_scene_tree *shell_bottom;
		struct wlr_scene_tree *tiling;
		struct wlr_scene_tree *floating;
		struct wlr_scene_tree *shell_top;
		struct wlr_scene_tree *fullscreen;
		struct wlr_scene_tree *fullscreen_global;
#ifdef HAVE_XWAYLAND
		struct wlr_scene_tree *unmanaged;
#endif
		struct wlr_scene_tree *shell_overlay;
		struct wlr_scene_tree *popup;
		struct wlr_scene_tree *seat;
		struct wlr_scene_tree *session_lock;
	} layers;

	struct wsm_node node;

	struct {
		struct wl_signal new_node;
	} events;

	struct wl_list all_outputs;

	struct wlr_output_layout *output_layout;

	struct wlr_scene *root_scene;
	struct wlr_scene_tree *staging;

	struct wlr_scene_tree *layer_tree;

	struct wsm_list *outputs;
	struct wsm_list *non_desktop_outputs;
	struct wsm_list *scratchpad;

	struct wsm_output *fallback_output;
	struct wsm_container *fullscreen_global;

	double x, y;
	double width, height;
};

enum wsm_scene_node_type {
	WLR_SCENE_NODE_TREE_FALLBACK,
	WLR_SCENE_NODE_RECT_FALLBACK,
	WLR_SCENE_NODE_BUFFER_FALLBACK,
	WSM_SCENE_NODE_RECT,
};

struct wsm_border {
	float color[4];
	int width;
};

struct wsm_radius_extra {
	int top_left_radius;
	int top_right_radius;
	int bottom_left_radius;
	int bottom_right_radius;
};

struct wsm_scene_rect {
	struct wlr_scene_node node;
	float color[4];
	struct wsm_border border;
	struct wsm_radius_extra radius_extra;
	int width, height, radius;
};

struct wsm_scene *wsm_scene_create(const struct wsm_server* server);
bool wsm_scene_output_commit(struct wlr_scene_output *scene_output,
	const struct wlr_scene_output_state_options *options);
bool wsm_scene_output_build_state(struct wlr_scene_output *scene_output,
	struct wlr_output_state *state, const struct wlr_scene_output_state_options *options);
void root_get_box(struct wsm_scene *root, struct wlr_box *box);
void root_scratchpad_show(struct wsm_container *con);

void scene_node_init(struct wlr_scene_node *node,
	enum wlr_scene_node_type type, struct wlr_scene_tree *parent);
void scene_node_update(struct wlr_scene_node *node,
	pixman_region32_t *damage);
struct wlr_scene *scene_node_get_root(struct wlr_scene_node *node);

struct wsm_scene_rect *wsm_scene_rect_create(struct wlr_scene_tree *parent,
	int width, int height, int radius, const float color[static 4],
	const struct wsm_border border, const struct wsm_radius_extra radius_extra);
void wsm_scene_rect_set_size(struct wsm_scene_rect *rect, int width, int height);
void wsm_scene_rect_set_color(struct wsm_scene_rect *rect, const float color[static 4]);
void wsm_scene_rect_set_border_width(struct wsm_scene_rect *rect, int width);
void wsm_scene_rect_set_border_color(struct wsm_scene_rect *rect, const float color[static 4]);
void wsm_scene_rect_set_radius(struct wsm_scene_rect *rect, int radius);
void wsm_scene_rect_set_top_left_radius(struct wsm_scene_rect *rect, int radius);
void wsm_scene_rect_set_top_right_radius(struct wsm_scene_rect *rect, int radius);
void wsm_scene_rect_set_bottom_left_radius(struct wsm_scene_rect *rect, int radius);
void wsm_scene_rect_set_bottom_right_radius(struct wsm_scene_rect *rect, int radius);

#endif
