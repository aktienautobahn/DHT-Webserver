#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#include "data.h"
#include "http.h"
#include "util.h"
#include "node.h"
#include "sockets_setup.h"   
#include "stream_sock.h"


#include <stdbool.h>

        
/**
 * NODE CHORD PROCESSOR: processes incoming connection and invokes nessessary functions depending on incoming request (client request or DHT CHORD lookups)
 * 
 * Processes both HTTP requests from clients over TCP as well as DHT Nodes lookups over UDP 
 * @param addr The sockaddr_in structure representing the IP address and port of the server.
 *
 * @param stream_socket represents TCP server socket
 * 
 * @param datagram_socket represents UDP server socket
 * 
 * @param own_node represents CHORD Node
 * 
 */
void chord_processor(struct sockaddr_in addr, int stream_socket, int datagram_socket, struct NetworkNodes own_node) {

    /* -------------------- DECLARATION & INITITALIZATION OF VARIABLES -------------------- */
    int connection;
    // Create an array of pollfd structures to monitor sockets.
    struct pollfd sockets[3] = {
        { .fd = stream_socket, .events = POLLIN }, // include TCP (stream socket)
        { .fd = datagram_socket, .events = POLLIN}, // include UDP (dgram socket)
        // for the third socket for existing TCP connection
    };

    DHTLookupMessage lookupMessages[MAX_LOOKUP_MESSAGES] = {0}; // declaration/initialization of an array for stroing dht lookup messages
    int nextFreeIndex = 0; // Keep track of the next free index


    char send_buffer[HTTP_MAX_SIZE+1];
    char recv_buffer[HTTP_MAX_SIZE+1];
    // reset the entire buffer
    memset(send_buffer, 0, sizeof(send_buffer));
    memset(recv_buffer, 0, sizeof(recv_buffer));

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    DHTLookupMessage lookup_msg;
    memset(&lookup_msg, 0, sizeof(lookup_msg)); // temp lookup message , send and forget

    struct connection_state state = {0};

    /* -------------------- MAIN LOOP -------------------- */
    while (true) {

        // Use poll() to wait for events on the monitored sockets.
        int ready = poll(sockets, sizeof(sockets) / sizeof(sockets[0]), -1);
        if (ready == -1) {
            if (errno == EINTR) {
                continue; // Retry poll
            } else {
                perror("poll");
                // Handle the error and exit loop
                break;
            }
        }

        // Process events on the monitored sockets.
        for (size_t i = 0; i < sizeof(sockets) / sizeof(sockets[0]); i += 1) {
            if (sockets[i].revents != POLLIN) {
                // If there are no POLLIN events on the socket, continue to the next iteration.
                continue;
            }
            int s = sockets[i].fd;


        /* -------------------- HANDLING NEW TCP CONNECTION -------------------- */
            if (s == stream_socket) {

                // If the event is on the stream_socket, accept a new connection from a client.
                connection = accept(stream_socket, NULL, NULL);
                if (connection == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    close(stream_socket);
                    perror("accept");
                    exit(EXIT_FAILURE);
                } else {
                    connection_setup(&state, connection);

                    // limit to one connection at a time
                    sockets[0].events = 0;
                    sockets[2].fd = connection;
                    sockets[2].events = POLLIN;
                }
        /* -------------------- HANDLING TCP CONNECTION -------------------- */
            } else if (s == datagram_socket) {
                
                /* to handle EWOULDBLOCK/EAGAIN errors in your recvfrom and sendto calls.*/
                int received_bytes = recvfrom(datagram_socket, (char *)recv_buffer, sizeof(recv_buffer), MSG_WAITALL, (struct sockaddr *)&client_addr, &client_addr_len); // later take care of the client address
                if (received_bytes == -1) {
                    perror("recvfrom failed");
                    // Handle the error
                }

                

                parse_dht_lookup_message(&lookup_msg, recv_buffer);

                if (lookup_msg.messageType == 0) {
                    /* -------------------- PROCESS INCOMING LOOKUP MESSAGE -------------------- */

                  

                    if (is_responsible_hashed(lookup_msg.key, own_node.self_id, own_node.pred.id)) {

                        /* -------------------- LOOKUP REPLY TO NODE ORIGIN -------------------- */
                        //TODO: isolate this later
                        // Reply to the origin
                        struct sockaddr_in origin_addr;
                        memset(&origin_addr, 0, sizeof(origin_addr));
                        // construct origin_addr as destination for a reply
                        origin_addr.sin_family = AF_INET;
                        origin_addr.sin_addr = lookup_msg.originNodeIP;
                        origin_addr.sin_port = htons(lookup_msg.originNodePort);
                        // construct reply msg struct
                        lookup_msg.messageType = 1;
                        lookup_msg.key = own_node.pred.id; // --> why in the Aufgabenstellung  Hash ID: ID des Vorgängers der antwortenden Node
                        lookup_msg.originNodeID = own_node.self_id;
                        lookup_msg.originNodeIP = addr.sin_addr;
                        lookup_msg.originNodePort = ntohs(addr.sin_port);

                        int lookup_size = construct_dht_lookup_message(&lookup_msg, send_buffer);
                        int sent_bytes = sendto(datagram_socket, send_buffer, lookup_size, 0, (struct sockaddr *)&origin_addr, sizeof(origin_addr));                       
                        if (sent_bytes == -1) {
                            perror("sendto failed");
                            // Handle the error
                        }
                        // } 
                 
                    } else if (is_responsible_hashed(lookup_msg.key, own_node.succ.id, own_node.self_id)) { // successor responsible? 
                        /* -------------------- LOOKUP REPLY TO NODE ORIGIN FOR SUCCESSOR -------------------- */
                        //TODO: isolate this later
                        struct sockaddr_in origin_addr;
                        memset(&origin_addr, 0, sizeof(origin_addr));

                        // construct origin_addr as destination for a reply
                        origin_addr.sin_family = AF_INET;
                        origin_addr.sin_addr = lookup_msg.originNodeIP;
                        origin_addr.sin_port = htons(lookup_msg.originNodePort);
                        
                        memset(&send_buffer, 0, sizeof(send_buffer));
                        // construct reply msg struct
                        lookup_msg.messageType = 1;
                        lookup_msg.key = own_node.self_id; // --> why in the Aufgabenstellung  Hash ID: ID des Vorgängers der antwortenden Node
                        lookup_msg.originNodeID = own_node.succ.id;
                        lookup_msg.originNodeIP = own_node.succ.ip;
                        lookup_msg.originNodePort = own_node.succ.port;

                        int lookup_size = construct_dht_lookup_message(&lookup_msg, send_buffer);
                        int sent_bytes = sendto(datagram_socket, send_buffer, lookup_size, 0, (struct sockaddr *)&origin_addr, sizeof(origin_addr));                       
                        if (sent_bytes == -1) {
                            perror("sendto failed");
                            // Handle the error
                        }
                        // } 
                    } else {

                        /* -------------------- FORWARD LOOKUP TO SUCCESSOR -------------------- */

                        //TODO: isolate this later
                        struct sockaddr_in successor_addr;
                        memset(&successor_addr, 0, sizeof(successor_addr));
                        successor_addr.sin_family = AF_INET;
                        successor_addr.sin_addr = own_node.succ.ip;
                        successor_addr.sin_port = htons(own_node.succ.port);

                        int lookup_size = construct_dht_lookup_message(&lookup_msg, send_buffer);

                        int sent_bytes = sendto(datagram_socket, send_buffer, lookup_size, 0, (struct sockaddr *)&successor_addr, sizeof(successor_addr));                       
                        if (sent_bytes == -1) {
                            perror("sendto failed");
                            // Handle the error
                        }
                    }

                /* -------------------- PROCESS LOOKUP REPLY MESSAGE -------------------- */
                } else if (lookup_msg.messageType == 1) {

                    // put the msg to the lookup msgs' array (save msg)
                    addOrUpdateMessage(lookupMessages, &nextFreeIndex, lookup_msg);

                }
            } else {

            /* -------------------- HANDLING EXISTING (CLIENT) TCP CONNECTION -------------------- */
                assert(s == state.sock);

                // Call the 'handle_connection' function to process the incoming data on the socket.
                bool cont = handle_connection(&state, own_node, datagram_socket, &lookup_msg, lookupMessages, &nextFreeIndex);
                if (!cont) {  // get ready for a new connection
                    sockets[0].events = POLLIN;
                    sockets[2].fd = -1;
                    sockets[2].events = 0;
                }
            }
        }

    }
}
