#ifndef _COMM_WORK_QUEUE
#define _COMM_WORK_QUEUE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <pthread.h>

typedef struct comm_queue_entry__ {
    int socket_fd;
    struct comm_queue_entry__ *next;
} comm_queue_entry_t;

typedef struct comm_queue__ {
    comm_queue_entry_t *queue_in;
    comm_queue_entry_t *queue_out;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} comm_queue_t;

comm_queue_entry_t * 
comm_queue_get(comm_queue_t *root);
comm_queue_t *
comm_init_queueroot();

void 
comm_queue_insert(comm_queue_entry_t *entry,
                      comm_queue_t *root);

#endif
