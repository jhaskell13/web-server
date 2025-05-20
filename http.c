#include "http.h"
#include "mime.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <dirent.h>

#define BUFFER_SIZE 2048
#define MAX_PATH_SIZE 1024

void serve_file(FILE *fp, char *full_path, int client_socket, int keep_alive, char *buffer, char *protocol) {
    // Get file size and read into buffer fb
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    char *fb = malloc(file_size);
    fread(fb, 1, file_size, fp);
    fclose(fp);

    // Construct HTTP Response. Send header then file content
    keep_alive = 1;
    if (strstr(buffer, "Connection: close") || strstr(protocol, "HTTP/1.0")) {
        keep_alive = 0;
    }

    char header[512];
    const char *mime_type = get_mime_type(full_path);
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: %s\r\n"
        "\r\n",
        mime_type, file_size, keep_alive ? "keep-alive" : "close");

    send(client_socket, header, strlen(header), 0);
    send(client_socket, fb, file_size, 0);
    free(fb);
}

void serve_template(int client_socket, const char *full_path, char **keys, char **values, int count, int keep_alive) {
    FILE *fp = fopen(full_path, "rb");
    if (!fp) {
        const char *not_found = "HTTP/1.1 404 Not Found\r\n\r\n404 Not Found";
        send(client_socket, not_found, strlen(not_found), 0);
        return;
    }
    // Get file size and read into buffer fb
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    char *fb = malloc(file_size + 1);
    fread(fb, 1, file_size, fp);
    fclose(fp);

    // Replace {{key}} with values
    char *output = malloc(4*BUFFER_SIZE);
    output[0] = '\0';

    char *p = fb;
    while (*p) {
        if (strncmp(p, "{{", 2) == 0) {
            char key[64];
            int i = 0;
            p += 2;

            // Loop until we detect "}}" marking the end of interpolation
            while (*p && !(*p == '}' && *(p+1) == '}') && i < sizeof(key) - 1) {
                key[i++] = *p++;
            }
            key[i] = '\0';
            if (*p == '}' && *(p+1) == '}') p += 2;

            // Match key
            const char *value = "";
            for (int j = 0; j < count; ++j) {
                if (strcmp(key, keys[j]) == 0) {
                    value = values[j];
                    break;
                }
            }
            strncat(output, value, (4*BUFFER_SIZE - 1) - strlen(output));
        } else {
            char tmp[2] = {*p++, '\0'};
            strncat(output, tmp, (4*BUFFER_SIZE - 1) - strlen(output));
        }
    }

    char header[256];
    snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %lu\r\n"
            "Connection: %s\r\n"
            "\r\n",
            strlen(output), keep_alive ? "keep-alive" : "close");

    send(client_socket, header, strlen(header), 0);
    send(client_socket, output, strlen(output), 0);
    free(fb);
    free(output);
}

void handle_client(int client_socket) {
    printf("Client connected.\n");

    char buffer[BUFFER_SIZE];
    int keep_alive = 1;

    // 5 second read timeout
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (keep_alive) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) break;

        printf("Received:\n%s\n", buffer);

        // Get the first line (e.g., GET /hello HTTP 1.1)
        char method[8], path[MAX_PATH_SIZE], protocol[16];

        // Parse method, path, and protocol
        if (sscanf(buffer, "%7s %1023s %15s", method, path, protocol) != 3) {
            const char *bad_request = "HTTP/1.1 400 Bad Request\r\n\r\n";
            send(client_socket, bad_request, strlen(bad_request), 0);
            break;
        }

        // Only allow GET
        if (strcmp(method, "GET") != 0) {
            const char *not_allowed = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
            send(client_socket, not_allowed, strlen(not_allowed), 0);
            break;
        }

        // Prevent backwards traversal
        if (strstr(path, "..") != NULL) {
            const char *forbidden = "HTTP/1.1 403 Forbidden\r\n\r\n403 Forbidden";
            send(client_socket, forbidden, strlen(forbidden), 0);
            break;
        }

        // DYNAMIC ROUTING
        if (strcmp(path, "/") == 0) {
            serve_template(client_socket, "templates/index.html",
                    (char*[]){"title", "content"},
                    (char*[]){"Home Page", "Welcome to the home page!"},
                    2, keep_alive);
            continue;
        }

        // STATIC FILE SERVING
        // Build static file path: ./static + path
        char full_path[MAX_PATH_SIZE];

        // Map requests for /assets and /static
        if (strncmp(path, "/assets/", 8) == 0) {
            snprintf(full_path, sizeof(full_path), "./assets/%.*s", MAX_PATH_SIZE - 10, path + 8);
        } else {
            snprintf(full_path, sizeof(full_path), "./static%.*s", MAX_PATH_SIZE - 9, path);
        }

        // Check if path is to a directory before trying to open file
        struct stat path_stat;
        if (stat(full_path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
            // Append trailing slash if not present
            if (full_path[strlen(full_path) - 1] != '/') {
                strcat(full_path, "/");
            }

            // Look for index.html in directory
            char index_path[BUFFER_SIZE];
            snprintf(index_path, sizeof(index_path), "%sindex.html", full_path);
            FILE *index_file = fopen(index_path, "rb");
            if (index_file) {
                serve_file(index_file, index_path, client_socket, keep_alive, buffer, protocol);
                continue;
            }

            // No index.html, generate directory listing
            DIR *dir = opendir(full_path);
            if (!dir) {
                const char *error = "HTTP/1.1 403 Forbidden\r\n\r\n403 Forbidden";
                send(client_socket, error, strlen(error), 0);
                continue;
            }

            char response[4*BUFFER_SIZE];
            snprintf(response, sizeof(response),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Connection: %s\r\n"
                "\r\n"
                "<html><body><h1>Index of %s</h1><ul>",
                keep_alive ? "keep-alive" : "close",
                path);

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0) continue;
                size_t line_size = strlen(path) + strlen(entry->d_name)*2 + 64;
                char *line = malloc(line_size);
                snprintf(line, line_size,
                        "<li><a href=\"%s%s%s\">%s</a></li>",
                        path,
                        (path[strlen(path) - 1] == '/' ? "" : "/"),
                        entry->d_name,
                        entry->d_name);
                strncat(response, line, sizeof(response) - strlen(response) - 1);
                free(line); 
            }

            closedir(dir);
            strncat(response, "</ul></body></html>", sizeof(response) - strlen(response) - 1);
            send(client_socket, response, strlen(response), 0);
            continue;

        }

        // Not a directory, try to open file 
        FILE *fp = fopen(full_path, "rb");
        if (!fp) {
            const char *not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(client_socket, not_found, strlen(not_found), 0);
            return;
        }

        serve_file(fp, full_path, client_socket, keep_alive, buffer, protocol);
    }
    close(client_socket);
}
