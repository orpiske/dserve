#include "dsclient.h"


static bool default_send_receive(connection_t *connection, transaction_t *transaction)
{
	transaction_t reply;
	const options_t *options = get_options_object();
	connection_send(connection, transaction, sizeof(transaction_t));

	ssize_t num_read = connection_read(connection, options->timeout, &reply,
			sizeof(transaction_t));
	if (num_read <= 0) {
		return false;
	}

	if (reply.status == SUCCESS) {
		fprintf(stderr, "Request executed successfully\n");
	}
	else {
		if (reply.status == PROCESSING) {
			fprintf(stderr, "The server is busy processing your request: %s\n",
						reply.payload.data);
		}
		else {
			fprintf(stderr, "Error processing the request: %s\n",
					reply.payload.data);
			return false;
		}
	}

	return true;
}


static bool status_command(connection_t *connection, transaction_t *transaction) {
	const options_t *options = get_options_object();

	transaction_t reply;
	do {

		bzero(&reply, sizeof(reply));
		connection_send(connection, transaction, sizeof(transaction_t));
		do {
			ssize_t num_read = connection_read(connection, options->timeout,
					&reply, sizeof(transaction_t));
			if (num_read <= 0) {
				printf("Timed out while trying to read");
				continue;
			}

			if (reply.status == PROCESSING) {
				entry_t *entry = (entry_t *) reply.payload.data;

				switch (entry->status) {
					case STARTED: {
						printf("Downloading %.0f of %.0f of file %s at %.0f Kb/s\n",
								entry->complete, entry->size, entry->id.name,
								(entry->speed / 1024));
						break;
					}
					case WAITING: {
						printf("Downloaded file %s is waiting for available slots\n",
								 entry->id.name);
						break;
					}
					case COMPLETED:
					case PAUSED: {
						printf("Downloaded %.0f of %.0f of file %s\n",
							entry->complete, entry->size, entry->id.name);
						break;
					}
					default: {
						break;
					}

				}
			}
		} while (reply.status == PROCESSING);
		//printf("Finished processing ... \n")
		sleep(1);
	} while (options->keep_alive);
}

static bool run(const options_t *options, transaction_t *transaction) {
	bool ret = false;
	connection_t *connection = NULL;
	transaction_t reply;

	connection = connection_new(options->host, options->port);
	if (!connection) {
		return EXIT_FAILURE;
	}

	ret = connection_connect(connection);
	if (!ret) {
		return ret;
	}


	switch (transaction->command) {
		case STATUS: {
			ret = status_command(connection, transaction);
			break;
		}
		default: {
			ret = default_send_receive(connection, transaction);
			break;
		}

	}

	return ret;
}

static options_t *get_configuration_object() {
	const char *filename = options_get_filename();

	if (!filename) {
		return NULL;
	}

	return options_new(filename, CLIENT_OPTIONS);
}

int main(int argc, char **argv) {
	int c;
	int digit_opt = 0;
	int option_index = 0;
	transaction_t transaction = {0};
	options_t *options = get_configuration_object();

	if (!options) {
		return EXIT_FAILURE;
	}

	set_options_object(options);
	while (1) {

		static struct option long_options[] = {
				{ "port", true, 0, 'p' },
				{ "host", true, 0, 'H' },
				{ "name", true, 0, 'n' },
				{ "url", true, 0, 'u' },
				{ "status", false, 0, 's' },
				{ "cancel", false, 0, 'c'},
				{ "pause", false, 0, 'P' },
				{ "restart", false, 0, 'r' },
				{ "keepalive", false, 0, 'k' },
				{ "init", false, 0, 'i' },
				{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "p:H:n:u:scPrki", long_options, &option_index);
		if (c == -1) {
			if (optind == 1) {
				fprintf(stderr, "Not enough options\n");
				return EXIT_FAILURE;
			}
			break;
		}

		switch (c) {
			case 'p': {
				options->port = (short) strtol(optarg, NULL, 10);
				break;
			}
			case 'H': {
				strncpy(options->host, optarg, sizeof(options->host) - 1);
				break;
			}
			case 'n': {
				strncpy(transaction.id.name, optarg, sizeof(transaction.id.name));
				break;
			}
			case 'u': {
				strncpy(transaction.payload.data, optarg, sizeof(transaction.payload.data));
				transaction.payload.len = strlen(transaction.payload.data);
				transaction.command = DOWNLOAD;
				break;
			}
			case 's': {
				transaction.command = STATUS;
				break;
			}
			case 'c': {
				transaction.command = CANCEL;
				break;
			}
			case 'P': {
				transaction.command = PAUSE;
				break;
			}
			case 'r': {
				transaction.command = RESTART;
				break;
			}
			case 'k': {
				options->keep_alive = true;
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

	bool ret = run(options, &transaction);

	return (ret == true) ? EXIT_SUCCESS : EXIT_FAILURE;
}
