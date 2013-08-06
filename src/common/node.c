/**
 Copyright 2012 Otavio Rodolfo Piske
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

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
