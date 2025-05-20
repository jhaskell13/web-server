#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "http.h"

#define MAX_QUEUE 64

static int queue[MAX_QUEUE];
static int front = 0, rear = 0;
static pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

static int thread_count = 4;
static pthread_t *threads;

static int queue_size() {
    return (rear + MAX_QUEUE - front) % MAX_QUEUE;
}

static int queue_full() {
    return queue_size() == MAX_QUEUE - 1;
}

static void enqueue(int client_socket) {
    queue[rear] = client_socket;
    rear = (rear + 1) % MAX_QUEUE;
}

static int dequeue() {
    int client_socket = queue[front];
    front = (front + 1) % MAX_QUEUE;
    return client_socket;
}

static void *worker_loop(void *arg) {
    while (1) {
        pthread_mutex_lock(&queue_lock);
        while (queue_size() == 0) {
            pthread_cond_wait(&queue_not_empty, &queue_lock);
        }

        int client_socket = dequeue();
        pthread_mutex_unlock(&queue_lock);

        handle_client(client_socket);
    }
    return NULL;
}

void threadpool_init(int max_threads) {
    thread_count = max_threads;
    threads = malloc(sizeof(pthread_t) * thread_count);
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], NULL, worker_loop, NULL);
        pthread_detach(threads[i]);  // No need to join
    }
}

void threadpool_add_client(int client_socket) {
    pthread_mutex_lock(&queue_lock);

    if (queue_full()) {
        fprintf(stderr, "Queue full. Rejecting client.\n");
        close(client_socket);
    } else {
        enqueue(client_socket);
        pthread_cond_signal(&queue_not_empty);
    }

    pthread_mutex_unlock(&queue_lock);
}

void threadpool_destroy() {
    // Not implemented — you'd signal all threads to exit, then join them.
    free(threads);
}

