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

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>

#include "messenger.h"
#include "transaction.h"
#include "options.h"
#include "defaults.h"

#define APPEND_SIZE_REMAP 64


#define likely(x)  __builtin_expect((x),1)
#define unlikely(x)  __builtin_expect((x),0)

char *rtrim(char *input, int size);
char *ltrim(char *input, int size);
char *trim(char *input, int size);

bool exists(const char *filename);
bool remap_log(const char *dir, const char *base_name, FILE *fd);
int create_queue(const char *type, key_t key);
bool send_message_local(int queue, const transaction_t *transaction);
bool read_message_local(int queue, transaction_t *transaction, int type, int opt);
bool recursive_mkdir(const char *path);

#endif
