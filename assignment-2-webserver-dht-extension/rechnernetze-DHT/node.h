    #pragma once
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h> // For in_addr
#include <stdbool.h>

#define MAX_LOOKUP_MESSAGES 10 // Define the maximum number of messages


struct NodeInfo {
    uint16_t id;          // For storing the ID
    struct in_addr ip;    // For storing the IP address
    uint16_t port;        // For storing the port number
};

struct NetworkNodes {
    uint16_t self_id; 
    struct NodeInfo pred; // Predecessor node information
    struct NodeInfo succ; // Successor node information
};


typedef struct _DHTLookupMessage {
    uint8_t messageType; // 1 byte, e.g., 0x01 for LOOKUP
    uint16_t key;        // 2 bytes (16-bit hash)
    uint16_t originNodeID;       // 2 bytes
    struct in_addr originNodeIP; // 4 bytes for IPv4
    uint16_t originNodePort;     // 2 bytes
} DHTLookupMessage;



struct NetworkNodes derive_nodes_data(const char* nodeId);

void addOrUpdateMessage(DHTLookupMessage lookupMessages[], int *nextFreeIndex, DHTLookupMessage newMessage);

DHTLookupMessage* findDHTreply(DHTLookupMessage lookupMessages[], uint16_t key, int *nextFreeIndex);

int construct_dht_lookup_message(const DHTLookupMessage *lookup_msg, char *buffer);


void parse_dht_lookup_message(DHTLookupMessage *lookup_msg, char *buffer);
void lookup_dht(int socket, struct NetworkNodes own_node, u_int16_t key, DHTLookupMessage *lookup_msg);


uint16_t hash(const char* str);

/*Takes the Hash-Room-Values of the server and its predicessor and decides, wheither the server
	responsible for the data*/
bool is_responsible(char* path, uint16_t node_id, uint16_t pred_id);
bool is_responsible_hashed(uint16_t key, uint16_t node_id, uint16_t pred_id);
