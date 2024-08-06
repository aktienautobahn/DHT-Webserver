#ifndef WEBSERVER_LOGGER_H
#define WEBSERVER_LOGGER_H


// logger functions
void initLogger(const char *logFilePath);
void logMessage(const char *format, ...);
void shutdownLogger();

#endif // WEBSERVER_LOGGER_H
