#ifndef WEBSERVERMIDDLEWARE_H
#define WEBSERVERMIDDLEWARE_H

#include <stdbool.h>
#include "webserver_parser.h"
#include "webserver_response.h"


// defining middleware type
typedef bool (*Middleware)(HttpRequest*, HttpResponse*);

// Middleware functions
bool dynamicUriEnforcement(HttpRequest* request, HttpResponse* response);
bool runMiddlewares(HttpRequest* request, HttpResponse* response, Middleware* middlewares, int count); 

#endif // WEBSERVERMIDDLEWARE_H