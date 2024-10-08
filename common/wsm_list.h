#ifndef WSM_LIST_H
#define WSM_LIST_H

struct wsm_list {
	void **items;
	int capacity;
	int length;
};

struct wsm_list *create_list(void);
void list_free(struct wsm_list *list);
void list_add(struct wsm_list *list, void *item);
void list_insert(struct wsm_list *list, int index, void *item);
void list_del(struct wsm_list *list, int index);
void list_cat(struct wsm_list *list, struct wsm_list *source);
void list_qsort(struct wsm_list *list,
	int compare(const void *left, const void *right));
int list_seq_find(struct wsm_list *list,
	int compare(const void *item, const void *cmp_to), const void *cmp_to);
int list_find(struct wsm_list *list, const void *item);
void list_stable_sort(struct wsm_list *list,
	int compare(const void *a, const void *b));
void list_swap(struct wsm_list *list, int src, int dest);
void list_move_to_end(struct wsm_list *list, void *item);
void list_free_items_and_destroy(struct wsm_list *list);

#endif
