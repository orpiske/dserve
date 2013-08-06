#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include <pthread.h>
#include <curl/curl.h>

#include "util.h"
#include "list.h"
#include "options.h"
#include "transaction.h"
#include "messenger.h"
#include "entry.h"

typedef struct download_thread_t_ {
	pthread_t thread;
	entry_t *entry;
} download_thread_t;


void download_start(entry_t *entry);
bool download_pause(const trx_id_t *id);
bool download_cancel(const trx_id_t *id);

#endif
