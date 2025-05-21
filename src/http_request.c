#include "../include/http_request.h"
#include <string.h>
#include <stdio.h>

int parse_http_request(const char *raw, HttpRequest *request) {
    char *line_end = strstr(raw, "\r\n");
    if (!line_end) return -1;

    // Parse request line
    sscanf(raw, "%7s %1023s %15s", request->method, request->path, request->http_version);

    const char *headers_start = line_end + 2;
    const char *line = headers_start;
    int count = 0;

    while ((line_end = strstr(line, "\r\n")) && line != line_end && count < MAX_HEADERS) {
        char key[MAX_HEADER_KEY], value[MAX_HEDER_VALUE];
        int scanned = sscanf(line, "%63[^:]: %255[^\r\n]", key, value);
        if (scanned == 2) {
            strncpy(request->headers[count].key, key, MAX_HEADER_KEY);
            strncpy(request->headers[count].value, value, MAX_HEDER_VALUE);
            count++;
        }
        line = line_end + 2;
    }

    request->header_count = count;
    return 0;
}
