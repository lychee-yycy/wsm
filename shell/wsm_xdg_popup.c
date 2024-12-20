#include "wsm_log.h"
#include "wsm_xdg_popup.h"
#include "wsm_xdg_shell.h"
#include "wsm_workspace.h"
#include "wsm_output.h"
#include "node/wsm_node_descriptor.h"

#include <stdlib.h>

#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>

static void popup_handle_reposition(struct wl_listener *listener, void *data) {
	struct wsm_xdg_popup *popup = wl_container_of(listener, popup, reposition);
	wsm_xdg_popup_unconstrain(popup);
}

static void popup_handle_surface_commit(struct wl_listener *listener, void *data) {
	struct wsm_xdg_popup *popup = wl_container_of(listener, popup, surface_commit);
	if (popup->xdg_popup_wlr->base->initial_commit) {
		wsm_xdg_popup_unconstrain(popup);
	}
}

static void popup_handle_new_popup(struct wl_listener *listener, void *data) {
	struct wsm_xdg_popup *popup =
		wl_container_of(listener, popup, new_popup);
	struct wlr_xdg_popup *wlr_popup = data;
	wsm_xdg_popup_create(wlr_popup, popup->view, popup->xdg_surface_tree);
}

static void popup_handle_destroy(struct wl_listener *listener, void *data) {
	struct wsm_xdg_popup *popup = wl_container_of(listener, popup, destroy);

	wl_list_remove(&popup->new_popup.link);
	wl_list_remove(&popup->destroy.link);
	wl_list_remove(&popup->surface_commit.link);
	wl_list_remove(&popup->reposition.link);
	wlr_scene_node_destroy(&popup->scene_tree->node);
	free(popup);
}

struct wsm_xdg_popup *wsm_xdg_popup_create(struct wlr_xdg_popup *wlr_popup,
		struct wsm_view *view, struct wlr_scene_tree *parent) {
	struct wlr_xdg_surface *xdg_surface = wlr_popup->base;

	struct wsm_xdg_popup *popup = calloc(1, sizeof(struct wsm_xdg_popup));
	if (!popup) {
		wsm_log(WSM_ERROR, "Could not create wsm_xdg_popup: allocation failed!");
		return NULL;
	}

	popup->xdg_popup_wlr = wlr_popup;
	popup->view = view;
	popup->scene_tree = wlr_scene_tree_create(parent);
	if (!popup->scene_tree) {
		free(popup);

		return NULL;
	}

	popup->xdg_surface_tree = wlr_scene_xdg_surface_create(
		popup->scene_tree, xdg_surface);
	if (!popup->xdg_surface_tree) {
		wlr_scene_node_destroy(&popup->scene_tree->node);
		free(popup);
		return NULL;
	}

	popup->desc.relative = &view->content_tree->node;
	popup->desc.view = view;

	if (!wsm_scene_descriptor_assign(&popup->scene_tree->node,
			WSM_SCENE_DESC_POPUP, &popup->desc)) {
		wsm_log(WSM_ERROR, "Failed to allocate a popup scene descriptor");
		wlr_scene_node_destroy(&popup->scene_tree->node);
		free(popup);

		return NULL;
	}

	popup->xdg_popup_wlr = xdg_surface->popup;
	struct wsm_xdg_shell_view *shell_view =
		wl_container_of(view, shell_view, view);
	xdg_surface->data = shell_view;

	wl_signal_add(&xdg_surface->surface->events.commit, &popup->surface_commit);
	popup->surface_commit.notify = popup_handle_surface_commit;
	wl_signal_add(&xdg_surface->events.new_popup, &popup->new_popup);
	popup->new_popup.notify = popup_handle_new_popup;
	wl_signal_add(&wlr_popup->events.reposition, &popup->reposition);
	popup->reposition.notify = popup_handle_reposition;
	wl_signal_add(&wlr_popup->events.destroy, &popup->destroy);
	popup->destroy.notify = popup_handle_destroy;

	return popup;
}

void wsm_xdg_popup_unconstrain(struct wsm_xdg_popup *popup) {
	struct wsm_view *view = popup->view;
	struct wlr_xdg_popup *wlr_popup = popup->xdg_popup_wlr;

	struct wsm_workspace *workspace = view->container->pending.workspace;
	if (!workspace) {
		return;
	}

	struct wsm_output *output = workspace->output;
	struct wlr_box output_toplevel_sx_box = {
		.x = output->lx - view->container->pending.content_x + view->geometry.x,
		.y = output->ly - view->container->pending.content_y + view->geometry.y,
		.width = output->width,
		.height = output->height,
	};

	wlr_xdg_popup_unconstrain_from_box(wlr_popup, &output_toplevel_sx_box);
}
