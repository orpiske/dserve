#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdlib.h>


struct node_t_ {
	struct node_t_ *next;
	struct node_t_ *previous;
	const void *data;
};

typedef struct node_t_ node_t;

node_t *new_node(const void *ptr);
void set_previous(node_t *node, node_t *previous);
void set_next(node_t *node, node_t *next);

#endif
