#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define SOCK_CAST(sock_addr) ((struct sockaddr *) & sock_addr)

#define CONNECTION_NO_DATA 0
#define CONNECTION_DISCONNECTED -1
#define CONNECTION_ERROR -2



typedef struct connection_t_ {
	int socket;
	struct sockaddr_in sock_addr;
} connection_t;


connection_t *connection_new(const char *host, short port);
bool connection_open(connection_t *connection);
bool connection_connect(connection_t *connection);
connection_t *connection_accept(const connection_t *connection);

ssize_t connection_send(const connection_t *connection, const void *data, size_t size);
ssize_t connection_read(const connection_t *connection, int seconds, void *data,
		size_t size);
connection_t *connection_close(connection_t *connection);

#endif
