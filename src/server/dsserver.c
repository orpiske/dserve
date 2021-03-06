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

#include "dsserver.h"

static void messenger_function(message_level_t level, const char *msg, ...) {
	va_list ap;
	pthread_mutex_t mutex;
	char *ret = NULL;

	pthread_mutex_lock(&mutex);
	va_start(ap, msg);
	vasprintf(&ret, msg, ap);

	switch(level) {
		case TRACE: {
			fprintf(stderr, "[TRACE]: %s\n", ret);
			break;
		}
		case DEBUG: {
			const options_t *options = get_options_object();

			if (options && options->debug) {
				fprintf(stderr, "[DEBUG]: %s\n", ret);
			}
			break;
		}
		case INFO: {
			fprintf(stderr, "[INFO]: %s\n", ret);
			break;
		}
		case WARNING: {
			fprintf(stderr, "[WARNING]: %s\n", ret);
			break;
		}
		case ERROR: {
			fprintf(stderr, "[ERROR]: %s\n", ret);
			break;
		}
		case FATAL: {
			fprintf(stderr, "[FATAL]: %s\n", ret);
			break;
		}
		default: {
			fprintf(stderr, "[MSG]: %s\n", ret);
			 break;
		}
	}

	va_end(ap);
	free(ret);
	pthread_mutex_unlock(&mutex);
}

static int run(void) {
	int ret = 0;

	ret = start_controller();
	if (ret == EXIT_FAILURE) {
		return ret;
	}

	return run_server();
}



static options_t *get_configuration_object() {
	const char *filename = options_get_filename();

	if (!filename) {
		return NULL;
	}

	return options_new(filename, SERVER_OPTIONS);
}

static bool create_directories(const options_t *opt) {
	if (!recursive_mkdir(opt->logdir)) {
		return false;
	}

	if (!recursive_mkdir(opt->destdir)) {
		return false;
	}

	return true;
}

static void show_help() {
	printf("Usage: ");
	printf("\t-p\t--port=<port> port used to listen to\n");
	printf("\t-h\t--host=<ip/hostname> hostname/ip listen to to\n");
	printf("\t-q\t--quiet runs quietly\n");
	printf("\t-d\t--debug runs in debug mode\n");
	printf("\t-i\t--init save options\n");
	printf("\t-h\t--help show this help\n");
}

int main(int argc, char **argv) {
	int c;
	int digit_opt = 0;
	int option_index = 0;

	options_t *options = get_configuration_object();
	if (!options) {
		messenger_function(ERROR, "Unable to obtain configuration");
		return -1;
	}

	if (!create_directories(options)) {
		messenger_function(ERROR, "Unable to create directories");

		return EXIT_FAILURE;
	}


	set_options_object(options);
	set_messenger(messenger_function);

	options->quiet = false;
	options->debug = true;
	while (1) {

		static struct option long_options[] = {
				{ "port", false, 0, 'p' },
				{ "host", false, 0, 'H' },
				{ "quiet", false, 0, 'q' },
				{ "debug", false, 0, 'd' },
				{ "init", false, 0, 'i' },
				{ "help", false, 0, 'h' },
				{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "p:H:qdi", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
			case 'p': {
				options->port = (int) strtol(optarg, NULL, 10);
				break;
			}
			case 'H': {
				bzero(options->host, sizeof(options->host));
				strncpy(options->host, optarg, sizeof(options->host) - 1);
				break;
			}
			case 'q': {
				options->quiet = true;
				break;
			}
			case 'd': {
				options->debug = true;
				break;
			}
			case 'i': {
				options_save(options);
				return EXIT_SUCCESS;
			}
			case 'h': {
				show_help();
				return EXIT_SUCCESS;
			}
			default: {
				printf("Invalid or missing option\n");
				show_help();
				break;
			}
		}
	}


	if (options->quiet == true) {
		remap_log(options->logdir, DS_USER_AGENT, stderr);
	}

	run();

	return EXIT_SUCCESS;
}
