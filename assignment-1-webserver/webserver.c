/*
** webserver.c -- a stream socket server
*/
// system libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
// custom utils
#include "webserverMiddleware.h"
#include "webserver_parser.h"
// #include "webserver_fs_emu.h"
#include "webserver_response.h"
#include "webserverRequestHandler.h"
#include "webserver_fileSystem.h"
#include "webserver_logger.h"

// func to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


/* - - - - - - - - - - - - - - - - - - - - MAIN - - - - - - - - - - - - - - - - - - - -  */
int main(int argc, const char *argv[]) {

if (argc < 3){
    printf("Aufruf: %s IP-Adresse Port\n", argv[0]);
    printf("Beispiel: %s localhost 4711\n", argv[0]);
    exit(1);
}
/*Passed parameters*/
const char* IPAddress = argv[1];
const char* PortID = argv[2];
void perror(const char *s);

/* -------------------- MIDDLEWARE DECLARATION -------------------- */
Middleware middlewares[] = {
    dynamicUriEnforcement // enforcement of an operation for /dynamic
};
int middlewareCount = sizeof(middlewares) / sizeof(middlewares[0]);
/* -------------------- END OF MIDDLEWARE DECLARATION -------------------- */


/* -------------------- METHODS REQUEST DECLARATION -------------------- */
/*handler hanlder struct*/
typedef void (*RequestHandler)(const HttpRequest*, HttpResponse*, int, file*);
/*method to handler mapping struct*/
typedef struct {
    HttpMethod method;
    RequestHandler handler;
} MethodHandlerMapping;
/*mapping of methods to handlers*/
MethodHandlerMapping methodHandlers[] = {
    { GET, handleGetRequest },
    { POST, handlePostRequest },
    { PUT, handlePutRequest },
    { DELETE, handleDeleteRequest },
    { HEAD, handleHeadRequest }
};
/* -------------------- END OF METHODS REQUEST DECLARATION -------------------- */

/* -------------------- LOGGER INIT -------------------- */

// initLogger("logfile.txt");

/* -------------------- END OF LOGGER INIT -------------------- */
/* -------------------- CREATE RESSOURCES -------------------- */

file* FS = makeDynamicFolder();
printf("Resources created succesfully: \n");

/* -------------------- CREATE SOCKET -------------------- */
//declare the server socket
int server_socket;

struct addrinfo hints, *res;
//clear struct
memset(&hints, 0, sizeof hints); // make sure the struct is empty
// declaration of socket specific variables/structs
hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
hints.ai_socktype = SOCK_STREAM; /*  socket */
hints.ai_flags = 0;
hints.ai_protocol = 0;          /* Any protocol */
struct sockaddr_storage their_addr; // connector's address information
socklen_t sin_size;
sin_size = sizeof their_addr;
char s[INET6_ADDRSTRLEN];
getaddrinfo(IPAddress, PortID, &hints, &res);

// define socket
server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// configure socket
int optval = 1;
if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == -1) {
    perror("setsockopt");
    exit(1);
}
// bind socket
bind(server_socket, res->ai_addr, res->ai_addrlen);
// listen at socket
listen(server_socket, BACKLOG);

printf("Server: waiting for connections on %s:%s...\n", IPAddress, PortID);

char buffer[8192];// define buffer 
int client_socket;// declare client socket
int running = 1;
while(running)
{
    // accept client requests (creating client specific socket)
	client_socket = accept(server_socket, (struct sockaddr *)&their_addr, &sin_size);
    // error handling
    if (client_socket == -1) {
        perror("accept");
        continue; 
        }

    if (inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s) == NULL) {
        perror("inet_ntop");
        continue; 
    }
    printf("server: got connection from %s\n", s);

    int keep_alive = 1;

    /* connection established while loop*/
    int content_length;
    while(keep_alive) {
            // read buffer
            int bytesReceived = recv(client_socket, buffer, sizeof buffer, 0);
            // error handling
            if (bytesReceived <= 0) {
                // Error or connection closed by client
                break;
            }

            // check if no more packets are being sent
            if (strncmp(buffer, "\r\n\r\n", bytesReceived)) {

                buffer[bytesReceived] = '\0';
                char *headers_end = strstr(buffer, "\r\n\r\n");
               

                /* -------------------- PARSE --------------------*/
                HttpRequest request; // struct declaration
                memset(&request, 0, sizeof(request));
                parse_request(buffer, &request); //custom parse request function in webserver_parser.h

                if (request.method != GET && request.method != DELETE) {
                char* content_length_str = get_header_value(&request, "Content-Length");
                if (content_length_str != NULL) {
                    content_length = atoi(content_length_str);
                }

                if (content_length > 0 && !strcmp(request.body, "")) {
                    int totalBytesReceived = bytesReceived;
                    bool packet_received = false;
                    while (!packet_received) {
                        totalBytesReceived = totalBytesReceived + recv(client_socket, buffer + totalBytesReceived, sizeof(buffer)-totalBytesReceived-1, 0);
                        
                        // content
                        if (totalBytesReceived >= content_length) {
                            packet_received = true;
                        }
                    }

                    memset(&request, 0, sizeof(request));

                    parse_request(buffer, &request); //custom parse request function in webserver_parser.h

                }
                }


                /* -------------------- RESPONSE CONSTRUCTOR --------------------*/
                HttpResponse response;
                initHttpResponse(&response); // initializes with

                /* -------------------- REQUEST PROCESSOR (HANDLER) --------------------*/

                if (runMiddlewares(&request, &response, middlewares, middlewareCount)) {
                    int i = 0;
                    bool methodHandled = false;
                    int numHandlers = sizeof(methodHandlers) / sizeof(methodHandlers[0]);
                    if (request.status == VALID) {
                        // loops in handlers list , maps method to the handler
                    while (i < numHandlers && !methodHandled) {
                        if (methodHandlers[i].method == request.method) {
                            methodHandlers[i].handler(&request, &response, client_socket, FS);
                            methodHandled = true;
                        }
                        i++;
                    }
                        if (!methodHandled) {
                            if (request.method == UNKNOWN) {
                                handleBadRequest(&response, client_socket);
                            } else {
                                handleNotImplementedMethodRequest(&response, client_socket);
                            }
                        }
                    } else {
                    handleBadRequest(&response, client_socket); 
                    }
                }
                else // if middleware fails
                {
                    char responseString[80];
                    response_constructor(&response, responseString, sizeof(responseString)); // constructs ASCII response line ready to send it out
                    send(client_socket, responseString, sizeof(responseString), 0); 
                }
                // clean each request/response after if was successfully processed
                cleanup_http_request(&request);
                cleanup_http_response(&response);
            }
        }
        close(client_socket); // close client socket
}
close(server_socket); // close server socket
deleteDynamicFolder(FS); // empty memory emulation folder

shutdownLogger(); // closing logger file
return 0;
}
