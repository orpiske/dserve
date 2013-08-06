#include "queue.h"

static list_t *download_queue = NULL;

static void queue_init(const transaction_t *request, transaction_t *response) {
	if (!download_queue) {

		download_queue = list_new();
		if (!download_queue) {
			if (request && response) {

				set_transaction(response, request->command, FAILURE, request->id.name,
						"Not enough memory to create a download queue");
			}
			else {
				messenger msg = get_messenger();

				msg(ERROR, "Not enough memory to create a download queue");
			}

			return;
		}
	}
}

static bool queue_count_by_status(const void *data, const void *ptr, void *result) {
	entry_t *entry = (entry_t *) data;


	if (!entry) {
		messenger msg = get_messenger();
		msg(ERROR, "Invalid data structure in the download queue");

		return false;
	}

	entry_status_t *tmp = (entry_status_t *) ptr;
	if (entry->status == *tmp) {
		int *count = (int *) result;

		(*count)++;
	}

	return false;
}

static int queue_count_started(void) {
	int count = 0;
	entry_status_t status = STARTED;

	list_for_each_compare(download_queue, false, queue_count_by_status, &status, &count);

	return count;
}


static bool queue_compare_id(const void *data, const void *ptr, void *result){
	const entry_t *existing = (const entry_t *) data;
	const trx_id_t *id = (const trx_id_t *) ptr;

	if (!existing) {
		messenger msg = get_messenger();
		msg(ERROR, "Invalid data structure in the download queue");

		return false;
	}

	if (strcmp(existing->id.name, id->name) == 0) {
		if (result != NULL) {
			bool *tmp_result = (bool *) result;

			*tmp_result = true;
		}
		return true;
	}

	return false;
}

static bool queue_entry_exists(const trx_id_t *id) {
	bool result;

	list_for_each_compare(download_queue, true, queue_compare_id, id, &result);

	return result;
}


static void queue_update(const void *data, void *ptr) {
	const options_t *options = get_options_object();
	messenger msg = get_messenger();

	entry_t *entry = (entry_t *) data;

	if (!entry) {
		msg(ERROR, "Invalid data structure in the download queue");

		return;
	}

	switch (entry->status) {
		case WAITING: {
			int count = queue_count_started();

			if (count < options->max_concurrent_downloads) {
				msg(INFO, "Starting to download %s", entry->id.name);
				download_start(entry);
				entry->status = STARTING;
			}
			else {
				msg(INFO, "Reached maximum number of concurrent downloads: %i / %i",
						count, options->max_concurrent_downloads);
				msg(INFO, "Download %s will wait until there are available slots",
						entry->id.name);
			}


			break;
		}
		case STARTED: {
			msg(DEBUG, "This one will do nothing");

			break;
		}
		default: {
			msg(DEBUG, "Default does nothing");

			break;
		}

	}
}

static void queue_add_container(const transaction_t *request, transaction_t *response) {
	const options_t *options = get_options_object();
	messenger msg = get_messenger();
	bool entry_exists = false;

	queue_init(request, response);
	if (!download_queue) {
		return;
	}

	entry_exists = queue_entry_exists(&request->id);
	if (entry_exists) {
		set_transaction(response, request->command, FAILURE, request->id.name,
				"File %s already exists in the queue", request->id.name);

		return;
	}

	entry_t *entry = entry_new();
	if (!entry) {
		set_transaction(response, request->command, FAILURE, request->id.name,
				"Not enough memory to create a new download entry");

		return;
	}
	entry_set_target(entry, request);

	const node_t *ret = list_append(download_queue, entry);
	if (ret == NULL) {
		entry = delete_entry(&entry);
		set_transaction(response, request->command, FAILURE, request->id.name,
				"Not enough memory to insert the download into the queue");

		return;
	}

	msg(INFO, "Successfully added the file to the download queue");
	set_transaction(response, request->command, SUCCESS, request->id.name,
					"Successfully added the file to the download queue");

	list_for_each(download_queue, queue_update, NULL);
}


void queue_add(const transaction_t *request, transaction_t *response)
{
	pthread_mutex_t mutex;
	pthread_mutex_lock(&mutex);

	queue_add_container(request, response);

	pthread_mutex_lock(&mutex);
}


static void queue_get_info(const void *data, void *ptr) {
	entry_t *entry = (entry_t *) data;
	transaction_dispatcher_t *dispatcher_obj = (transaction_dispatcher_t *) ptr;

	set_transaction_data(dispatcher_obj->transaction, STATUS, PROCESSING, entry->id.name,
								entry, sizeof(entry_t));


	messenger msg = get_messenger();
	msg(DEBUG, "Downloaded %.0f of %.0f for %s (%u)", entry->complete, entry->size,
			entry->id.name, sizeof(entry_t));
	dispatcher_obj->dispatcher_func(dispatcher_obj->transaction);
}

void queue_status(const transaction_t *request, transaction_t *response,
		bool(*sender)(const transaction_t *))
{
	transaction_dispatcher_t dispatcher_obj;

	dispatcher_obj.dispatcher_func = sender;
	dispatcher_obj.transaction = response;

	queue_init(request, response);
	if (!download_queue) {
		set_transaction(response, request->command, FAILURE, request->id.name,
				"The queue is empty");
		return;
	}

	list_for_each(download_queue, queue_get_info, &dispatcher_obj);

	set_transaction(response, request->command, SUCCESS, request->id.name,"");
}

static void queue_data_save(const void *data, void *ptr) {
	entry_t *entry = (entry_t *) data;
	FILE *file = (FILE *) ptr;

	if (entry->status == STARTED || entry->status == STARTING) {
		download_pause(&entry->id);
		entry->status = WAITING;
	}
	size_t nelem = fwrite(entry, sizeof(entry_t), 1, file);

	if (nelem != 1) {
		messenger msg = get_messenger();

		msg(ERROR, "Unable to save data for %s: %s", entry->id.name, strerror(errno));

		return;
	}
}


void queue_save(const transaction_t *request, transaction_t *response) {
	FILE *idx_file = NULL;
	const options_t *options = get_options_object();
	char filename[512];

	if (!download_queue) {
		return;
	}

	bzero(filename, sizeof(filename));
	snprintf(filename, sizeof(filename), "%s%s%s", options->destdir,
			FILE_SEPARATOR, ".download.idx");
	idx_file = fopen(filename, "w");

	if (!idx_file) {
		messenger msg = get_messenger();
		msg(ERROR, "Unable to create the index file %s: %s", filename, strerror(errno));

		return;
	}

	list_for_each(download_queue, queue_data_save, idx_file);
	fclose(idx_file);
}


bool queue_load(void) {
	FILE *idx_file = NULL;
	const options_t *options = get_options_object();
	char filename[512];
	messenger msg = get_messenger();

	queue_init(NULL, NULL);
	if (!download_queue) {
		return false;
	}


	bzero(filename, sizeof(filename));
	snprintf(filename, sizeof(filename), "%s%s%s", options->destdir,
			FILE_SEPARATOR, ".download.idx");

	if (!exists(filename)) {
		return false;
	}

	idx_file = fopen(filename, "r");
	if (!idx_file) {
		msg(ERROR, "Unable to open file: %s", strerror(errno));

		return false;
	}


	while (!feof(idx_file)) {
		entry_t *entry = entry_new();

		if (!entry) {
			msg(ERROR, "Not enough memory to create a new download entry");

			return false;
		}

		size_t nelem = fread(entry, sizeof(entry_t), 1, idx_file);

		if (nelem != 1) {
			entry = delete_entry(&entry);

			break;
		}

		msg(INFO, "Successfully loaded the download file %s", entry->id.name);
		const node_t *ret = list_append(download_queue, entry);
		if (!ret) {
			entry = delete_entry(&entry);
			msg(ERROR, "Not enough memory to insert the download into the queue");

			return false;
		}

		msg(INFO, "Successfully loaded the file %s to the download queue",
				entry->id.name);
	}

	list_for_each(download_queue, queue_update, NULL);

	return true;
}


void queue_pause(const transaction_t *request, transaction_t *response) {
	entry_t *entry = NULL;
	bool ret = true;

	messenger msg = get_messenger();

	msg(DEBUG, "Pausing download ...");
	entry = (entry_t *) list_get_item(download_queue, queue_compare_id, &request->id);

	if (!entry) {
		set_transaction(response, request->command, FAILURE, request->id.name,
			"File %s does not exist in the queue", request->id.name);

		return;
	}

	switch (entry->status) {
		case STARTED: {
			ret = download_pause(&request->id);
			break;
		}
		case CANCELLED: {
			break;
		}
		case COMPLETED: {
			break;
		}
		default: {
			entry->status = PAUSED;
			break;
		}
	}

	if (ret == true) {
		set_transaction(response, request->command, SUCCESS, request->id.name,
								"Successfully paused download in the the queue");
	}
	else {
		set_transaction(response, request->command, SUCCESS, request->id.name,
			"Failed to pause the download in the the queue. Check the server logs");
	}
}


void queue_cancel(const transaction_t *request, transaction_t *response) {
	entry_t *entry = NULL;
	bool ret = true;

	messenger msg = get_messenger();

	msg(DEBUG, "Cancelling download ...");
	entry = (entry_t *) list_get_item(download_queue, queue_compare_id, &request->id);

	if (!entry) {
		set_transaction(response, request->command, FAILURE, request->id.name,
			"File %s does not exist in the queue", request->id.name);

		return;
	}

	switch (entry->status) {
		case STARTED: {
			download_cancel(&request->id);
			break;
		}
		case CANCELLED: {
			break;
		}
		default: {
			entry->status = PAUSED;
			break;
		}
	}

	ret = list_remove_item(download_queue, queue_compare_id, &request->id);

	if (ret == true) {
		set_transaction(response, request->command, SUCCESS, request->id.name,
								"Successfully removed download from the the queue");
	}
	else {
		set_transaction(response, request->command, SUCCESS, request->id.name,
			"Failed to remove the download from the the queue. Check the server logs");
	}
}

void queue_restart(const transaction_t *request, transaction_t *response) {
	entry_t *entry = NULL;
	bool ret = true;

	messenger msg = get_messenger();

	msg(DEBUG, "Cancelling download ...");
	entry = (entry_t *) list_get_item(download_queue, queue_compare_id, &request->id);

	if (!entry) {
		set_transaction(response, request->command, FAILURE, request->id.name,
			"File %s does not exist in the queue", request->id.name);

		return;
	}

	switch (entry->status) {
		case STARTING:
		case STARTED: {
			set_transaction(response, request->command, FAILURE, request->id.name,
				"File %s is already being downloaded", request->id.name);
			break;
		}
		case CANCELLED: {
			set_transaction(response, request->command, FAILURE, request->id.name,
				"File %s is already being cancelled", request->id.name);
			break;
		}
		case COMPLETED: {
			set_transaction(response, request->command, FAILURE, request->id.name,
				"File %s is already finished", request->id.name);
			break;
		}
		default: {
			entry->status = WAITING;
			break;
		}
	}

	msg(INFO, "Your download will start as soon as there are slots available");
	set_transaction(response, request->command, SUCCESS, request->id.name,
		"Success: your download will start as soon as there are slots available");

	list_for_each(download_queue, queue_update, NULL);
}
