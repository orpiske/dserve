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
