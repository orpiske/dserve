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
