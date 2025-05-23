#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

typedef struct {
    const char *status;
    const char *ct;
    int cl;
    char *con;
    char *body;
} HttpResponse;

typedef enum {
    OK,
    BAD_REQUEST,
    FORBIDDEN,
    NOT_FOUND,
    INT_SERVER_ERR,
    SIZE
} HttpStatus;

const char *http_response_status(HttpStatus status);

void send_response(HttpResponse *response, int client_socket);

#endif
