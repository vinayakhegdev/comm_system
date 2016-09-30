#ifndef _COMM_TCP_CLIENT_H
#define _COMM_TCP_CLIENT_H
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
#include <sys/un.h>
#include "log_macros.h"
#include <assert.h>

int
comm_tcp_client_init(comm_client_t *comm_client,
                             comm_client_info_t *comm);
ssize_t
comm_tcp_client_send(void *arg1, void *arg2);
ssize_t
comm_tcp_client_send_header(void *arg1, void *arg2);
ssize_t
comm_tcp_client_send_data(void *arg1, void *arg2);
ssize_t
comm_tcp_servresp_send(void *arg1, void *arg2);
ssize_t 
comm_tcp_client_wait(void *arg1, void *arg2);
void
comm_tcp_client_finish(void *arg1);
#endif
