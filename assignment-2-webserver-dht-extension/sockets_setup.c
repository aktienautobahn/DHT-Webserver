
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>	
#include <openssl/sha.h>
#include <unistd.h>
#include <netdb.h>
#include "http.h"
#include "sockets_setup.h"


/**
 * Derives a sockaddr_in structure from the provided host and port information.
 *
 * @param host The host (IP address or hostname) to be resolved into a network address.
 * @param port The port number to be converted into network byte order.
 *
 * @return A sockaddr_in structure representing the network address derived from the host and port.
 */
struct sockaddr_in derive_sockaddr(const char* host, const char* port) {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
    };
    struct addrinfo *result_info, *rp;
    struct sockaddr_in result;
    // Resolve the host (IP address or hostname) into a list of possible addresses.
    int returncode = getaddrinfo(host, port, &hints, &result_info);
    if (returncode) {
        fprintf(stderr, "Error parsing host/port");
        exit(EXIT_FAILURE);
    }

    // Loop through the list and pick the first address we can bind to
    for (rp = result_info; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET || rp->ai_family == AF_INET6) {
            result = *((struct sockaddr_in*)rp->ai_addr);
            break;
        }
    }
    if (rp == NULL) { // No address succeeded
        fprintf(stderr, "Could not bind to any address\n");
        exit(EXIT_FAILURE);
    }
    
    // Free the allocated memory for the result_info
    freeaddrinfo(result_info);
    return result;
}

/**
 * Sets up a TCP server socket and binds it to the provided sockaddr_in address.
 *
 * @param addr The sockaddr_in structure representing the IP address and port of the server.
 *
 * @return The file descriptor of the created TCP server socket.
 */
int setup_stream_socket(struct sockaddr_in addr) {
    const int enable = 1;
    const int backlog = 1;

    // Create a socket
    int sock = socket(addr.sin_family, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Avoid dead lock on connections that are dropped after poll returns but before accept is called
    if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

    // Set the SO_REUSEADDR socket option to allow reuse of local addresses
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket to the provided address
    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Start listening on the socket with maximum backlog of 1 pending connection
    if (listen(sock, backlog)) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return sock;
}

/**
 * Sets up a UDP server socket and binds it to the provided sockaddr_in address.
 *
 * @param addr The sockaddr_in structure representing the IP address and port of the server.
 *
 * @return The file descriptor of the created UDP server socket.
 */
int setup_datagram_socket(struct sockaddr_in addr){
    const int enable = 1;

	/*Create UDP socket*/
	// define socket
	int sock = socket(addr.sin_family, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == -1) {
    	perror("socket failed");
    	exit(EXIT_FAILURE);
}
    // Avoid dead lock on connections that are dropped after poll returns but before accept is called
    if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl");
		close(sock);
        exit(EXIT_FAILURE);
    }

    // Set the SO_REUSEADDR socket option to allow reuse of local addresses
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1) {
        perror("setsockopt");
		close(sock);
        exit(EXIT_FAILURE);
    }

	/*Bind the socket*/
	if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
		/*Error Handling if binding was unsuccessfull*/
        printf("Error binding UDP socket to Port %hu \n", addr.sin_port);
        close(sock);
        exit(EXIT_FAILURE);
    }
    // Debugging output
    fprintf(stderr, "UDP socket set up and bound to port %d\n", ntohs(addr.sin_port));
	return sock;
}


/**
 * Sets up the connection state for a new socket connection.
 *
 * @param state A pointer to the connection_state structure to be initialized.
 * @param sock The socket descriptor representing the new connection.
 *
 */
void connection_setup(struct connection_state* state, int sock) {
    // Set the socket descriptor for the new connection in the connection_state structure.
    state->sock = sock;

    // Set the 'end' pointer of the state to the beginning of the buffer.
    state->end = state->buffer;

    // Clear the buffer by filling it with zeros to avoid any stale data.
    memset(state->buffer, 0, HTTP_MAX_SIZE);
}
