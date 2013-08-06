#include "controller.h"

static int input_queue = 0;
static int output_queue = 0;

static messenger msg;


static int create_input_queue() {
	key_t key = ftok("/tmp", 0x29a);
	return create_queue("input", key);
}

static int create_output_queue() {
	key_t key = ftok("/etc", 0x30a);
	return create_queue("output", key);
}

bool send_to_controller(const transaction_t *transaction) {
	return send_message_local(input_queue, transaction);
}

bool send_to_server(const transaction_t *transaction) {
	return send_message_local(output_queue, transaction);
}


bool read_from_controller(transaction_t *transaction) {
	return read_message_local(output_queue, transaction, 1, 0);
}

bool read_from_server(transaction_t *transaction) {
	return read_message_local(input_queue, transaction, 1, 0);
}


static bool process_request(const transaction_t *request, transaction_t *response)
{

	switch (request->command) {
		case DOWNLOAD: {
			queue_add(request, response);
			break;
		}
		case STATUS: {
			queue_status(request, response, send_to_server);
			break;
		}
		case PAUSE: {
			queue_pause(request, response);
			break;
		}
		case CANCEL: {
			queue_cancel(request, response);
			break;
		}
		case RESTART: {
			queue_restart(request, response);
			break;
		}
		default: {
			msg(DEBUG, "Unknown command ...");
			break;
		}
	}

	return true;
}

static void termination_handler(int num) {
	msg(INFO, "Signal %i caught. Terminating ...", num);
	queue_save(NULL, NULL);

	exit(0);
}


static void install_signal_handler() {
	struct sigaction sig = {0};

	sig.sa_handler = termination_handler;
	sig.sa_flags = SA_RESTART;

	sigaction(SIGHUP, &sig, NULL);
	sigaction(SIGINT, &sig, NULL);
	sigaction(SIGQUIT, &sig, NULL);
	sigaction(SIGTERM, &sig, NULL);
}


int start_controller(void) {
	int child = 0;
	const options_t *options = get_options_object();

	msg = get_messenger();
	input_queue = create_input_queue();
	output_queue = create_output_queue();

	child = fork();
	if (child > 0) {
		return EXIT_SUCCESS;
	}
	if (child < 0) {
		msg(ERROR, "Unable to create child process: %s", strerror(errno));

		return EXIT_FAILURE;
	}

	if (queue_load() != true) {
		msg(WARNING, "Unable to load downloads from the queue. Trying to ignore the error");
	}

	install_signal_handler();
	if (options->quiet == true) {
		if (remap_log(options->logdir, "controller", stderr) != true) {
			exit(-1);
		}
	}

	msg(INFO, "Waiting for messages ... ");
	while (true) {
		bool ret = false;
		transaction_t transaction;
		transaction_t response;

		ret = read_from_server(&transaction);

		if (ret == true) {
			process_request(&transaction, &response);
			send_to_server(&response);
		}

		fflush(NULL);
	}

	return EXIT_SUCCESS;
}

