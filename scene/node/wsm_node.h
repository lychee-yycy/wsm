#ifndef WSM_NODE_H
#define WSM_NODE_H

#include "wsm_list.h"

#include <stddef.h>
#include <stdbool.h>

#include <wayland-server-core.h>

struct wlr_box;
struct wlr_scene_tree;

struct wsm_root;
struct wsm_output;
struct wsm_workspace;
struct wsm_container;
struct wsm_transaction_instruction;

enum wsm_node_type {
	N_ROOT,
	N_OUTPUT,
	N_WORKSPACE,
	N_CONTAINER,
};

struct wsm_node {
	struct {
		struct wl_signal destroy;
	} events;

	union {
		struct wsm_root *root;
		struct wsm_output *output;
		struct wsm_workspace *workspace;
		struct wsm_container *container;
	};

	size_t id;
	size_t ntxnrefs;
	struct wsm_transaction_instruction *instruction;

	enum wsm_node_type type;
	bool destroying;
	bool dirty;
};

void node_init(struct wsm_node *node, enum wsm_node_type type, void *thing);
const char *node_type_to_str(enum wsm_node_type type);
void node_set_dirty(struct wsm_node *node);
bool node_is_view(struct wsm_node *node);
char *node_get_name(struct wsm_node *node);
void node_get_box(struct wsm_node *node, struct wlr_box *box);
struct wsm_output *node_get_output(struct wsm_node *node);
enum wsm_container_layout node_get_layout(struct wsm_node *node);
struct wsm_node *node_get_parent(struct wsm_node *node);
struct wsm_list *node_get_children(struct wsm_node *node);
void scene_node_disown_children(struct wlr_scene_tree *tree);
bool node_has_ancestor(struct wsm_node *node, struct wsm_node *ancestor);
struct wlr_scene_tree *alloc_scene_tree(struct wlr_scene_tree *parent,
	bool *failed);

#endif
