#ifndef WEBSERVER_RESPONSE_H
#define WEBSERVER_RESPONSE_H

#include "webserver_parser.h"
#include <stddef.h> // For size_t
#define BACKLOG 10	 // how many pending connections queue will hold
#define MAX_URI_SIZE 2048
#define MAX_HTTP_VERSION_SIZE 10
#define MAX_HEADER_SIZE 4096
#define MAX_BODY_SIZE 8192


typedef struct {
    HttpStatusCode code;
    const char *message;
} HttpStatusDescription;

static const HttpStatusDescription httpStatusDescriptions[] = {
    {HTTP_STATUS_OK, "OK"},
    {HTTP_STATUS_CREATED, "Created"},
    {HTTP_STATUS_ACCEPTED, "Accepted"},
    {HTTP_STATUS_NO_CONTENT, "No Content"},
    {HTTP_STATUS_MOVED_PERMANENTLY, "Moved Permanently"},
    {HTTP_STATUS_FOUND, "Found"},
    {HTTP_STATUS_SEE_OTHER, "See Other"},
    {HTTP_STATUS_NOT_MODIFIED, "Not Modified"},
    {HTTP_STATUS_TEMPORARY_REDIRECT, "Temporary Redirect"},
    {HTTP_STATUS_PERMANENT_REDIRECT, "Permanent Redirect"},
    {HTTP_STATUS_BAD_REQUEST, "Bad Request"},
    {HTTP_STATUS_UNAUTHORIZED, "Unauthorized"},
    {HTTP_STATUS_FORBIDDEN, "Forbidden"},
    {HTTP_STATUS_NOT_FOUND, "Not Found"},
    {HTTP_STATUS_METHOD_NOT_ALLOWED, "Method Not Allowed"},
    {HTTP_STATUS_NOT_ACCEPTABLE, "Not Acceptable"},
    {HTTP_STATUS_REQUEST_TIMEOUT, "Request Timeout"},
    {HTTP_STATUS_CONFLICT, "Conflict"},
    {HTTP_STATUS_GONE, "Gone"},
    {HTTP_STATUS_LENGTH_REQUIRED, "Length Required"},
    {HTTP_STATUS_PRECONDITION_FAILED, "Precondition Failed"},
    {HTTP_STATUS_PAYLOAD_TOO_LARGE, "Payload Too Large"},
    {HTTP_STATUS_URI_TOO_LONG, "URI Too Long"},
    {HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
    {HTTP_STATUS_RANGE_NOT_SATISFIABLE, "Range Not Satisfiable"},
    {HTTP_STATUS_EXPECTATION_FAILED, "Expectation Failed"},
    {HTTP_STATUS_TOO_MANY_REQUESTS, "Too Many Requests"},
    {HTTP_STATUS_INTERNAL_SERVER_ERROR, "Internal Server Error"},
    {HTTP_STATUS_NOT_IMPLEMENTED, "Not Implemented"},
    {HTTP_STATUS_BAD_GATEWAY, "Bad Gateway"},
    {HTTP_STATUS_SERVICE_UNAVAILABLE, "Service Unavailable"},
    {HTTP_STATUS_GATEWAY_TIMEOUT, "Gateway Timeout"},
    {HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"}
};


// data structure for Http Responses
typedef struct {
    char version[MAX_HTTP_VERSION_SIZE];   // "HTTP/1.1" including null terminator
    HttpStatusCode status_code;     
    char *headers;     // Dynamically allocated string for all headers
    char *body;        // Dynamically allocated string for the body
} HttpResponse;

// declaration of functions
void initHttpResponse(HttpResponse *response);
void addContentLengthHeader(HttpResponse *response);
const char* getStatusMessage(HttpStatusCode code);
void response_constructor(HttpResponse *httpResponse, char *buffer, size_t bufferSize);
void cleanup_http_response(HttpResponse *response);

#endif // WEBSERVER_RESPONSE_H