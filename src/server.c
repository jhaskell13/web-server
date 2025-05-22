// server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "../include/http.h"
#include "../include/threadpool.h"

#define SERVER_PORT 8080

pthread_mutex_t thread_count_lock = PTHREAD_MUTEX_INITIALIZER;
int active_threads = 0;

void *client_thread(void *arg) {
    pthread_mutex_lock(&thread_count_lock);
    active_threads++;
    printf("Active threads: %d\n", active_threads);
    pthread_mutex_unlock(&thread_count_lock);

    int client_socket = *(int *)arg;
    free(arg); // malloc'ed in main()
    handle_client(client_socket);

    pthread_mutex_lock(&thread_count_lock);
    active_threads--;
    printf("Active threads: %d\n", active_threads);
    pthread_mutex_unlock(&thread_count_lock);
    return NULL;
}

int main() {
    threadpool_init(8); // Stat 8 worker threads

    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Bind server address (IPv4, any network interface, port 8080)
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }

    // Listen
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }

    printf("Server listening on port %d...\n", SERVER_PORT);

    while (1) {
        // Accept a connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (*client_socket < 0) {
            perror("Accept failed");
            close(*client_socket);
            free(client_socket);
            continue;
        }
        threadpool_add_client(*client_socket);
    }

    // Clean up
    threadpool_destroy();
    close(server_socket);

    printf("Server shut down.\n");
    return 0;
}
