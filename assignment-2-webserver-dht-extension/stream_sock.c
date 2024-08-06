#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "data.h"
#include "http.h"
#include "util.h"

#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>

#include "chord_processor.h"

#define MAX_RESOURCES 100


struct tuple resources[MAX_RESOURCES] = {
    {"/static/foo", "Foo", sizeof "Foo" - 1},
    {"/static/bar", "Bar", sizeof "Bar" - 1},
    {"/static/baz", "Baz", sizeof "Baz" - 1}
};


/**
 * Sends an HTTP reply to the client based on the received request.
 *
 * @param conn      The file descriptor of the client connection socket.
 * @param request   A pointer to the struct containing the parsed request information.
 */
void send_reply(int conn, struct request* request) {

    // Create a buffer to hold the HTTP reply
    char buffer[HTTP_MAX_SIZE];
    char *reply = buffer;


    if (strcmp(request->method, "GET") == 0) {
        // Find the resource with the given URI in the 'resources' array.
        size_t resource_length;
        const char* resource = get(request->uri, resources, MAX_RESOURCES, &resource_length);

        // check if responsible

        if (resource) {
            sprintf(reply, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n\r\n%.*s", resource_length, (int) resource_length, resource);
        } else {
            reply = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        }
    } else if (strcmp(request->method, "PUT") == 0) {
        // Try to set the requested resource with the given payload in the 'resources' array.
        if (set(request->uri, request->payload, request->payload_length, resources, MAX_RESOURCES
        )) {
            reply = "HTTP/1.1 204 No Content\r\n\r\n";
        } else {
            reply = "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n";
        }
    } else if (strcmp(request->method, "DELETE") == 0) {
        // Try to delete the requested resource from the 'resources' array
        if (delete(request->uri, resources, MAX_RESOURCES)) {
            reply = "HTTP/1.1 204 No Content\r\n\r\n";
        } else {
            reply = "HTTP/1.1 404 Not Found\r\n\r\n";
        }
    } else {
        reply = "HTTP/1.1 501 Method Not Supported\r\n\r\n";
    }

    // Send the reply back to the client
    if (send(conn, reply, strlen(reply), 0) == -1) {
        perror("send");
        close(conn);
    }
}

/**
 * PROCESS PACKET: Handles and processes an incoming packet from a client connection.
 *
 * This function is responsible for processing incoming packets received on a specified connection. It interprets the packet
 * data, performs necessary actions based on the packet content, and may send replies back to the client. This function also
 * handles DHT-related messages, if applicable. It is capable of managing malformed packets and various error scenarios during
 * processing.
 *
 * @param conn The socket descriptor representing the connection to the client.
 * @param buffer A pointer to the buffer containing the incoming packet's data.
 * @param n The size of the incoming packet in bytes.
 * @param node A NetworkNodes structure representing the current node's state in the DHT.
 * @param dram_socket The socket descriptor for the DHT's UDP communication.
 * @param lookup_msg A pointer to a DHTLookupMessage structure for handling DHT messages.
 * @param lookupMessages An array of DHTLookupMessage structures for storing DHT lookup messages.
 * @param nextFreeIndex A pointer to an integer tracking the next free index in the lookupMessages array.
 *
 * @return The number of bytes processed from the packet. If the packet is successfully processed, the return value
 *         indicates the number of bytes processed. If the packet is malformed or an error occurs, the return value is -1.
 */
size_t process_packet(int conn, char* buffer, size_t n, struct NetworkNodes node, int dram_socket, DHTLookupMessage *lookup_msg, DHTLookupMessage lookupMessages[], int *nextFreeIndex) {
    
    struct request request = {
        .method = NULL,
        .uri = NULL,
        .payload = NULL,
        .payload_length = -1
    };
    ssize_t bytes_processed = parse_request(buffer, n, &request);

    if (bytes_processed > 0) {

        // is responsible
        if (is_responsible_hashed(hash(request.uri), node.self_id, node.pred.id)) {
            send_reply(conn, &request);


        } else {
            char buffer[HTTP_MAX_SIZE];
            char *reply = buffer; 
            // is successor responsible
            if (is_responsible_hashed(hash(request.uri), node.succ.id, node.self_id)) {
                sprintf(reply, "HTTP/1.1 303 See Other\r\nLocation: http://%s:%d%s\r\nContent-Length: 0\r\n\r\n", inet_ntoa(node.succ.ip), node.succ.port, request.uri);//, resource_length, (int) resource_length, resource) 
            } else {
            // is reply message? iterate in array[10]  if found reply exists --> 303
            DHTLookupMessage *foundMessage = findDHTreply(lookupMessages, hash(request.uri), nextFreeIndex);
            // if reply message exists, send 303
            if (foundMessage != NULL) {
                sprintf(reply, "HTTP/1.1 303 See Other\r\nLocation: http://%s:%d%s\r\nContent-Length: 0\r\n\r\n", inet_ntoa(foundMessage->originNodeIP), foundMessage->originNodePort, request.uri);//, resource_length, (int) resource_length, resource)
                // disallocate memory
                free(foundMessage);

            } else {
            // else put off till later with 503 
            reply = "HTTP/1.1 503 Service Unavailable\r\nRetry-After: 1\r\nContent-Length: 0\r\n\r\n";

            // LOOKUP INIT (initial lookup, if other condition are not fulfilled)
            lookup_dht(dram_socket, node, hash(request.uri), lookup_msg);
            }
            }

            // send reply to client over TCP HTTP
            if (send(conn, reply, strlen(reply), 0) == -1) {
                perror("send");
                close(conn);
            }

        }

            // Check the "Connection" header in the request to determine if the connection should be kept alive or closed.
            const string connection_header = get_header(&request, "Connection");
            if (connection_header && strcmp(connection_header, "close")) {
                return -1;
            } 
       
    } else if (bytes_processed == -1) {
        // If the request is malformed or an error occurs during processing, send a 400 Bad Request response to the client.
        const string bad_request = "HTTP/1.1 400 Bad Request\r\n\r\n";
        send(conn, bad_request, strlen(bad_request), 0);
        printf("Received malformed request, terminating connection.\n");
        close(conn);
        return -1;
    }

    return bytes_processed;
}




/**
 * Discards the front of a buffer
 *
 * @param buffer A pointer to the buffer to be modified.
 * @param discard The number of bytes to drop from the front of the buffer.
 * @param keep The number of bytes that should be kept after the discarded bytes.
 *
 * @return Returns a pointer to the first unused byte in the buffer after the discard.
 * @example buffer_discard(ABCDEF0000, 4, 2):
 *          ABCDEF0000 ->  EFCDEF0000 -> EF00000000, returns pointer to first 0.
 */
char* buffer_discard(char* buffer, size_t discard, size_t keep) {
    memmove(buffer, buffer + discard, keep);
    memset(buffer + keep, 0, discard);  // invalidate buffer
    return buffer + keep;
}

/**
 * HANDLE CONNECTION: Manages incoming connections and processes data received through the socket.
 *
 * This function is responsible for handling an active connection represented by the connection_state structure. It reads data
 * from the socket, processes the received packets, and performs necessary actions based on the packet contents. The function
 * integrates with DHT functionality, handling DHT-related messages as part of the data processing. In case of an error during
 * data reception or processing, appropriate error handling is performed.
 *
 * @param state A pointer to the connection_state structure containing the current state of the connection, including the buffer
 *              and the socket descriptor.
 * @param node A NetworkNodes structure representing the current node's state in the DHT.
 * @param dgram_socket The socket descriptor for the DHT's UDP communication.
 * @param lookup_msg A pointer to a DHTLookupMessage structure for handling DHT messages.
 * @param lookupMessages An array of DHTLookupMessage structures for storing DHT lookup messages.
 * @param nextFreeIndex A pointer to an integer tracking the next free index in the lookupMessages array.
 *
 * @return Returns true if the connection and data processing were successful, false if the connection is closed or an error
 *         occurs in data processing. In case of a receive error, the function exits the program.
 */

bool handle_connection(struct connection_state* state, struct NetworkNodes node, int dgram_socket, DHTLookupMessage *lookup_msg, DHTLookupMessage lookupMessages[], int *nextFreeIndex) {
    // Calculate the pointer to the end of the buffer to avoid buffer overflow
    const char* buffer_end = state->buffer + HTTP_MAX_SIZE;

    // Check if an error occurred while receiving data from the socket
    ssize_t bytes_read = recv(state->sock, state->end, buffer_end - state->end, 0);
    if (bytes_read == -1) {
        perror("recv");
        close(state->sock);
        exit(EXIT_FAILURE);
    } else if (bytes_read == 0) {
        return false;
    }

    char* window_start = state->buffer;
    char* window_end = state->end + bytes_read;

    ssize_t bytes_processed = 0;
    while((bytes_processed = process_packet(state->sock, window_start, window_end - window_start, node, dgram_socket, lookup_msg, lookupMessages, nextFreeIndex)) > 0) {
        window_start += bytes_processed;
    }
    if (bytes_processed == -1) {
        return false;
    }

    state->end = buffer_discard(state->buffer, window_start - state->buffer, window_end - window_start);
    return true;
}
