#ifndef NODE_H
#define NODE_H

#include "node.h"
#include <poll.h>

void chord_processor(struct sockaddr_in addr, int stream_socket, int dram_socket, struct NetworkNodes own_node) ;

#endif