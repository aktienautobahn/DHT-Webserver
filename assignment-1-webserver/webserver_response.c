
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stddef.h>
#include "webserver_response.h"
#include "webserver_logger.h"
#define HTTP_DEFAULT_VERSION "HTTP/1.1"

void initHttpResponse(HttpResponse *response) {
    strcpy(response->version, HTTP_DEFAULT_VERSION); // Set default version
    response->status_code = 0;             // Default status code to 0
    response->headers = NULL;              // Default to no headers
    response->body = NULL;                 // Default to no body
}
void cleanup_http_response(HttpResponse *response) {
    if (response == NULL) {
        return; // Nothing to clean up
    }
    // Free the headers array
    if (response->headers) {
        response->headers = NULL; // Avoid dangling pointer
        free(response->headers);
    }
    // Free the body 
    if (response->body) {
        response->body = NULL; // Avoid dangling pointer
        free(response->body);
    }
    response->status_code = 0;// Default status code to 0

}
void addContentLengthHeader(HttpResponse *response) {
    int bodyLength = 0;
    if (response->body != NULL) {
    // handle the condition
        bodyLength = strlen(response->body);
    }




    // Estimate the length of the header string
    // "Content-Length: " is 16 characters, plus up to 10 characters for the length, plus 2 for "\r\n"
    int headerLength = 16 + 10 + 2;

    // Create a new header string
    char *newHeader = malloc(headerLength * sizeof(char));
    if (newHeader == NULL) {
        // Handle memory allocation error
        return;
    }

    // Format the Content-Length header
    snprintf(newHeader, headerLength, "Content-Length: %d\r\n", bodyLength);

    // Append this header to the existing headers
    if (response->headers == NULL) {
        // If there are no existing headers, just assign the new header
        response->headers = newHeader;
    } else {
        // If there are existing headers, append the new header
        int existingHeadersLength = strlen(response->headers);
        response->headers = realloc(response->headers, existingHeadersLength + headerLength);

        if (response->headers == NULL) {
            // Handle memory allocation error
            response->status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
            free(newHeader);
            return;
        }

        // Append the new header
        strcat(response->headers, newHeader);
        free(newHeader);
    }
}

const char* getStatusMessage(HttpStatusCode code) {
    int numStatuses = sizeof(httpStatusDescriptions) / sizeof(HttpStatusDescription);
    for (int i = 0; i < numStatuses; ++i) {
        if (httpStatusDescriptions[i].code == code) {
            return httpStatusDescriptions[i].message;
        }
    }
    return "Unknown Status"; // Default message for unknown status codes
}

void response_constructor(HttpResponse *httpResponse, char *buffer, size_t bufferSize) {
 // Check if headers are NULL or empty
    const char *headers = httpResponse->headers && httpResponse->headers[0] != '\0' ? httpResponse->headers : NULL;

    // Check if body is NULL or empty
    const char *body = httpResponse->body && httpResponse->body[0] != '\0' ? httpResponse->body : NULL;
    // logMessage("Response_constructer body: %s, headers: %s", httpResponse->body, httpResponse->headers);

    if (headers && body) {
        // Both headers and body are present
        snprintf(buffer, bufferSize, "%s %d %s\r\n%s\r\n%s",
            httpResponse->version, 
            httpResponse->status_code, 
            getStatusMessage(httpResponse->status_code), 
            headers, 
            body);
    } else if (headers && !body) {
        // Only headers are present
        snprintf(buffer, bufferSize, "%s %d %s\r\n%s\r\n",
            httpResponse->version, 
            httpResponse->status_code, 
            getStatusMessage(httpResponse->status_code), 
            headers);
    } else {
        // Neither headers nor body are present
        snprintf(buffer, bufferSize, "%s %d %s\r\n\r\n",
            httpResponse->version, 
            httpResponse->status_code, 
            getStatusMessage(httpResponse->status_code));
    }
}
