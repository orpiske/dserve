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

#include "options.h"

static options_t *options = NULL;

extern bool exists(const char *filename);
extern char *trim(char *input, int size);


static void options_set_string(char *dest, int size, const char *fmt, ...) {
	va_list ap;
	pthread_mutex_t mutex;

	pthread_mutex_lock(&mutex);
	va_start(ap, fmt);

	vsnprintf(dest, size, fmt, ap);

	va_end(ap);
	pthread_mutex_unlock(&mutex);
}

static void options_set_logdir(options_t *dest, const char *dir) {
	if (!dir) {
		options_set_logdir(dest, OPT_DEFAULT_ALTERNATE_DIR);
	}
	else {
		options_set_string(dest->logdir, OPT_MAX_STR_SIZE, "%s%s%s%s%s%s",
						dir, FILE_SEPARATOR,
						DS_DEFAULT_DIR, FILE_SEPARATOR,
						OPT_LOG_DIR, FILE_SEPARATOR);
	}
}

static void options_set_destdir(options_t *dest, const char *dir) {
	if (!dir) {
		options_set_destdir(dest, OPT_DEFAULT_ALTERNATE_DIR);
	}
	else {
		options_set_string(dest->destdir, OPT_MAX_STR_SIZE, "%s%s%s",
				dir, FILE_SEPARATOR, OPT_DOWNLOAD_DIR);
	}
}

static void options_read(void *dest, FILE *source, const char *mask, const char *name) {
	pthread_mutex_t mutex;
	pthread_mutex_lock(&mutex);

	rewind(source);
	while (!feof(source)) {
		char line[OPT_MAX_STR_SIZE];

		bzero(line, OPT_MAX_STR_SIZE);
		fgets(line, OPT_MAX_STR_SIZE - 1, source);

		trim(line, sizeof(line));

		if (strstr(line, name) != NULL) {
			char none[OPT_MAX_STR_SIZE];

			bzero(none, sizeof(none));
			sscanf(line, mask, none, dest);

			goto end;
		}
	}

	end:
	pthread_mutex_unlock(&mutex);
}

static void options_read_char(char *dest, FILE *source, const char *name) {
	options_read(dest, source, "%[^=]=" OPT_MAX_STR_SIZE_MASK, name);

}


static void options_read_int(int *dest, FILE *source, const char *name) {
	options_read(dest, source, "%[^=]=%d", name);
}


static void options_read_short(short *dest, FILE *source, const char *name) {
	options_read(dest, source, "%[^=]=%hi", name);
}


static void options_read_for_server(options_t *dest, FILE *source) {
	options_read_short(&dest->port, source, "server.listen.port");
	options_read_char(dest->host, source, "server.listen.ip");
	options_read_short(&dest->timeout, source, "server.connection.timeout");
	options_read_char(dest->logdir, source, "server.directory.log");
	options_read_char(dest->destdir, source, "server.directory.dest");
	options_read_short(&dest->max_concurrent_downloads, source, "server.downloads.max");
	options_read_short((short *) &dest->quiet, source, "server.mode.quiet");
	options_read_short((short *) &dest->debug, source, "server.mode.debug");
}

static void options_write_for_server(const options_t *source, FILE *dest) {
	fprintf(dest, "server.listen.port=%hi\n", source->port);
	fprintf(dest, "server.listen.ip=%s\n", source->host);
	fprintf(dest, "server.connection.timeout=%hi\n", source->timeout);
	fprintf(dest, "server.directory.log=%s\n", source->logdir);
	fprintf(dest, "server.directory.dest=%s\n", source->destdir);
	fprintf(dest, "server.downloads.max=%hi\n", source->max_concurrent_downloads);
	fprintf(dest, "server.mode.quiet=%hi\n", source->quiet);
	fprintf(dest, "server.mode.debug=%hi\n", source->debug);
}

static void options_read_for_client(options_t *dest, FILE *source) {
	options_read_short(&dest->port, source, "client.server.port");
	options_read_char(dest->host, source, "client.server.ip");
	options_read_short(&dest->timeout, source, "client.connection.timeout");
	options_read_short((short *) &dest->keep_alive, source, "client.connection.keepalive");
}


static void options_write_for_client(const options_t *source, FILE *dest) {

	fprintf(dest, "client.server.port=%hi\n", source->port);
	fprintf(dest, "client.server.ip=%s\n", source->host);
	fprintf(dest, "client.connection.timeout=%hi\n", source->timeout);
	fprintf(dest, "client.connection.keepalive=%hi\n", source->keep_alive);
}

options_t *options_save(options_t *opt) {
	FILE *file = NULL;

	file = fopen(opt->filename, "w+");
	if (!file) {
		return NULL;
	}

#ifdef DSSERVER
	options_write_for_server(opt, file);
#else
	options_write_for_client(opt, file);
#endif

	return opt;
}

static void options_new_for_server(const char *filename, options_t *ret) {
	FILE *file = NULL;

	if (!exists(filename)) {
		const char *home = getenv("HOME");

		ret->quiet = false;
		ret->debug = true;
		ret->max_concurrent_downloads = OPT_DEFAULT_MAX_CONCURRENT_DOWNLOADS;
		ret->port = OPT_DEFAULT_PORT;
		snprintf(ret->host, OPT_MAX_STR_SIZE - 1, "%s", OPT_DEFAULT_HOST);

		options_set_logdir(ret, home);
		options_set_destdir(ret, home);

		file = fopen(filename, "w+");
		options_write_for_server(ret, file);
	}
	else {
		file = fopen(filename, "r");
		options_read_for_server(ret, file);

	}

	ret->timeout = OPT_DEFAULT_TIMEOUT;
	ret->queue_retry_count = OPT_DEFAULT_Q_RETRY_COUNT;
	ret->queue_retry_wait = OPT_DEFAULT_Q_RETRY_WAIT;
	ret->filename = filename;

	fflush(NULL);
	fclose(file);
}


static void options_new_for_client(const char *filename, options_t *ret) {
	FILE *file = NULL;

	if (!exists(filename)) {
		const char *home = getenv("HOME");

		ret->timeout = OPT_DEFAULT_TIMEOUT;
		ret->keep_alive = false;
		ret->port = OPT_DEFAULT_PORT;

		snprintf(ret->host, OPT_MAX_STR_SIZE - 1, "%s", OPT_DEFAULT_HOST);

		file = fopen(filename, "w+");
		options_write_for_client(ret, file);
	}
	else {
		file = fopen(filename, "r");
		options_read_for_client(ret, file);

	}

	ret->filename = filename;

	fflush(NULL);
	fclose(file);
}

options_t *options_new(const char *filename, options_type_t type) {
	options_t *ret = (options_t *) malloc(sizeof(options_t));

	if (!ret) {
		fprintf(stderr, "Not enough memory to allocate for options object\n");

		return NULL;
	}

	if (type == SERVER_OPTIONS) {
		options_new_for_server(filename, ret);
	}

	options_new_for_client(filename, ret);

	return ret;
}


const char *options_get_filename(void) {
	char *home = getenv("HOME");
	int len = 0;
	char *filename = NULL;

	if (!home) {
		fprintf(stderr, "Unable to discover home directory");
		return NULL;
	}

	len = strlen(home) + strlen(DS_DEFAULT_DIR) + strlen(DS_CONFIG_FILE) + 3;
	filename = (char *) malloc(len);
	if (!filename) {
		fprintf(stderr, "Not enough memory to allocate %i bytes", len);
		return NULL;
	}

	memset(filename, 0, len);
	snprintf(filename, len - 1, "%s%s%s", home, FILE_SEPARATOR, DS_DEFAULT_DIR);

	if (!recursive_mkdir(filename)) {
		free(filename);

		return NULL;
	}

	strcat(filename, FILE_SEPARATOR);
	strcat(filename, DS_CONFIG_FILE);

	return filename;
}

void set_options_object(options_t *obj) {
	if (options == NULL) {
		options = obj;
	}
}


const options_t *get_options_object(void) {
	return options;
}


