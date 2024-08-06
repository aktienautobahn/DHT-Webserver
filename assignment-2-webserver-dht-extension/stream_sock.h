#ifndef STREAM_SOCK_H
#define STREAM_SOCK_H

bool handle_connection(struct connection_state* state, struct NetworkNodes node, int dgram_socket, DHTLookupMessage *lookup_msg, DHTLookupMessage lookupMessages[], int *nextFreeIndex);


#endif