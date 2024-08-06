#ifndef SOCKETS_SETUP_H
#define SOCKETS_SETUP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include "http.h"

struct sockaddr_in derive_sockaddr(const char* host, const char* port);

int setup_stream_socket(struct sockaddr_in addr);

int setup_datagram_socket(struct sockaddr_in addr);

void connection_setup(struct connection_state* state, int sock);


#endif