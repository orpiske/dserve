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
