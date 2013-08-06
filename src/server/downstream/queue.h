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
