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

#include "transaction.h"

void set_transaction(transaction_t *dest, command_t command, status_t status,
		const char *name, const char *fmt, ...)
{
	pthread_mutex_t mutex;
	pthread_mutex_lock(&mutex);

	assert(dest != NULL);

	dest->command = command;
	dest->status = status;

	bzero(dest->id.name, MAX_NAME_LEN);
	snprintf(dest->id.name, MAX_NAME_LEN - 1, "%s", name);

	bzero(dest->payload.data, MAX_DATA_LEN);

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(dest->payload.data, MAX_DATA_LEN - 1, fmt, ap);
	va_end(ap);

	dest->payload.len = strlen(dest->payload.data);

	pthread_mutex_unlock(&mutex);
}


void set_transaction_data(transaction_t *dest, command_t command, status_t status,
		const char *name, const void *data, size_t size)
{
	assert(dest != NULL);

	dest->command = command;
	dest->status = status;

	bzero(dest->id.name, MAX_NAME_LEN);
	snprintf(dest->id.name, MAX_NAME_LEN - 1, "%s", name);

	bzero(dest->payload.data, MAX_DATA_LEN);

	if (size > MAX_DATA_LEN) {
		set_transaction(dest, command, FAILURE, name,
				"Internal data error: data too is too big to handle");
	}
	else {
		memcpy(dest->payload.data, data, size);
	}
}
