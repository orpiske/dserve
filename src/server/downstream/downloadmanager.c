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

#include "downloadmanager.h"

static list_t *thread_list;

static int progress_callback(void *ptr, double dltotal, double dlnow,
		double ultotal, double ulnow)
{
	entry_t *entry = (entry_t *) ptr;
	time_t now;

	if (entry->size == 0) {
		entry->size = dltotal;
		time(&entry->time.start);
	}

	if (entry->time.measured == 0) {
		time(&entry->time.measured);
	}
	else {
		double diff;

		time(&now);
		diff = difftime(now, entry->time.measured);
		if (diff >= 1) {
			entry->speed = dlnow - entry->complete;

			entry->time.measured = now;

			entry->remaining = dltotal;
			entry->complete = dlnow;

			messenger msg = get_messenger();
		}
	}



	return 0;
}


/**
 *
 * @param entry
 * @param size
 * @param dest
 * TODO: Sanitize function
 */
static void get_file_name(const entry_t *entry, int size, char *dest) {
	const options_t *options = get_options_object();

	bzero(dest, size);
	snprintf(dest, size, "%s%s%s", options->destdir, FILE_SEPARATOR,
			entry->id.name);
}

static FILE *open_file(entry_t *entry, CURL *easy) {
	char dest_file[MAX_DATA_LEN];
	FILE *ret = NULL;
	messenger msg = get_messenger();

	get_file_name(entry, sizeof(dest_file) - 1, dest_file);

	ret = fopen(dest_file, "a");

	if (!ret) {
		msg(ERROR, "Unable to open file: %s\n", strerror(errno));

		return false;
	}

	if (exists(dest_file)) {
		msg(INFO, "File %s already exists. Checking if we can resume the download",
				dest_file);
		fseek(ret, 0L, SEEK_END);
		long pos = ftell(ret);

		if (pos == EOF) {
			msg(ERROR, "Download %s cannot be resumed: unhandled error");
			ret = freopen(NULL, "w+", ret);
			if (!ret) {
				msg = get_messenger();

				msg(ERROR, "Unable to open file: %s", strerror(errno));

				return false;
			}
		}
		else {
			msg(INFO, "Already downloaded %li bytes from %s", pos, dest_file);
			curl_easy_setopt(easy, CURLOPT_RESUME_FROM, pos);
			curl_easy_setopt(easy, CURLOPT_SEEKFUNCTION, fseek);
			curl_easy_setopt(easy, CURLOPT_SEEKDATA, ret);
		}
	}

	return ret;
}

static void delete_file(entry_t *entry) {
	char dest_file[MAX_DATA_LEN];
	messenger msg = get_messenger();

	get_file_name(entry, sizeof(dest_file) - 1, dest_file);

	if (exists(dest_file)) {
		msg(INFO, "Removing file %s", entry->id.name);
		if (unlink(dest_file) != 0) {
			msg(ERROR, "Unable to delete file %s: %s", entry->id.name,
					strerror(errno));
		}
	}
}

static bool download_file(entry_t *entry) {
	CURL *easy;
	CURLcode result;
	char buffer[CURL_ERROR_SIZE];
	FILE *file;
	messenger msg = get_messenger();

	easy = curl_easy_init();
	if (!easy) {
		fprintf(stderr, "Unable to get an easy download handle\n");
	}

	file = open_file(entry, easy);
	if (!file) {
		return false;
	}

	curl_easy_setopt(easy, CURLOPT_ERRORBUFFER, buffer);
	curl_easy_setopt(easy, CURLOPT_URL, entry->url);
	curl_easy_setopt(easy, CURLOPT_HEADER, 0);
	curl_easy_setopt(easy, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, fwrite);
	curl_easy_setopt(easy, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(easy, CURLOPT_PROGRESSFUNCTION, progress_callback);
	curl_easy_setopt(easy, CURLOPT_PROGRESSDATA, entry);
	curl_easy_setopt(easy, CURLOPT_USERAGENT, DS_USER_AGENT "/" DS_VERSION);

	const char *proxy = getenv("http_proxy");
	if (proxy) {
		curl_easy_setopt(easy, CURLOPT_PROXY, proxy);
	}

	entry->status = STARTED;
	msg(INFO, "Downloading file %s from %s", entry->id.name, entry->url);
	curl_easy_perform(easy);
	msg(INFO, "File %s successfully downloaded from %s", entry->id.name,
			entry->url);

	curl_easy_cleanup(easy);
	fclose(file);

	entry->status = COMPLETED;
}

static void *download_file_main(void *ptr) {
	bool ret = false;

	ret = download_file((entry_t *) ptr);
	pthread_exit(NULL);

}

static download_thread_t *new_download_thread(entry_t *entry) {
	int sys_ret = 0;
	messenger msg = get_messenger();
	download_thread_t *ret = (download_thread_t *) malloc(sizeof(download_thread_t));

	if (!ret) {
		msg(ERROR, "Unable to create a new download thread object");

		return NULL;
	}

	bzero(ret, sizeof(download_thread_t));
	ret->entry = entry;

	sys_ret = pthread_create(&ret->thread, NULL, download_file_main, ret->entry);

	if (sys_ret != 0) {
		msg(ERROR, "Unable to create a download thread object: %s", strerror(errno));
		free(ret);

		return NULL;
	}

	msg(INFO, "Download thread created successfully");
	return ret;
}


void download_start(entry_t *entry) {
	download_thread_t *download_thread = NULL;
	messenger msg = get_messenger();

	if (!thread_list) {
		thread_list = list_new();

		if (!thread_list) {
			msg(ERROR, "Unable to create thread list");

			return;
		}
	}

	download_thread = new_download_thread(entry);
	if (!download_thread) {
		msg(ERROR, "The download did not start");
	}

	list_append(thread_list, download_thread);
}


static bool download_thread_compare_id(const void *data, const void *other, void *result) {
	download_thread_t *thread = (download_thread_t *) data;
	const trx_id_t *id = (const trx_id_t *) other;

	messenger msg = get_messenger();

	msg(DEBUG, "Comparing %s with %s", thread->entry->id.name, id->name);


	if (strcmp(thread->entry->id.name, id->name) == 0) {
		return true;
	}

	return false;
}


bool download_pause(const trx_id_t *id) {
	bool ret = false;
	download_thread_t *thread = NULL;
	messenger msg = get_messenger();

	msg(DEBUG, "Pausing download %s", id->name);

	thread = (download_thread_t *) list_get_item(thread_list, download_thread_compare_id, id);
	if (!thread) {

		msg(DEBUG, "Not found", id->name);
		return false;
	}

	thread->entry->status = PAUSED;
	pthread_cancel(thread->thread);

	ret = list_remove_item(thread_list, download_thread_compare_id, &thread->entry->id);
	free(thread);

	msg(DEBUG, "Download successfully paused");
	return ret;
}


bool download_cancel(const trx_id_t *id) {
	bool ret = false;
	download_thread_t *thread = NULL;
	messenger msg = get_messenger();

	msg(DEBUG, "Cancelling download %s", id->name);

	thread = (download_thread_t *) list_get_item(thread_list, download_thread_compare_id, id);
	if (!thread) {

		msg(DEBUG, "Not found", id->name);
		return false;
	}

	thread->entry->status = PAUSED;
	pthread_cancel(thread->thread);

	msg(DEBUG, "Removing item %s", id->name);
	ret = list_remove_item(thread_list, download_thread_compare_id, thread);
	delete_file(thread->entry);
	free(thread);

	msg(DEBUG, "Download successfully cancelled");
	return ret;
}
