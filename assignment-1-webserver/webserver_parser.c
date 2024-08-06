#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "webserver_parser.h"

// HTTP string to Enumerator converter
HttpMethod StringToHttpMethod(const char *method) {
    if (strcmp(method, "GET") == 0) return GET;
    else if (strcmp(method, "POST") == 0) return POST;
    else if (strcmp(method, "PUT") == 0) return PUT;
    else if (strcmp(method, "DELETE") == 0) return DELETE;
    else if (strcmp(method, "HEAD") == 0) return HEAD;
    else return UNKNOWN; // Return UNKNOWN for unsupported methods
}
// URI Validator functions
int _isValidHex(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f') ? 0: -1;
}

int _isValidPath(const char *path) {
    for (int i = 0; path[i] != '\0'; i++) {
        char c = path[i];

        if (isalnum(c) || strchr("./- _~!$&'()*+,;=:@/", c) != NULL) {
            continue;
        } else if (c == '%' && _isValidHex(path[i+1]) == 0 && _isValidHex(path[i+2]) == 0) {
            i += 2; // Skip the next two characters
        } else {
            return -1; // Invalid path
        }
    }
    return 0; // Valid path
}
// Length checking function
int _isLengthValid(size_t actualLength, size_t maxLength) {
    if (actualLength > maxLength) {
        return -1; // Length exceeds the maximum allowed
    }
    return 0; // Length is within the allowed range
}
/* ------------ PARSE HEADERS' LINES  ------------ */
HttpHeader *_parse_headers(const char *headers, int *header_count) {
    // Copy the headers to a mutable string
    char *headers_copy = strdup(headers);
    if (!headers_copy) return NULL;

    // Replace \\r\\n with \r\n to normalize line endings
    for (char *c = headers_copy; *c; c++) {
        if (strncmp(c, "\\r\\n", 4) == 0) {
            *c = '\r'; // Replace '\\' with '\r'
            *(c + 1) = '\n'; // Replace 'r' with '\n'
            memmove(c + 2, c + 4, strlen(c + 4) + 1); // Shift the rest of the string
        }
    }

    // Count headers
    int count = 0;
    for (char *c = headers_copy; *c; c++) {
        if (*c == '\n') count++;
    }
    if (strlen(headers_copy) > 0) { // Count the last header if it exists
        count++;
    }

    // Allocate an array of HttpHeader structures
    HttpHeader *header_array = calloc(count, sizeof(HttpHeader));
    if (!header_array) {
        free(headers_copy);
        return NULL;
    }

    // Tokenize the headers and parse them
    int i = 0;
    char *line = strtok(headers_copy, "\n");
    while (line != NULL && i < count) {
        // Trim leading whitespace
        while (*line == ' ' || *line == '\r') line++;

        char *colon = strchr(line, ':');
        if (colon != NULL) {
            *colon = '\0'; // Split the line at the colon
            strncpy(header_array[i].key, line, sizeof(header_array[i].key) - 1);
            strncpy(header_array[i].value, colon + 2, sizeof(header_array[i].value) - 1);
            i++;
        }
        line = strtok(NULL, "\n");
    }

    *header_count = i; // Set the actual number of headers parsed
    free(headers_copy);
    return header_array;
}


char *get_header_value(const HttpRequest *httpRequest, const char *key) {
    if (httpRequest == NULL || key == NULL) {
        return NULL;
    }

    for (int i = 0; i < httpRequest->header_count; i++) {
        if (strcmp(httpRequest->headers[i].key, key) == 0) {
            return httpRequest->headers[i].value;
        }
    }

    return NULL; // Key not found
}

void cleanup_http_request(HttpRequest *request) {
    if (request == NULL) {
        return; // Nothing to clean up
    }

    // Free the headers array
    if (request->headers) {
        free(request->headers);
        request->headers = NULL; // Avoid dangling pointer
    }

    request->header_count = 0;

}

/* ------------ REQUEST PARSE FUNCTION  ------------ */

void parse_request(const char *request, HttpRequest *httpRequest) {
    const char *end_of_line;
    const char *start_of_line = request;
    char *token;

/* ------------ PARSE REQUEST LINE (Method, URI, Version) ------------ */
    // PARSE METHOD
    token = memchr(start_of_line, ' ', strlen(start_of_line));
    if (token == NULL) {
        httpRequest->status = INVALID;
        httpRequest->status_code = HTTP_STATUS_BAD_REQUEST;
        return;
    }
    int len = token - start_of_line;
    char method[len + 1];
    strncpy(method, start_of_line, len);
    method[len] = '\0';
    httpRequest->method = StringToHttpMethod(method);
    if (httpRequest->method == UNKNOWN) {
        httpRequest->status = INVALID;
        httpRequest->status_code = HTTP_STATUS_BAD_REQUEST;
        return;
    }

    // PARSE URI
    start_of_line = token + 1;
    token = memchr(start_of_line, ' ', strlen(start_of_line));
    len = token - start_of_line;
    // length validator (lengh < HttpRequest->version)
    if (_isLengthValid(len, sizeof(httpRequest->uri) - 1) == -1) {
        httpRequest->status = INVALID;
        httpRequest->status_code = HTTP_STATUS_BAD_REQUEST;
        return; // or handle the error as needed
    }
    strncpy(httpRequest->uri, start_of_line, len);
    httpRequest->uri[len] = '\0';

    // Validate URI path for forbidden symbols
    if (_isValidPath(httpRequest->uri) == -1) {
        httpRequest->status = INVALID;
        httpRequest->status_code = HTTP_STATUS_BAD_REQUEST;
        return; // or handle the error as needed
    }

    // PARSE HTTP VERSION
    start_of_line = token + 1;
    end_of_line = strstr(start_of_line, "\r\n");
    // handling both \\r\\n and \r\n
    if (end_of_line == NULL) {
        end_of_line = strstr(start_of_line, "\\r\\n");
    }
    len = end_of_line - start_of_line;
    // length validator (lengh < HttpRequest->version)
    if (_isLengthValid(len, sizeof(httpRequest->version) - 1) == -1) {
        httpRequest->status = INVALID;
        httpRequest->status_code = HTTP_STATUS_BAD_REQUEST;
        return; // or handle the error as needed
    }
    // Proceed with copying the version
    printf("start of version : %s", start_of_line);
    strncpy(httpRequest->version, start_of_line, end_of_line - start_of_line);
    httpRequest->version[len] = '\0';
    printf("version is %s\n", httpRequest->version);

    // Validate HTTP version
    if (strncmp(httpRequest->version, "HTTP/", 5) != 0) {
        // Handle invalid HTTP version
        httpRequest->status = INVALID;
        httpRequest->status_code = HTTP_STATUS_BAD_REQUEST;
        return;
    }
/*-------------------- PARSE HEADERS -------------------- */

    // Headers start : move the pointer to the next line
    start_of_line = end_of_line + 2;
    // Find end of headers
    char *headers_end = strstr(start_of_line, "\r\n\r\n");
    // handling both \\r\\n\\r\\n and \r\n\r\n
    if (headers_end == NULL) {
        headers_end = strstr(start_of_line, "\\r\\n\\r\\n");
    }
    if (headers_end != NULL) {
    // Calculate the length of the headers section
    size_t headers_length = headers_end - start_of_line;

    // Create a substring for the headers
    char headers_substring[headers_length + 1];
    strncpy(headers_substring, start_of_line, headers_length);
    headers_substring[headers_length] = '\0';
    printf("Headers to parse: %s\n", headers_substring);
    // Parse headers
    HttpHeader *parsed_headers = _parse_headers(headers_substring, &httpRequest->header_count);
   // Check if parse_headers was successful
    if (parsed_headers != NULL && (parsed_headers[0].key[0] != '\0' || parsed_headers[0].value[0] != '\0')) {
        // Array is not empty
        httpRequest->headers = parsed_headers;

    } else {
        // Array is empty or headers is NULL
        httpRequest->status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
        return;
    }
 /* -------------------- PARSE BODY --------------------*/
 // Check for the presence of a body
    if (*(headers_end + 4) != '\0') {
        // Find Content-Length header (assuming you have a function to do this)
        char *content_length_str = get_header_value(httpRequest, "Content-Length");
        if (content_length_str != NULL) {
            int content_length = atoi(content_length_str);

            // Check if content_length is within expected bounds
            if (content_length > 0 && content_length < MAX_BODY_SIZE) {
                // Copy the body content
                const char *body_start = headers_end + 4;
                strncpy(httpRequest->body, body_start, content_length);
                httpRequest->body[content_length] = '\0';
            } else {
                // Invalid content length or exceeds buffer size
                httpRequest->status = INVALID;
                httpRequest->status_code = HTTP_STATUS_BAD_REQUEST;
                return;
            }
        } else {
            // No Content-Length header, but body was expected
            httpRequest->status = INVALID;
            httpRequest->status_code = HTTP_STATUS_BAD_REQUEST;
            return;
        }
    } else {
        // No body present
        httpRequest->body[0] = '\0';
    }
    }
}
