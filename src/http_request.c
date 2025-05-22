#include "../include/http_request.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>

int parse_http_request(const char *raw, HttpRequest *request) {
    char *line_end = strstr(raw, "\r\n");
    if (!line_end) return -1;

    // Parse request line
    sscanf(raw, "%7s %1023s %15s", request->method, request->path, request->http_version);

    // Parse headers
    const char *headers_start = line_end + 2;
    const char *line = headers_start;
    int h_count = 0;

    while ((line_end = strstr(line, "\r\n")) && line != line_end && h_count < MAX_HEADERS) {
        char key[MAX_HEADER_KEY], value[MAX_HEADER_VALUE];
        int scanned = sscanf(line, "%63[^:]: %255[^\r\n]", key, value);
        if (scanned == 2) {
            strncpy(request->headers[h_count].key, key, MAX_HEADER_KEY);
            request->headers[h_count].key[MAX_HEADER_KEY - 1] = '\0';
            strncpy(request->headers[h_count].value, value, MAX_HEADER_VALUE);
            request->headers[h_count].value[MAX_HEADER_VALUE - 1] = '\0';
            h_count++;
        }
        line = line_end + 2;
    }
    request->header_count = h_count;

    // Parse body
    const char *body_start = strstr(raw, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        request->body = body_start;

        const char *cl = NULL;
        for (int i = 0; i < request->header_count; i++) {
            if (strcasecmp(request->headers[i].key, "Content-Length") == 0) {
                cl = request->headers[i].value;
                break;
            }
        }
        request->body_length = cl ? atoi(cl) : strlen(body_start);
    }

    return 0;
}
