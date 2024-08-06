#ifndef WEBSERVER_PARSER_H
#define WEBSERVER_PARSER_H

#include <stddef.h> // For size_t
#define BACKLOG 10	 // how many pending connections queue will hold
#define MAX_URI_SIZE 2048
#define MAX_HTTP_VERSION_SIZE 10
#define MAX_HEADER_SIZE 4096
#define MAX_BODY_SIZE 8192

// own key-value pair structure
typedef struct {
    char key[256];
    char value[1024];
} HttpHeader;

// http methods as enum
typedef enum {
    UNKNOWN,
    GET,
    POST,
    PUT,
    DELETE,
    HEAD,
} HttpMethod;

//http status valid/invalid
typedef enum {
    VALID,
    INVALID
} ParsingStatus;

// HTTP status codes
typedef enum {
    HTTP_STATUS_OK = 200,
    HTTP_STATUS_CREATED = 201,
    HTTP_STATUS_ACCEPTED = 202,
    HTTP_STATUS_NO_CONTENT = 204,
    HTTP_STATUS_MOVED_PERMANENTLY = 301,
    HTTP_STATUS_FOUND = 302,
    HTTP_STATUS_SEE_OTHER = 303,
    HTTP_STATUS_NOT_MODIFIED = 304,
    HTTP_STATUS_TEMPORARY_REDIRECT = 307,
    HTTP_STATUS_PERMANENT_REDIRECT = 308,
    HTTP_STATUS_BAD_REQUEST = 400,
    HTTP_STATUS_UNAUTHORIZED = 401,
    HTTP_STATUS_FORBIDDEN = 403,
    HTTP_STATUS_NOT_FOUND = 404,
    HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
    HTTP_STATUS_NOT_ACCEPTABLE = 406,
    HTTP_STATUS_REQUEST_TIMEOUT = 408,
    HTTP_STATUS_CONFLICT = 409,
    HTTP_STATUS_GONE = 410,
    HTTP_STATUS_LENGTH_REQUIRED = 411,
    HTTP_STATUS_PRECONDITION_FAILED = 412,
    HTTP_STATUS_PAYLOAD_TOO_LARGE = 413,
    HTTP_STATUS_URI_TOO_LONG = 414,
    HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE = 415,
    HTTP_STATUS_RANGE_NOT_SATISFIABLE = 416,
    HTTP_STATUS_EXPECTATION_FAILED = 417,
    HTTP_STATUS_TOO_MANY_REQUESTS = 429,
    HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
    HTTP_STATUS_NOT_IMPLEMENTED = 501,
    HTTP_STATUS_BAD_GATEWAY = 502,
    HTTP_STATUS_SERVICE_UNAVAILABLE = 503,
    HTTP_STATUS_GATEWAY_TIMEOUT = 504,
    HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED = 505
} HttpStatusCode;

// data structure for Http Requests
typedef struct HttpRequest {
    ParsingStatus status;
    HttpStatusCode status_code; 
    HttpMethod method;
    char uri[MAX_URI_SIZE];
    char version[MAX_HTTP_VERSION_SIZE];
    HttpHeader *headers; // Dynamic array of headers
    int header_count;    // Number of headers
    char body[MAX_BODY_SIZE];
} HttpRequest;



// Function declarations
HttpMethod StringToHttpMethod(const char *method);
HttpHeader *_parse_headers(const char *headers, int *header_count);
char *get_header_value(const HttpRequest *httpRequest, const char *key);
void cleanup_http_request(HttpRequest *request);
void parse_request(const char *request, HttpRequest *httpRequest); 

#endif // WEBSERVER_PARSER_H