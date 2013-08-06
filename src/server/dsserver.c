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



int main(int argc, char **argv) {
	int c;
	int digit_opt = 0;
	int option_index = 0;
	 // options_t options;

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
			default: {
				printf("Error\n");
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
