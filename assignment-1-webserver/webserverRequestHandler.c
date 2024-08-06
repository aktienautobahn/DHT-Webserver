#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "webserver_parser.h"
#include "webserver_response.h"
#include "webserver_fileSystem.h"
#include "webserver_logger.h"

#define BUFFER_SIZE 8192

char buffer[BUFFER_SIZE];

/* not implemented method handler*/
void handleNotImplementedMethodRequest(HttpResponse* response, int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    response->status_code = HTTP_STATUS_NOT_IMPLEMENTED;
    response_constructor(response, buffer, sizeof(buffer)); // constructs ASCII response line ready to send it out
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUFFER_SIZE);

}
/*GET Request handler*/
void handleGetRequest(const HttpRequest* request, HttpResponse* response, int client_socket, file* filesystem) {
    char* content = readFileContent(filesystem, request->uri);
    char buffer[BUFFER_SIZE] = {0};
    if (content != NULL) {
        response->status_code = HTTP_STATUS_OK;
        response->body = content;
        addContentLengthHeader(response);
    } else {
        response->status_code = HTTP_STATUS_NOT_FOUND;
        response->body = NULL;
        addContentLengthHeader(response);
    }
        response_constructor(response, buffer, sizeof(buffer));
        // logMessage("Response constructed: %s", buffer);

        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
}
/*PUT Request handler*/
void handlePutRequest(const HttpRequest* request, HttpResponse* response, int client_socket, file* filesystem) {
    char buffer[BUFFER_SIZE] = {0};
    char* content = readFileContent(filesystem, request->uri);
    // if file does not exist
    if (content == NULL) {
        //  create ressource, reply with Created (201)
        if (putFile(filesystem, request->uri, request->body) < 0) {
            response->status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
            response->body = NULL;
        } else {
            response->status_code = HTTP_STATUS_CREATED;
            response->body = NULL;
            addContentLengthHeader(response); 
        }
    } else {
        //  overwrite ressource, reply with No Content (204)
        if (putFile(filesystem, request->uri, request->body) < 0) {
            response->status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
            response->body = NULL;
        } else {
            response->status_code = HTTP_STATUS_NO_CONTENT;
            response->body = NULL;
            addContentLengthHeader(response);
        }
    }
        response_constructor(response, buffer, sizeof(buffer));
        // logMessage("sending response: %s", buffer);
        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);

}

/*DELETE Request handler*/
void handleDeleteRequest(const HttpRequest* request, HttpResponse* response, int client_socket, file* filesystem) {
    char buffer[BUFFER_SIZE] = {0};
    char* content = readFileContent(filesystem, request->uri);
    // if file does not exist
    if (content == NULL) {
        //  response Not found 
        response->status_code = HTTP_STATUS_NOT_FOUND;
        response->body = NULL;
        addContentLengthHeader(response); 
    
    } else {
        //  delete successful rssource, reply with No Content (204)
        if (deleteFile(filesystem, request->uri) < 0) {
            response->status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
            response->body = NULL;
        } else {
            response->status_code = HTTP_STATUS_NO_CONTENT;
            response->body = NULL;
            addContentLengthHeader(response);
        }
    }
        response_constructor(response, buffer, sizeof(buffer));
        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);

}

/* future-proof POST request handler*/
void handlePostRequest(const HttpRequest* request, HttpResponse* response, int client_socket, file* filesystem) {
    handleNotImplementedMethodRequest(response, client_socket);
}

/* future-proof HEAD request handler*/
void handleHeadRequest(const HttpRequest* request, HttpResponse* response, int client_socket, file* filesystem) {
    handleNotImplementedMethodRequest(response, client_socket);
}

/* Bad Request handler*/
void handleBadRequest(HttpResponse* response, int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    response->status_code = HTTP_STATUS_BAD_REQUEST;
    
    response_constructor(response, buffer, sizeof(buffer)); // constructs ASCII response line ready to send it out
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUFFER_SIZE);

}


