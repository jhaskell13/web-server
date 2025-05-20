#ifndef THREADPOOL_H
#define THREADPOOL_H

void threadpool_init(int max_threads);
void threadpool_add_client(int client_fd);
void threadpool_destroy();

#endif

