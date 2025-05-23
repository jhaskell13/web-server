#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "../include/http_response.h"

#define MAX_RESPONSE_SIZE 8192

const char *http_response_status(HttpStatus status) {
    switch (status) {
        case OK:
            return "200 OK";
        case BAD_REQUEST:
            return "400 Bad Request";
        case FORBIDDEN:
            return "403 Forbidden";
        case NOT_FOUND:
            return "404 Not Found";
        case INT_SERVER_ERR:
            return "500 Internal Server Error";
        default:
            return "500 Unknown Error";
    }
}

void send_response(HttpResponse *response, int client_socket) {
    char buffer[MAX_RESPONSE_SIZE];
    snprintf(buffer, sizeof(buffer),
            "HTTP/1.1 %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %d\r\n"
            "Connection: %s\r\n"
            "\r\n"
            "%s",
            response->status, response->ct, response->cl, response->con, response->body);
    send(client_socket, buffer, strlen(buffer), 0);
}
