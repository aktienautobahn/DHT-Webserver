#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#include "webserver_logger.h"

FILE *logFile = NULL;

// Function to initialize the logger
void initLogger(const char *logFilePath) {
    logFile = fopen(logFilePath, "a");  // Open for appending
    if (logFile == NULL) {
        perror("Failed to open log file");
    }
}

// Function to log a message
void logMessage(const char *format, ...) {
    if (logFile == NULL) {
        return;
    }

    // Get the current time
    time_t now;
    time(&now);
    char *timeStr = ctime(&now);
    timeStr[strlen(timeStr) - 1] = '\0';  // Remove the newline at the end

    // Write the time to the log file
    fprintf(logFile, "[%s] ", timeStr);

    // Write the user's message to the log file
    va_list args;
    va_start(args, format);
    vfprintf(logFile, format, args);
    va_end(args);

    fprintf(logFile, "\n");  // New line for next log entry

    fflush(logFile);  // Flush the stream
}

// Function to shutdown the logger
void shutdownLogger() {
    if (logFile != NULL) {
        fclose(logFile);
        logFile = NULL;
    }
}


