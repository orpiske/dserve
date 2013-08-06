#include "server.h"

static void process_request(connection_t *connection) {
	const options_t *options = get_options_object();
	ssize_t num_read = 0;
	transaction_t transaction = { 0 };
	transaction_t reply = { 0 } ;
	messenger msg = get_messenger();

	unsigned short timeout = options->timeout;

	if (timeout == 0) {
		timeout = OPT_DEFAULT_TIMEOUT;
	}

	do {
		msg(DEBUG, "Process %d started to process client requests", getpid());
		num_read = connection_read(connection, timeout, &transaction,
				sizeof(transaction_t));

		if (num_read <= 0) {
			if (num_read == CONNECTION_DISCONNECTED) {
				msg(DEBUG, "Peer disconnected");
			}
			else {
				if (num_read == CONNECTION_ERROR) {
					msg(ERROR, "Connection error");
				}
				else {
					msg(ERROR, "Read error");
				}
			}
			break;
		}

		msg(DEBUG, "Dispatching command %i to controller", transaction.command);
		send_to_controller(&transaction);

		do {
			read_from_controller(&reply);
			msg(DEBUG, "Received reply from controller: %s", reply.payload.data);
			connection_send(connection, &reply, sizeof(transaction_t));
		} while (reply.status == PROCESSING);

		fflush(NULL);

	} while(true);
}


int run_server(void) {
	bool ret = 0;
	connection_t *connection = NULL;

	const options_t *options = get_options_object();
	messenger msg = get_messenger();

	connection = connection_new(NULL, options->port);
	if (!connection) {
		return EXIT_FAILURE;
	}

	ret = connection_open(connection);
	if (!ret) {
		return EXIT_FAILURE;
	}

	while (true) {
		connection_t *client_connection = NULL;

		client_connection = connection_accept(connection);
		if (!client_connection) {
			continue;
		}

		ret = fork();
		if (ret == 0) {
			process_request(client_connection);
			client_connection = connection_close(connection);
			break;
		}
		else {
			if (ret < 0) {
				msg(ERROR, "Unable to fork process: %s", strerror(errno));
			}
			fflush(NULL);
		}
	}

	return EXIT_SUCCESS;
}
