#include "node.h"


node_t *new_node(const void *ptr) {
	node_t *ret = (node_t *) malloc(sizeof(node_t));

	if (!ret) {
		fprintf(stderr, "Not enough memory to allocate for new node\n");
	}
	ret->previous = NULL;
	ret->next = NULL;
	ret->data = ptr;

	return ret;
}


void set_previous(node_t *node, node_t *previous) {
	if (!node) {
		return;
	}

	node->previous = previous;

	if (previous) {
		previous->next = node;
	}
}


void set_next(node_t *node, node_t *next) {
	if (!node) {
		return;
	}

	node->next = next;
	if (next) {
		next->previous = node;
	}
}
