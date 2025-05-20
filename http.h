#include <stdio.h>

#ifndef HTTP_H
#define HTTP_H

void serve_file(FILE *fp, char *full_path, int client_socket, int keep_alive, char *buffer, char *protocol);
void serve_template(int client_socket, const char *full_path, char **keys, char **values, int count, int keep_alive);
void handle_client(int client_socket);

#endif
