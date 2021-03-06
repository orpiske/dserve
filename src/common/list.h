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

#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "node.h"
#include "util.h"
#include "messenger.h"

typedef struct list_t_ {
	node_t *root;
	node_t *current;
} list_t;

typedef bool(*compare_function_t)(const void *, const void *data, void *result);
typedef void(*handle_function_t)(const void *, void *);

list_t *list_new(void);

const node_t *list_append(list_t *list, const void *data);
node_t *list_insert(list_t *list, const void *data, unsigned int position);


node_t *list_remove(list_t *list, unsigned int position);
bool list_remove_item(list_t *list, compare_function_t comparable,
		const void *other);
const void *list_get(const list_t *list, unsigned int position);
unsigned int list_count(const list_t *list);


void list_for_each_compare(list_t *list, bool uniqueness,
		compare_function_t comparable, const void *compare, void *result);
void list_for_each(list_t *list, handle_function_t handle, void *data);

const void *list_get_item(list_t *list, compare_function_t comparable, const void *other);




#endif
