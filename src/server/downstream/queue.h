#ifndef QUEUE_H_
#define QUEUE_H_

#include <pthread.h>

#include "options.h"

#include "node.h"
#include "list.h"
#include "entry.h"
#include "transaction.h"
#include "messenger.h"

typedef bool(*dispatcher)(const transaction_t *);

typedef struct transaction_dispatcher_t_ {
	transaction_t *transaction;
	dispatcher dispatcher_func;
} transaction_dispatcher_t;

void queue_add(const transaction_t *request, transaction_t *response);
void queue_status(const transaction_t *request, transaction_t *response,
		bool(*sender)(const transaction_t *));
void queue_pause(const transaction_t *request, transaction_t *response);
void queue_cancel(const transaction_t *request, transaction_t *response);
void queue_restart(const transaction_t *request, transaction_t *response);
void queue_save(const transaction_t *request, transaction_t *response);
bool queue_load(void);

// QUEUE_H_
#endif
