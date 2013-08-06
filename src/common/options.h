#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>

#include "defaults.h"


typedef struct options_t_ {
	const char const *filename;
	unsigned short port;
	char host[OPT_MAX_STR_SIZE];
	unsigned short int timeout;
	char logdir[OPT_MAX_STR_SIZE];
	char destdir[OPT_MAX_STR_SIZE];
	unsigned short max_concurrent_downloads;
	unsigned short queue_retry_count;
	unsigned short queue_retry_wait;
	bool keep_alive;
	bool quiet;
	bool debug;
} options_t;


typedef enum {
		CLIENT_OPTIONS,
		SERVER_OPTIONS,
} options_type_t;


void set_options_object(options_t *ojb);
const options_t *get_options_object(void);
options_t *options_save(options_t *opt);
options_t *options_new(const char *filename, options_type_t type);
const char *options_get_filename(void);



#endif
