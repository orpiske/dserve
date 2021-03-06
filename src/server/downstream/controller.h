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

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include "options.h"
#include "transaction.h"
#include "util.h"
#include "node.h"
#include "list.h"

#include "entry.h"
#include "queue.h"
#include "downloadmanager.h"
#include "messenger.h"

bool send_to_controller(const transaction_t *transaction);
bool send_to_server(const transaction_t *transaction);

bool read_from_server(transaction_t *transaction);
bool read_from_controller(transaction_t *transaction);
int start_controller(void);


#endif
