
#include "node.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h> // For in_addr
#include <string.h>     // For memset
#include <arpa/inet.h>  // For inet_pton
#include <stdio.h> // remove later
#include <stdbool.h> // for bool
#include <openssl/sha.h>
#include "http.h"



/**
 * SHIFT MESSAGES LEFT: Shifts all messages in the lookupMessages array to the left by one position.
 *
 * This function is used to remove the first element from the array and shift all subsequent elements one position to the left.
 * It is typically called when an element is removed from the array, and the array needs to be compacted.
 *
 * @param lookupMessages[] The array of DHTLookupMessage from which the first message is to be removed, and the rest shifted left.
 */
void shiftMessagesLeft(DHTLookupMessage lookupMessages[]) {
    for (int i = 0; i < MAX_LOOKUP_MESSAGES - 1; i++) {
        lookupMessages[i] = lookupMessages[i + 1];
    }
}

/**
 * MESSAGES ARE EQUAL: Compares two DHTLookupMessage structures for equality.
 *
 * This function checks if two DHTLookupMessage structures are equal based on their key and originNodeID fields.
 * It is used to determine if two messages represent the same data. Additional conditions can be added to the function
 * if more criteria are needed for determining equality.
 *
 * @param a The first DHTLookupMessage structure to compare.
 * @param b The second DHTLookupMessage structure to compare.
 *
 * @return Returns true if the messages are considered equal, false otherwise.
 */
bool messagesAreEqual(DHTLookupMessage a, DHTLookupMessage b) {
    // comparison logic
    return (a.key == b.key) && (a.originNodeID == b.originNodeID);
    //  more conditions needed?
}

/**
 * ADD OR UPDATE MESSAGE: Manages the insertion or update of DHTLookupMessage entries in the lookupMessages array. 
 * 
 * This function checks if a message with the same key already exists in the array. If it does, the existing message is updated 
 * with the new information. If not, the new message is added to the next available index in the array. In the case where 
 * the array is full, the oldest message is removed to make space for the new message.
 * 
 * @param lookupMessages[] The array of DHTLookupMessage where new messages are added or existing messages are updated.
 * @param nextFreeIndex A pointer to an integer that tracks the next free index in the lookupMessages array.
 * @param newMessage The DHTLookupMessage to be added or used for updating an existing message in the array.
 */
void addOrUpdateMessage(DHTLookupMessage lookupMessages[], int *nextFreeIndex, DHTLookupMessage newMessage) {
    // Search for existing message
 
    int foundIndex = -1;
    for (int i = 0; i < MAX_LOOKUP_MESSAGES; i++) {
        if (messagesAreEqual(lookupMessages[i], newMessage)) {
            foundIndex = i;
            break;
        }
    }

    // Replace existing message or add new message
    if (foundIndex != -1) {
        lookupMessages[foundIndex] = newMessage;
    } else {
        if (*nextFreeIndex < MAX_LOOKUP_MESSAGES) {
            lookupMessages[*nextFreeIndex] = newMessage;
            (*nextFreeIndex)++;
        } else {
            // Shift messages and add new message at the end
            shiftMessagesLeft(lookupMessages);
            lookupMessages[MAX_LOOKUP_MESSAGES - 1] = newMessage;
        }
    }
}

/**
 * FIND DHT REPLY: Searches for a DHTLookupMessage in the lookupMessages array based on a given key and removes it from the array.
 *
 * This function iterates through the lookupMessages array to find a message where the messageType is 1 and the message key
 * matches the provided key, according to the is_responsible_hashed function's criteria. If such a message is found, the function:
 *   - Creates a dynamic copy of the message.
 *   - Shifts the remaining elements in the array to fill the gap.
 *   - Decrements the next free index pointer, as the array now contains one less element.
 * If no matching message is found, the function returns NULL. The caller is responsible for freeing the memory of the returned
 * DHTLookupMessage* when it is no longer needed.
 *
 * @param lookupMessages[] The array of DHTLookupMessage from which the message is to be searched and removed.
 * @param key The key to be used for finding the DHTLookupMessage.
 * @param nextFreeIndex A pointer to an integer that tracks the next free index in the lookupMessages array.
 * @return A pointer to the found DHTLookupMessage, or NULL if no matching message is found.
 */
DHTLookupMessage* findDHTreply(DHTLookupMessage lookupMessages[], uint16_t key, int *nextFreeIndex) {
    for (int i = 0; i < *nextFreeIndex; i++) {
        if (lookupMessages[i].messageType == 1) {
            if (is_responsible_hashed(key, lookupMessages[i].originNodeID, lookupMessages[i].key)) {

                // Create a copy of the found message
                DHTLookupMessage* foundMessage = malloc(sizeof(DHTLookupMessage));
                if (foundMessage == NULL) {
                    // Handle memory allocation error
                    return NULL;
                }
                *foundMessage = lookupMessages[i];

                // Shift the rest of the elements
                for (int j = i; j < *nextFreeIndex - 1; j++) {
                    lookupMessages[j] = lookupMessages[j + 1];
                }

                // Decrement the next free index as we've removed an element
                (*nextFreeIndex)--;

                return foundMessage;
            }
        }
    }
    return NULL;
}

/**
 * PRINT BUFFER AS HEX: Prints the contents of a buffer in hexadecimal and ASCII format.
 *
 * This function iterates through the provided buffer and prints each byte in hexadecimal format. It also prints the ASCII
 * representation of the buffer, using a dot ('.') for non-printable characters. This is useful for debugging and visualizing
 * binary data in a human-readable form.
 *
 * @param buffer The buffer whose contents are to be printed.
 * @param length The length of the buffer.
 */
void print_buffer_as_hex(const char *buffer, int length) {
    fprintf(stderr, "Ergebnis aus dem Constructor (Hex): ");
    for (int i = 0; i < length; ++i) {
        fprintf(stderr, "%02x ", (unsigned char)buffer[i]);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "Ergebnis aus dem Constructor (ASCII): ");
    for (int i = 0; i < length; ++i) {
        unsigned char ch = (unsigned char)buffer[i];
        if (ch >= 32 && ch <= 126) {  // Printable ASCII range
            fprintf(stderr, "%c", ch);
        } else {
            fprintf(stderr, ".");  // Non-printable character
        }
    }
    fprintf(stderr, "\n");
}


	/*Define the hash function */
uint16_t hash(const char* str){
	uint8_t digest[SHA256_DIGEST_LENGTH];
	SHA256((uint8_t*)str, strlen(str), digest);
	return htons(*((uint16_t*)digest));
}



/**
 * IS RESPONSIBLE: Determines if a node is responsible for a given path based on its hashed value.
 *
 * This function hashes the provided path and determines whether the resulting hash falls within the range of the node's identifier
 * and its predecessor's identifier. It handles both the normal and edge cases (where the identifiers wrap around).
 *
 * @param path The path or key to be hashed and checked.
 * @param node_id The identifier of the current node.
 * @param pred_id The identifier of the predecessor node.
 * @return True if the node is responsible for the path, otherwise False.
 */
bool is_responsible(char* path, uint16_t node_id, uint16_t pred_id){
	// hash the path
	uint16_t hashValue = hash(path);

	// Decide, wheither the node is responsible for the data
  	// Normal case: pred_id < node_id
    if (pred_id < node_id) {
        return hashValue > pred_id && hashValue <= node_id;
    }
    // Edge case: pred_id > node_id, wrap around
    return hashValue > pred_id || hashValue <= node_id;
}

/**
 * IS RESPONSIBLE HASHED: Determines if a node is responsible for a given hashed key.
 *
 * Similar to 'is_responsible', this function checks if the provided key falls within the range of the node's identifier and
 * its predecessor's identifier. It is specifically used when the key is already hashed.
 *
 * @param key The hashed key to be checked.
 * @param node_id The identifier of the current node.
 * @param pred_id The identifier of the predecessor node.
 * @return True if the node is responsible for the key, otherwise False.
 */
bool is_responsible_hashed(uint16_t key, uint16_t node_id, uint16_t pred_id){

	// Decide, wheither the node is responsible for the data

    if (pred_id < node_id) {   	// Normal case: pred_id < node_id
        return key > pred_id && key <= node_id;
    } else if (pred_id > node_id) {    // Edge case: pred_id > node_id, wrap around
    	return key > pred_id || key <= node_id;
	} else {
		// single node in the DHT
		return 1;
	}

}

/**
 * DERIVE NODES DATA: Constructs a NetworkNodes structure from environment variables and the provided node identifier.
 *
 * This function reads environment variables to set up the successor and predecessor node information and combines it with
 * the provided node identifier to create a complete NetworkNodes structure. It is useful for initializing node data in
 * a distributed hash table system.
 *
 * @param nodeId The identifier of the current node.
 * @return A NetworkNodes structure populated with the current node, successor, and predecessor information.
 */
struct NetworkNodes derive_nodes_data(const char* nodeId) {

    struct NetworkNodes node;
    memset(&node, 0, sizeof(node));

    // Get environment variables
    char *SUCC_ID = getenv("SUCC_ID");
    char *SUCC_IP = getenv("SUCC_IP");
    char *SUCC_PORT = getenv("SUCC_PORT");

    char *PRED_ID = getenv("PRED_ID");
    char *PRED_IP = getenv("PRED_IP");
    char *PRED_PORT = getenv("PRED_PORT");


    // Convert and assign to struct
    if (nodeId) node.self_id = (uint16_t)atoi(nodeId); 

    if (SUCC_ID) node.succ.id = (uint16_t)atoi(SUCC_ID);
    if (SUCC_IP) inet_pton(AF_INET, SUCC_IP, &node.succ.ip);
    if (SUCC_PORT) node.succ.port = (uint16_t)atoi(SUCC_PORT);

    if (PRED_ID) node.pred.id = (uint16_t)atoi(PRED_ID);
    if (PRED_IP) inet_pton(AF_INET, PRED_IP, &node.pred.ip);
    if (PRED_PORT) node.pred.port = (uint16_t)atoi(PRED_PORT);


    return node;
}

/**
 * CONSTRUCT DHT LOOKUP MESSAGE: Serializes a DHTLookupMessage into a binary format for network transmission.
 *
 * This function converts the DHTLookupMessage fields into network byte order and packs them into the provided buffer.
 * It assumes that the originNodeIP is already in network byte order. The function returns the size of the serialized data.
 *
 * @param lookup_msg A pointer to the DHTLookupMessage to be serialized.
 * @param buffer The buffer where the serialized message is stored.
 * @return The size of the serialized message in bytes.
 */
int construct_dht_lookup_message(const DHTLookupMessage *lookup_msg, char *buffer) {
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &lookup_msg->originNodeIP, ipStr, sizeof(ipStr));

    //TODO: error handling for constructor, if there is no data passed
    // Serialize fields into binary format
    buffer[0] = lookup_msg->messageType;

    uint16_t keyNet = htons(lookup_msg->key);
    memcpy(buffer + 1, &keyNet, sizeof(keyNet));
    
    uint16_t originNodeIDNet = htons(lookup_msg->originNodeID);
    memcpy(buffer + 3, &originNodeIDNet, sizeof(originNodeIDNet));
    
    // Assuming originNodeIP is already in network byte order
    memcpy(buffer + 5, &lookup_msg->originNodeIP, sizeof(lookup_msg->originNodeIP));
    
    uint16_t originNodePortNet = htons(lookup_msg->originNodePort);
    memcpy(buffer + 9, &originNodePortNet, sizeof(originNodePortNet));
  

    // print_buffer_as_hex(buffer, 11);     // Print the buffer content as hexadecimal 


    return 11;
}

/**
 * PARSE DHT LOOKUP MESSAGE: Deserializes a binary format message into a DHTLookupMessage structure.
 *
 * This function extracts and converts the fields from the network byte order to the host byte order and populates
 * the DHTLookupMessage structure. It handles the conversion of message type, key, originNodeID, and originNodePort from
 * the buffer to the DHTLookupMessage.
 *
 * @param lookup_msg A pointer to the DHTLookupMessage where the deserialized data will be stored.
 * @param buffer The buffer containing the serialized message.
 */
void parse_dht_lookup_message(DHTLookupMessage *lookup_msg, char *buffer) {
    uint8_t messageTypeNet = 0; 
    uint16_t keyNet = 0, originNodeIDNet = 0, originNodePortNet = 0;

    // Convert from network byte order to host byte order
    messageTypeNet = buffer[0];
    lookup_msg->messageType = messageTypeNet;

    memcpy(&keyNet, buffer + 1, 2); // parse two bytes
    lookup_msg->key = ntohs(keyNet);

    memcpy(&originNodeIDNet, buffer + 3, 2);  // parse two bytes
    lookup_msg->originNodeID  = ntohs(originNodeIDNet);

    //  originNodeIP is already in network byte order
    memcpy(&lookup_msg->originNodeIP, buffer + 5, 4);

    
    memcpy(&originNodePortNet, buffer + 9, 2);  
    
    lookup_msg->originNodePort = ntohs(originNodePortNet);




}


/**
 * LOOKUP DHT: Performs a DHT lookup operation using the given socket and network node information.
 *
 * This function prepares a DHTLookupMessage with the provided hashed key and network node information. It constructs
 * the DHT lookup message and sends it to the successor node. The function handles the retrieval of the local socket's IP
 * and port, populates the lookup message, constructs the message, and sends it to the successor node.
 *
 * @param socket The socket used for sending the DHT lookup message.
 * @param own_node The NetworkNodes structure containing information about the current node.
 * @param hashed_key The hashed key for which the responsible node is being looked up.
 * @param lookup_msg A pointer to the DHTLookupMessage to be sent.
 */
void lookup_dht(int socket, struct NetworkNodes own_node, uint16_t hashed_key, DHTLookupMessage *lookup_msg) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    // Create a buffer to hold the HTTP reply
    char send_buffer[HTTP_MAX_SIZE+1];


    // Use getsockname to get the socket's local IP and port
    if (getsockname(socket, (struct sockaddr *)&addr, &addr_len) < 0) {
        perror("getsockname failed");
        exit(EXIT_FAILURE);
    }

    uint16_t port = ntohs(addr.sin_port);


    // lookup msg muss befuellt werden
    lookup_msg->key = hashed_key;
    lookup_msg->originNodeID = own_node.self_id;
    lookup_msg->originNodeIP = addr.sin_addr;
    lookup_msg->originNodePort = port;


    struct sockaddr_in successor_addr;
    memset(&successor_addr, 0, sizeof(successor_addr));
    successor_addr.sin_family = AF_INET;
    successor_addr.sin_addr = own_node.succ.ip;
    successor_addr.sin_port = htons(own_node.succ.port);

    memset(&send_buffer, 0, sizeof(send_buffer));

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &lookup_msg->originNodeIP, ipStr, sizeof(ipStr));
    // construct msg
    int lookup_size = construct_dht_lookup_message(lookup_msg, send_buffer);

    // sendto msg
    int sent_bytes = sendto(socket, send_buffer, lookup_size, 0, (struct sockaddr *)&successor_addr, sizeof(successor_addr));
                 
    if (sent_bytes == -1) {
        perror("sendto failed");
        // Handle the error
    }

}



