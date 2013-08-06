#ifndef TRANSACTION_H
#define TRANSACTION_H

#define MAX_NAME_LEN 255
#define MAX_URL_LEN 512
#define MAX_DATA_LEN 1536

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

typedef enum command_t_ {
	NONE = 0,
	DOWNLOAD = 1,
	STATUS = 2,
	PAUSE = 4,
	CANCEL = 8,
	RESTART = 16,
} command_t;

typedef enum status_t_ {
	SUCCESS,
	PROCESSING,
	FAILURE,
} status_t;

typedef struct trx_id_t_ {
	char name[MAX_NAME_LEN];
} trx_id_t;

typedef struct payload_t_ {
	unsigned short int len;
	char data[MAX_DATA_LEN];
} payload_t;

typedef struct transaction_t_ {
	command_t command;
	status_t status;
	trx_id_t id;
	payload_t payload;
} transaction_t;

typedef struct message_t_ {
	long type;
	transaction_t transaction;
} message_t;


void set_transaction(transaction_t *dest, command_t command, status_t status,
		const char *name, const char *fmt, ...);
void set_transaction_data(transaction_t *dest, command_t command, status_t status,
		const char *name, const void *data, size_t size);

#endif
