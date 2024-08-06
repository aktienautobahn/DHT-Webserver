#include <stdbool.h>
#include <string.h>
#include "webserverMiddleware.h"
#include "webserver_parser.h"
#include "webserver_response.h"

/*  function for enforcement of write protection on resources other than dynamic*/
bool dynamicUriEnforcement(HttpRequest* request, HttpResponse* response) {
    // Check if the request method is PUT or DELETE
    if (request->method == PUT || request->method == DELETE) {
        // Check if the URI starts with "/dynamic" -> it should not!
        if (strncmp(request->uri, "/dynamic", strlen("/dynamic")) != 0) {
            response->status_code = HTTP_STATUS_FORBIDDEN;
            return false; // false = Stop further processing
        }
    }
    return true; // Continue processing
}

/* middlewares handler*/
bool runMiddlewares(HttpRequest* request, HttpResponse* response, Middleware* middlewares, int count) {
    for (int i = 0; i < count; i++) {
        if (!middlewares[i](request, response)) {
            return false; // Stop processing if middleware fails
        }
    }
    return true; // All middlewares passed
}