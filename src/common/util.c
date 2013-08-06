#include "util.h"


/**
 * Removes spaces from the right side of a string
 * @param input Input string
 * @param size Size of the string
 * @return A pointer to the new string (which is the same address as input)
 */
char *rtrim(char *input, int size) {
	int i = size;

	assert(input != NULL);

	while (isspace(input[i])) {
		input[i] = 0;
		i--;
	}

	return input;
}


/**
 * Removes spaces from the left side of a string
 * @param input Input string
 * @param size Size of the string
 * @return A pointer to the new string (which is the same address as input)
 */
char *ltrim(char *input, int size) {
	int i = 0;
	assert(input != NULL);

	while (isspace(input[i])) {
		i++;
	}

	if (i > 0) {
		memcpy(input, input + i, size - i);
		memset(input + (size - i), 0, i);
	}

	return input;
}


/**
 * Removes spaces from all sides of a string
 * @param input Input string
 * @param size Size of the string
 * @return A pointer to the new string (which is the same address as input)
 */
char *trim(char *input, int size) {
	int tmp_size = 0;
	char *tmp_string = NULL;

	tmp_string = rtrim(input, size);
	tmp_size = strlen(tmp_string);

	return ltrim(tmp_string, tmp_size);
}

/**
 * Checks whether a given file exists
 * @param filename the filename
 * @return
 */
bool exists(const char *filename) {
	int ret = 0;
	struct stat info;

	ret = stat(filename, &info);
	if (ret == 0) {
		return true;
	}

	return false;
}


/**
 * Checks whether can read/write a given file
 * @param filename the filename
 * @return
 */
static bool can_read_write(const char *filename) {
	int ret = 0;


	ret = access(filename, R_OK | W_OK);
	if (ret < 0) {
		messenger msg = get_messenger();
		switch (errno) {
			case ENOENT: {
				msg(ERROR, "No such file or directory %s: %s",
							filename, strerror(errno));
				return false;
			}
			case EACCES: {
				msg(ERROR, "Access denied (no read/write permission) %s: %s",
								filename, strerror(errno));
				return false;
			}
			default: {
				msg(ERROR, "Unhandled IO error trying to access %s: %s",
												filename, strerror(errno));
				return false;
			}
		}
	}

	return true;
}


static bool rename_if_exists(const char *filename) {
	messenger msg = get_messenger();

	if (!exists(filename)) {
		return true;
	}

	if (!can_read_write(filename)) {
		return false;
	}

	int size = strlen(filename) + 16;
	char *new_file = (char *) malloc(size);

	if (!new_file) {
		msg(ERROR, "Not enough memory to allocate for renaming the existing log file");
		return false;
	}

	int i = 0;
	do {
		bzero(new_file, size);
		snprintf(new_file, size, "%s.%03i", filename, i);

		if (!exists(new_file)) {
			int ret = 0;

			ret = rename(filename, new_file);
			if (ret != 0) {
				msg(ERROR, "Unable to rename log file: %s", strerror(errno));

				free(new_file);
				return false;
			}

			break;
		}
		i++;
	} while (true);

	free(new_file);
	return true;
}


static bool remap_io(const char *dir, const char *name, FILE *fd) {
	char *fullpath;
	size_t size = strlen(dir) + APPEND_SIZE_REMAP;
	messenger msg = get_messenger();

	fullpath = (char *) malloc(size);
	if (fullpath == NULL) {
		msg(ERROR, "Unable to remap IO: not enough free memory");

		return false;
	}

	bzero(fullpath, size);
	snprintf(fullpath, size - 1 , "%s/%s", dir, name);

	if (!rename_if_exists(fullpath)) {
		return false;
	}

	FILE *f = freopen(fullpath, "a", fd);
	if (f == NULL) {
			msg(ERROR, "Unable to remap error IO: %s (%s)", strerror(errno),
				fullpath);

		return false;
	}

	msg(INFO, "Log file successfully opened");

	free(fullpath);
	return true;
}


/**
 * Remaps standard output/error output to a given file in a given dir
 * @param dir
 * @param base_name
 * @param fd
 * @return
 */
bool remap_log(const char *dir, const char *base_name, FILE *fd) {
	char name[32];

	bzero(name, sizeof(name));
	snprintf(name, sizeof(name) - 1, "%s.log", base_name);

	return remap_io(dir, name, fd);
}


/**
 * Does the obvious: creates a POSIX queue
 * @param type
 * @param key
 * @return
 */
int create_queue(const char *type, key_t key) {
	int msg_flag = IPC_CREAT | 0666;
	messenger msg = get_messenger();

	int queue_id = msgget(key, msg_flag);
	if (queue_id < 0) {
		msg(ERROR, "Unable to create a %s queue: %s", type, strerror(errno));

		return EXIT_FAILURE;
	}

	msg(INFO, "An %s queue was successfully created with id: %i", type, queue_id);
	return queue_id;
}


/**
 * Sends a control message to the local queue used by the controller/server
 * @param queue
 * @param transaction
 * @return
 */
bool send_message_local(int queue, const transaction_t *transaction) {
	int ret = 0;
	short retry_count = 0;
	message_t message = {0};
	const options_t *options = get_options_object();

	message.type = 1;
	message.transaction = *transaction;

	do {
		ret = msgsnd(queue, &message, sizeof(message.transaction), IPC_NOWAIT);
		if (ret < 0) {
			/**
			 * If the resource is temporarly unavailable we wait some time
			 * before trying again. We default to 100 miliseconds as it seems to
			 * be enough to make the data flow through the queues.
			 */
			retry_count++;
			if (errno != EAGAIN || retry_count >= options->queue_retry_count) {
				messenger msg = get_messenger();

				msg(ERROR, "Unable to send control message to %i after %i retries: %s",
						queue, retry_count, strerror(errno));

				return false;
			}

			usleep(options->queue_retry_wait);
		}
		else {
			break;
		}

	} while (retry_count < options->queue_retry_count);

	return true;
}


/**
 * Reads a control message from the local queue used by the controller/server
 * @param queue
 * @param transaction
 * @param type
 * @param opt
 * @return
 */
bool read_message_local(int queue, transaction_t *transaction, int type, int opt) {
	int ret = 0;
	message_t message;
	short retry_count =0;
	const options_t *options = get_options_object();

	memset(transaction, 0, sizeof(transaction_t));
	do {
		ret = msgrcv(queue, &message, sizeof(transaction_t), type, opt);
		if (ret < 0) {
			retry_count++;

			if (errno != EAGAIN || retry_count >= options->queue_retry_count) {
				messenger msg = get_messenger();

				msg(ERROR, "Unable to receive control message from %i after %i retries: %s",
						queue, retry_count, strerror(errno));

				return false;
			}

			usleep(options->queue_retry_wait);
		}
		else {
			break;
		}
	} while (retry_count < options->queue_retry_count);

	memcpy(transaction, &message.transaction, sizeof(transaction_t));
	return true;
}


static bool create_dir(const char *path) {
	int ret = 0;

	ret = mkdir(path, 0774);
	if (ret != 0) {
		messenger msg = get_messenger();

		if (msg != NULL) {
			msg(ERROR, "Unable to create directory %s: %s", path,
					strerror(errno));

			return false;
		}
	}

	return true;
}


/**
 * Given a path, this function will recursively create a directory
 * @param path The path
 * @return true if successfully create the directories or false otherwise
 */
bool recursive_mkdir(const char *path) {
	const char *ptr = path;
	int count = 0;
	int tmp_size = strlen(path) + 1;
	char *tmp = (char *) malloc(tmp_size);

	if (!tmp) {
		messenger msg = get_messenger();

		if (msg != NULL) {
			msg(ERROR, "Not enough memory to allocate %i bytes", tmp_size);
		}
		else {
			fprintf(stderr, "[ERROR] Not enough memory to allocate %i bytes",
					tmp_size);
		}

		return false;
	}

	bzero(tmp, tmp_size);
	do {
		const char *last = ptr;

		ptr++;
		ptr = strstr(ptr, FILE_SEPARATOR);
		count += ptr - last;

		snprintf(tmp, tmp_size, "%.*s", count, path);

		if (exists(tmp)) {
			continue;
		}

		if (!create_dir(tmp)) {
			free(tmp);

			return false;
		}
	} while (ptr != NULL);

	free(tmp);
	return true;
}
