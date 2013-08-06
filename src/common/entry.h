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

#ifndef ENTRY_H
#define ENTRY_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "transaction.h"

// TODO: Make thread safe.

typedef enum entry_status_t_ {
	WAITING,
	STARTING,
	STARTED,
	PAUSED,
	CANCELLED,
	COMPLETED,
} entry_status_t;

typedef struct entry_time_t_ {
	time_t start;
	time_t measured;
} entry_time_t;

typedef struct entry_t_ {
	trx_id_t id;
	entry_status_t status;
	char url[MAX_URL_LEN];
	double size;
	double remaining;
	double complete;
	double speed;
	entry_time_t time;
} entry_t;


static entry_t *entry_new() {
	entry_t *ret = (entry_t *) malloc(sizeof(entry_t));

	if (ret == NULL) {
		fprintf(stderr, "Unable to allocate memory for new entry\n");

		return NULL;
	}

	bzero(ret, sizeof(entry_t));
	ret->status = WAITING;
	return ret;
}


static entry_t *delete_entry(entry_t **ptr) {
	entry_t *entry = (*ptr);

	free(entry);
	return NULL;
}


static void entry_set_target(entry_t *entry, const transaction_t *request) {
	memcpy(&entry->id, &request->id, sizeof(request->id));
	memcpy(&entry->url, request->payload.data, sizeof(request->payload.data));
}

#endif
