#include "comm_i.h"
#include "comm_tcp_client.h"

int
comm_tcp_client_init(comm_client_t *comm_client,
                             comm_client_info_t *comm) {
    int s, t;
    struct sockaddr_in remote;
    char str[100];
    int rc = 0;

    s = socket(AF_INET, SOCK_STREAM, 0);

    if(s == -1) {
        LogCrit(COMPONENT_COMM, "client socket creation failed");
        rc = COMM_STATUS_GENERIC;
        goto end;
    }

    memset(&remote, 0, sizeof(struct sockaddr_in)); 
    remote.sin_family = AF_INET;
    remote.sin_port = htons(comm->comm_client_info_type.
                                comm_tcp_client_info.server_port);
    (void) inet_pton(AF_INET, comm->comm_client_info_type.
                                comm_tcp_client_info.server_ip, 
                     &remote.sin_addr);
    
    if(connect(s, (struct sockaddr *)&remote, 
               sizeof(struct sockaddr_in)) == -1) {
        close(s);
        rc = COMM_STATUS_GENERIC;
        LogCrit(COMPONENT_COMM, "connect to server %s failed",
                comm->comm_client_info_type.
                comm_tcp_client_info.server_ip);
        goto end;
    }
    comm_client->ipc_type = COMM_TCP;
    comm_client->comm_client_type.comm_tcp_client.socket_fd = s;
    comm_client->comm_client_send = &comm_tcp_client_send;
    comm_client->comm_client_wait = &comm_tcp_client_wait; 
    comm_client->comm_client_send_header = &comm_tcp_client_send_header;
    comm_client->comm_client_send_data = &comm_tcp_client_send_data; 
    comm_client->comm_client_finish = &comm_tcp_client_finish;    
    usleep(5);

end:
    return rc;
}

void
comm_tcp_client_finish(void *arg1) {
    comm_client_t *comm = NULL;

    comm = (comm_client_t *) arg1;

    if(comm) {
        close(comm->comm_client_type.
                    comm_tcp_client.socket_fd);
    }
    return;
}

ssize_t
comm_tcp_client_send(void *arg1, void *arg2) {
    ssize_t tot_len = 0;
    ssize_t write_len = 0;
    ssize_t write_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int socket_fd = 0;
    comm_client_t *comm_client = NULL;
    comm_msg_t *comm_msg = NULL;

    comm_client = (comm_client_t *) arg1;
    comm_msg = (comm_msg_t *) arg2;
    socket_fd = comm_client->comm_client_type.
                    comm_tcp_client.socket_fd;

    tot_len = sizeof(comm_msg_header_t);
    write_len = tot_len;
    write_count = 0;
    count = 0;
    
    p_msg_header = (char *) &comm_msg->msg_header;                
    
    while(count < tot_len) {
        p_msg_header = p_msg_header + write_count;
        write_count = write(socket_fd, p_msg_header, 
                            write_len);
        if(write_count == 0) {
            LogCrit(COMPONENT_COMM, "write returned 0 continueing");
            usleep(1);
            continue;
        }       
        if(write_count == -1) {
            write_count = 0;
            if(errno == EAGAIN ||
               errno == EINTR) {
                usleep(1);
                continue;
            }
            LogCrit(COMPONENT_COMM, "failed to send the data socket %d"
                    ", error %d",
                    socket_fd, errno);
            ret = COMM_STATUS_GENERIC;
            goto end;
        }
        count = count + write_count;
        write_len = tot_len - count;
        ret = ret + write_count;
    }
    
    if(comm_msg->msg_header.msg_write_len == 0) {
        tot_len = comm_msg->msg_header.msg_len;
    } else {
        tot_len = comm_msg->msg_header.msg_write_len;
    }

    write_len = tot_len;
    write_count = 0;
    count = 0;

    data = (char *) comm_msg->data;
    while(count < tot_len) {
        data = data + write_count;       
        write_count = write(socket_fd, (char *) data, 
                            write_len);
        
        if(write_count == 0) {
            LogCrit(COMPONENT_COMM, "write returned 0 continueing");
            usleep(1);
            continue;
        }       
        if(write_count == -1) {
            write_count = 0;
            if(errno == EAGAIN ||
               errno == EINTR) {
                usleep(1);
                continue;
            }
            LogCrit(COMPONENT_COMM, "failed to send the data socket %d, error %d",
                    socket_fd, errno);
            ret = COMM_STATUS_GENERIC;
            goto end;
        }
        count = count + write_count;
        write_len = tot_len - count;
        ret = ret + write_count; 
    }

end:
    return ret;
}

ssize_t
comm_tcp_client_send_header(void *arg1, void *arg2) {
    ssize_t tot_len = 0;
    ssize_t write_len = 0;
    ssize_t write_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int socket_fd = 0;
    comm_client_t *comm_client = NULL;
    comm_msg_t *comm_msg = NULL;

    comm_client = (comm_client_t *) arg1;
    comm_msg = (comm_msg_t *) arg2;
    socket_fd = comm_client->comm_client_type.
                        comm_tcp_client.socket_fd;

    tot_len = sizeof(comm_msg_header_t);
    write_len = tot_len;
    write_count = 0;
    count = 0;
    
    p_msg_header = (char *) &comm_msg->msg_header;                
    
    while(count < tot_len) {
        p_msg_header = p_msg_header + write_count;
        write_count = write(socket_fd, p_msg_header, 
                            write_len);
        if(write_count == 0) {
            LogCrit(COMPONENT_COMM, "write returned 0 continueing");
            usleep(1);
            continue;
        }       
        if(write_count == -1) {
            write_count = 0;
            if(errno == EAGAIN ||
               errno == EINTR) {
                usleep(1);
                continue;
            }
            LogCrit(COMPONENT_COMM, "failed to send the data socket %d"
                    ", error %d",
                    socket_fd, errno);
            ret = COMM_STATUS_GENERIC;
            goto end;
        }
        count = count + write_count;
        write_len = tot_len - count;
        ret = ret + write_count;
    }

end:
    return ret;
}

ssize_t
comm_tcp_client_send_data(void *arg1, void *arg2) {
    ssize_t tot_len = 0;
    ssize_t write_len = 0;
    ssize_t write_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int socket_fd = 0;
    comm_client_t *comm_client = NULL;
    comm_msg_t *comm_msg = NULL;

    comm_client = (comm_client_t *) arg1;
    comm_msg = (comm_msg_t *) arg2;
    socket_fd = comm_client->comm_client_type.
                        comm_tcp_client.socket_fd;

    if(comm_msg->msg_header.msg_write_len == 0) {
        tot_len = comm_msg->msg_header.msg_len;
    } else {
        tot_len = comm_msg->msg_header.msg_write_len;
    }
    write_len = tot_len;
    write_count = 0;
    count = 0;

    data = (char *) comm_msg->data;
    while(count < tot_len) {
        data = data + write_count;       
        write_count = write(socket_fd, (char *) data, 
                           write_len);
        
        if(write_count == 0) {
            LogCrit(COMPONENT_COMM, "write returned 0 continueing");
            usleep(1);
            continue;
        }       
        if(write_count == -1) {
            write_count = 0;
            if(errno == EAGAIN ||
               errno == EINTR) {
                usleep(1);
                continue;
            }
            LogCrit(COMPONENT_COMM, "failed to send the data socket %d, error %d",
                    socket_fd, errno);
            ret = COMM_STATUS_GENERIC;
            goto end;
        }
        count = count + write_count;
        write_len = tot_len - count;
        ret = ret + write_count; 
    }

end:
    return ret;
}

ssize_t 
comm_tcp_client_wait(void *arg1, void *arg2) {
    ssize_t tot_len = 0;
    ssize_t read_len = 0;
    ssize_t read_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int socket_fd = 0;
    comm_client_t *comm_client = NULL;
    comm_msg_t *comm_msg = NULL;

    comm_client = (comm_client_t *) arg1;
    comm_msg = (comm_msg_t *) arg2;
    socket_fd = comm_client->comm_client_type.
                        comm_tcp_client.socket_fd;
    tot_len = sizeof(comm_msg_header_t);
    read_len = tot_len;
    read_count = 0;
    count = 0;
    
    p_msg_header = (char *) &comm_msg->msg_header;                

    while(count < tot_len) {
        p_msg_header = p_msg_header + read_count;
        read_count = read(socket_fd, p_msg_header, 
                          read_len);
        
        if(read_count == 0) {
            ret = COMM_STATUS_PEER_CLOSE;
            LogCrit(COMPONENT_COMM, "other end close the connection");
            goto end;
        }       
        if(read_count == -1) {
            read_count = 0;
            if(errno == EAGAIN ||
               errno == EINTR) {
                usleep(1);
                continue;
            }
           
            LogCrit(COMPONENT_COMM, "failed to read on %d",
                    socket_fd);
            ret = COMM_STATUS_GENERIC;
            goto end;
        }
        
        count = count + read_count;
        read_len = tot_len - count;
        ret = ret + read_count;
    }
                
    tot_len = comm_msg->msg_header.msg_len;
    read_len = tot_len;
    read_count = 0;
    count = 0;
    comm_msg->
        comm_client.
            comm_client_type.comm_tcp_client.socket_fd = socket_fd;
     
    if(tot_len > 0) {  
        comm_msg->data = malloc(tot_len);
        if(!comm_msg->data) {
            LogCrit(COMPONENT_COMM, "failed to allcate memory for data");  
            ret = COMM_STATUS_GENERIC;
            goto end;
        }
    
        data = (char *) comm_msg->data;
        while(count < tot_len) {
            data = data + read_count;       
            read_count = read(socket_fd, (char *) data, 
                              read_len);
        
            if(read_count == 0) {
                ret = COMM_STATUS_PEER_CLOSE;
                LogCrit(COMPONENT_COMM, "other end close the connection");
                goto end;
            }       
            if(read_count == -1) {
                read_count = 0;
                if(errno == EAGAIN ||
                    errno == EINTR) {
                    usleep(1);
                    continue;
                }
            
                LogCrit(COMPONENT_COMM, "failed to read on the fd %d",
                        socket_fd);
                ret = COMM_STATUS_GENERIC;
                goto end;
            }
        
            count = count + read_count;
            read_len = tot_len - count;
            ret = ret + read_count;
        }
    }
end:
    return ret;
}


