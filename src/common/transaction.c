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
