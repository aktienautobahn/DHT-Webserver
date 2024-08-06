#ifndef WEBSERVER_REQUEST_HANDLER_H
#define WEBSERVER_REQUEST_HANDLER_H

#include "webserver_parser.h"
#include "webserver_response.h"
#include "webserver_fileSystem.h"

// function declarations
void handleGetRequest(const HttpRequest* request, HttpResponse* response, int client_socket, file* filesystem);
void handlePutRequest(const HttpRequest* request, HttpResponse* response, int client_socket, file* filesystem);
void handleDeleteRequest(const HttpRequest* request, HttpResponse* response, int client_socket, file* filesystem);
void handlePostRequest(const HttpRequest* request, HttpResponse* response, int client_socket, file* filesystem);
void handleHeadRequest(const HttpRequest* request, HttpResponse* response, int client_socket, file* filesystem);
void handleBadRequest(HttpResponse* response, int client_socket); 
void handleNotImplementedMethodRequest(HttpResponse* response, int client_socket);

#endif // WEBSERVER_REQUEST_HANDLER_H