#ifndef _COMM_I_H
#define _COMM_I_H
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
#include "comm_work_queue.h"
#include <netinet/in.h>
#include <netinet/tcp.h>

#define MAXEVENTS 64
#define EPOLL_WAIT_TIMEOUT 100

#define COMM_STATUS_GENERIC -1
#define COMM_STATUS_PEER_CLOSE -2
#define COMM_TCP_MAX_IP_LEN 25

typedef enum comm_type__ {
    COMM_TCP = 0,
    COMM_UNIX = 1,
} comm_type_t;

typedef enum comm_msg_type__ {
    COMM_DM_MSG_SNAP = 0,
    COMM_DM_MSG_START = 1,
    COMM_FU_MSG_START = 2,
    COMM_ORCH_STATS_MSG_START = 3,
    COMM_ORCH_STATS_MSG_END = 4,
    COMM_ORCH_STATS_MSG_GET = 5,
    COMM_DUMP_STATS_MSG = 6,
    COMM_SFP_GETATTR = 7,
    COMM_SFP_SETATTR = 8,
    COMM_SFP_READ = 9,  
    COMM_SFP_WRITE = 10,
    COMM_SFP_UNLINK = 11,
    COMM_SFP_COMPOUND_GETATTR = 12,
} comm_msg_type_t;

typedef struct comm_client__ {
    union {
        struct {
            int socket_fd;
        } comm_unix_client;      
        struct {
            int socket_fd;
        } comm_tcp_client;      
    } comm_client_type;
    comm_type_t ipc_type;

    ssize_t (*comm_client_send) (void *,
                                         void *);
    ssize_t (*comm_client_send_header) (void *,
                                                void *);
    ssize_t (*comm_client_send_data) (void *,
                                              void *);
    ssize_t (*comm_client_wait) (void *,
                                         void *);  
    void (*comm_client_finish)(void *);
} comm_client_t;

typedef struct comm_server__ {
    union {
        struct {
            int socket_fd;
            int worker_index;
        } comm_unix_server;
        struct {
            int socket_fd;
            int worker_index;
        } comm_tcp_server;
    } comm_server_type; 
    comm_type_t ipc_type;
    ssize_t (*comm_servresp_send) (void *,
                                           void *);

    ssize_t (*comm_servresp_send_header) (void *,
                                                  void *);
    ssize_t (*comm_servresp_send_data) (void *,
                                                void *);
} comm_server_t;

typedef struct comm_msg_header__ {
    comm_msg_type_t msg_type;
    uint64_t msg_len;
    uint64_t msg_write_len;
} comm_msg_header_t;

typedef struct comm_msg__ {
    comm_msg_header_t msg_header;
    comm_server_t comm_server;
    comm_client_t comm_client;
    char *data;
} comm_msg_t;

typedef struct comm_err_msg__ {
    comm_server_t comm_server;
    comm_client_t comm_client;
} comm_err_msg_t;

typedef struct comm_serv_info__ {
    union {
        struct {
            int sap_number;
            char *dom_sock_name;
        } comm_unix_serv_info;
        struct {
            char *port;
            int sap_number;
        } comm_tcp_serv_info;       
    } comm_serv_info_type;
    comm_type_t ipc_type;
    int num_workers;
    void (*client_func)(void *data);
    void (*error_func) (void *data);
    comm_queue_t *comm_queue;
} comm_serv_info_t;

typedef struct comm_client_info__ {
    union {
        struct {
            char *dom_sock_name;
        } comm_unix_client_info;       
        struct {
            char server_ip[COMM_TCP_MAX_IP_LEN];
            uint64_t server_port;
        } comm_tcp_client_info;       
    } comm_client_info_type;
    comm_type_t ipc_type;
} comm_client_info_t;

typedef struct comm_worker_info__ {
    int worker_index;
    int num_fds;
    int num_workers;
    comm_type_t ipc_type;
    void (*client_func)(void *data);
    void (*error_func) (void *data);
    comm_queue_t *comm_queue;
} comm_worker_info_t;

int 
comm_server_init(comm_serv_info_t *comm);
comm_client_t *
comm_client_init(comm_client_info_t *comm);
ssize_t
comm_client_send(comm_client_t *comm_client, 
                         comm_msg_t *comm_msg);
ssize_t
comm_client_send_header(comm_client_t *comm_client, 
                         comm_msg_t *comm_msg);
ssize_t
comm_client_send_data(comm_client_t *comm_client, 
                         comm_msg_t *comm_msg);
ssize_t
comm_servresp_send(comm_server_t *comm_server, 
                           comm_msg_t *comm_msg);
ssize_t
comm_servresp_send_header(comm_server_t *comm_server, 
                           comm_msg_t *comm_msg);
ssize_t
comm_servresp_send_data(comm_server_t *comm_server, 
                           comm_msg_t *comm_msg);
ssize_t 
comm_client_wait(comm_client_t *comm_client, 
                         comm_msg_t *comm_msg);
void
comm_client_finish(comm_client_t *comm);
#endif
