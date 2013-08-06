#include "list.h"

/**
 * Creates a new list
 * @return A new list_t object
 */
list_t *list_new(void) {
	list_t *ret = (list_t *) malloc(sizeof(list_t));

	if (!ret) {
		messenger msg = get_messenger();
		msg(ERROR, "Not enough memory to allocate for a new list");
	}
	else {
		ret->root = NULL;
		ret->current = NULL;
	}

	return ret;
}


inline static bool can_continue(unsigned int count, unsigned int position) {
	if (likely(count < position)) {
		return true;
	}

	return false;
}


static node_t *go_to(list_t *list, unsigned int position, unsigned int *count) {
	node_t *node = list->root;

	*count = 0;
	while (node && can_continue(*count, position)) {
		node = node->next;
		(*count)++;
	}

	return node;
}


static node_t *go_to_end(list_t *list) {
	node_t *node = list->root;

	while (node && node->next) {
		node = node->next;
	}

	return node;
}


/**
 * Returns the number of items in a list
 * @param list A given list to check the number of items
 * @return The number of items
 * @note The list must not be NULL
 */
unsigned int list_count(const list_t *list) {
	unsigned int count = 0;
	node_t *node = NULL;

	assert(list != NULL);

	node = list->root;
	while (node) {
		node = node->next;
		count++;
	}

	return count;
}


/**
 * Appends an item in the list
 * @param list The list to receive the new item
 * @param data The data for the new node
 * @return A pointer to the new node
 */
const node_t *list_append(list_t *list, const void *data) {
	node_t *last = go_to_end(list);
	node_t *node = NULL;

	node = new_node(data);

	if (!last) {
		list->root = node;
		list->current = node;
	}
	else {
		set_next(last, node);
		set_previous(node, last);
	}

	return node;
}


/**
 * Inserts an item in the list
 * @param list The list to receive the new item
 * @param data The data for the new node
 * @param position The position to insert the data
 * @return A pointer to the new node
 */
node_t *list_insert(list_t *list, const void *data, unsigned int position) {
	node_t *current = NULL;
	node_t *node = NULL;
	unsigned int count = 0;

	current = go_to(list, position, &count);
	if (count != position) {
		return NULL;
	}

	node = new_node(data);
	if (current = NULL) {
		list->root = node;
		list->current = node;
	}
	else {
		set_next(current->previous, node);
		set_previous(node, current->previous);

		set_previous(current, node);
		set_next(node, current);
	}

	return node;
}


/**
 * Removes an item from a list
 * @param list The list to remove the item
 * @param position The position of the item to be removed
 * @return A pointer to the removed node
 */
node_t *list_remove(list_t *list, unsigned int position) {
	node_t *node = NULL;
	unsigned int count = 0;

	node = go_to(list, position, &count);
	if (count != position) {
		return NULL;
	}

	if (node == NULL) {
		return node;
	}

	set_next(node->previous, node->next);
	set_previous(node->next, node->previous);
	list->current = node->previous;

	node->previous = NULL;
	node->next = NULL;

	return node;
}

/**
 * Removes a node that matches a given data as returned by comparable
 * @param list The list to remove the node from
 * @param comparable The comparison function (void *result will be NULL)
 * @param other The data to compare the node with
 * @return true if the node was removed or false if it was not found
 */
bool list_remove_item(list_t *list, compare_function_t comparable,
		const void *other)
{
	unsigned int i = 0;
	node_t *node = NULL;

	if (list == NULL) {
		return;
	}

	node = list->root;

	while (node) {
		bool ret = false;
		i++;

		ret = comparable(node->data, other, NULL);
		if (ret == true) {
			set_next(node->previous, node->next);
			set_previous(node->next, node->previous);
			list->current = node->previous;

			node->previous = NULL;
			node->next = NULL;

			free(node);

			return true;
		}

		node = node->next;
	}

	return false;
}


/**
 * Traverses the list comparing the data. It aborts if the uniqueness is true and
 * the comparison returns true
 * @param list
 * @param comparable
 * @param other
 * @param result
 */
void list_for_each_compare(list_t *list, bool uniqueness,
		compare_function_t comparable, const void *other, void *result)
{
	unsigned int i = 0;
	node_t *node = NULL;

	if (list == NULL) {
		return;
	}

	node = list->root;

	while (node) {
		bool ret = false;

		ret = comparable(node->data, other, result);
		if (ret == true && uniqueness == true) {
			return;
		}

		node = node->next;
	}
}


/**
 * Traverses the list executing a set of operations depending on the result of a
 * comparison function
 * @param list The list to be traversed
 * @param comparable A comparison function. If NULL, then defaults to true
 * @param true_handle A function handle for when comparable returns true
 * @param false_handle A function handle for when comparable returns false. If
 * NULL then it will be ignored
 * @param data Payload data for the true_handle and false_handle functions
 */
void list_for_each(list_t *list, handle_function_t handle, void *data)
{
	unsigned int i = 0;
	node_t *node = NULL;

	if (list == NULL) {
		return;
	}

	node = list->root;

	while (node) {
		bool result = true;

		handle(node->data, data);

		node = node->next;
	}
}



/**
 * Traverses the list comparing the data. It aborts if the uniqueness is true and
 * the comparison returns true
 * @param list
 * @param comparable
 * @param other
 * @param result
 */
const void *list_get_item(list_t *list, compare_function_t comparable, const void *other)
{
	unsigned int i = 0;
	node_t *node = NULL;

	if (list == NULL) {
		return NULL;
	}

	node = list->root;

	while (node) {
		bool ret = false;

		ret = comparable(node->data, other, NULL);
		if (ret == true) {
			return node->data;
		}

		node = node->next;
	}

	return NULL;
}
