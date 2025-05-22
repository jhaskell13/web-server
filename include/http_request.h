#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#define MAX_HEADERS 20
#define MAX_HEADER_KEY 64
#define MAX_HEADER_VALUE 256

typedef struct {
    char method[8];
    char path[1024];
    char http_version[16];
    struct {
        char key[MAX_HEADER_KEY];
        char value[MAX_HEADER_VALUE];
    } headers[MAX_HEADERS];
    int header_count;
    const char *body;
    int body_length;
} HttpRequest;

int parse_http_request(const char *raw, HttpRequest *request);

#endif
