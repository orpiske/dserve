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

#include "connection.h"

static struct sockaddr_in new_sockaddr(const char *host, short port) {
	struct sockaddr_in sockaddr;

	bzero(&sockaddr, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	if (host != NULL) {
		sockaddr.sin_addr.s_addr = inet_addr(host);
	}
	else {
		sockaddr.sin_addr.s_addr = INADDR_ANY;
	}

	return sockaddr;
}

static connection_t *connection_new_for_accept() {
	connection_t *ret = NULL;

	ret = (connection_t *) malloc(sizeof(connection_t));
	if (!ret) {
		fprintf(stderr, "Not enough memory to accept a client connection\n");

		return NULL;
	}

	bzero(ret, sizeof(connection_t));
	return ret;
}

static bool connection_is_valid_address(connection_t *connection, const char *host) {
	if (connection->sock_addr.sin_addr.s_addr == INADDR_NONE) {
		fprintf(stderr, "Invalid address: %s\n", host);

		return false;
	}

	return true;
}

connection_t *connection_new(const char *host, short port) {
	connection_t *connection = NULL;

	connection = (connection_t *) malloc(sizeof(connection_t));
	if (!connection) {
		fprintf(stderr, "Not enough memory to allocate for a new connection\n");

		return NULL;
	}

	connection->sock_addr = new_sockaddr(host, port);
	if (host != NULL) {
		if (!connection_is_valid_address(connection, host)) {
			free(connection);

			return NULL;
		}
	}

	connection->socket = socket(PF_INET, SOCK_STREAM, 0);
	if (connection->socket == -1) {
		fprintf(stderr, "Unable to create socket: %s\n", strerror(errno));

		free(connection);
		return NULL;
	}

	int val = 1;
	setsockopt(connection->socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(1));
	setsockopt(connection->socket, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(1));

	return connection;
}


bool connection_open(connection_t *connection) {
	int ret = 0;
	int len_inet = sizeof(connection->sock_addr);

	ret = bind(connection->socket, SOCK_CAST(connection->sock_addr), len_inet);
	if (ret == -1) {
		fprintf(stderr, "Unable to bind socket: %s\n", strerror(errno));

		return false;
	}

	ret = listen(connection->socket, 10);
	if (ret == -1) {
		fprintf(stderr, "Unable to open port for listening: %s\n", strerror(errno));

		return false;
	}

	return true;
}

bool connection_connect(connection_t *connection) {
	int ret = 0;
	int len_inet = sizeof(connection->sock_addr);

	ret = connect(connection->socket, SOCK_CAST(connection->sock_addr), len_inet);
	if (ret == -1) {
		fprintf(stderr, "Unable to connect to remote host\n", strerror(errno));

		return false;
	}

	return true;
}


connection_t *connection_accept(const connection_t *connection) {
	struct sockaddr_in client_addr;
	int len_inet;
	connection_t *client_connection = NULL;

	client_connection = connection_new_for_accept();

	client_connection->socket = accept(connection->socket,
			SOCK_CAST(client_connection->sock_addr), &len_inet);
	if (client_connection->socket == -1) {
		fprintf(stderr, "Unable to accept connection: %s\n", strerror(errno));

		free(client_connection);
		return NULL;
	}

	fprintf(stdout, "Connection received from %s\n",
			inet_ntoa(client_connection->sock_addr.sin_addr));

	return client_connection;
}


ssize_t connection_send(const connection_t *connection, const void *data, size_t size) {
	ssize_t ret = 0;

	ret = send(connection->socket, data, size, MSG_DONTWAIT);
	if (ret < 0 || ret != size) {
		fprintf(stderr, "Send data failed: %s\n", strerror(errno));
	}

	return ret;
}


static int connection_can_read(const connection_t *connection, int seconds) {
	struct timeval timeout;
	fd_set fd;
	int sel = 0;

	FD_ZERO(&fd);
	FD_SET(connection->socket, &fd);

	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;

	sel = select(connection->socket + 1, &fd, NULL, NULL, &timeout);
	if (sel < 0) {
		fprintf(stderr, "Unhandled select() error: %s\n", strerror(errno));
	}

	if (sel == 0) {
		//fprintf(stderr, "Connection timeout\n");
	}

	return sel;
}

ssize_t connection_read(const connection_t *connection, int seconds, void *data,
		size_t size)
{
	ssize_t num_read;
	int sel = 0;
	socklen_t len;

	sel = connection_can_read(connection, seconds);
	if (sel <= 0) {
		return CONNECTION_NO_DATA;
	}

	memset(data, 0, size);
	num_read = recvfrom(connection->socket, data, size, MSG_DONTWAIT,
			SOCK_CAST(connection->sock_addr), &len);

	if (num_read != size) {
		if (num_read == 0) {
			return CONNECTION_DISCONNECTED;
		}
		fprintf(stderr, "Read error: read size %zu does not match expected size %zu\n",
				num_read, size);

		return CONNECTION_ERROR;
	}

	return num_read;
}


connection_t *connection_close(connection_t *connection) {
	close(connection->socket);
	free(connection);

	return NULL;
}
