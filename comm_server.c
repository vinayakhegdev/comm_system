#include "comm_i.h"
#include "comm_work_queue.h"

pthread_mutex_t global_cur_worker_lock = PTHREAD_MUTEX_INITIALIZER;
int cur_worker_index = 0;

ssize_t
comm_unix_servresp_send(void *arg1, void *arg2) {
    ssize_t tot_len = 0;
    ssize_t write_len = 0;
    ssize_t write_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int socket_fd = 0;
    comm_server_t *comm_server = NULL;
    comm_msg_t *comm_msg = NULL;

    comm_server = (comm_server_t *) arg1;
    comm_msg = (comm_msg_t *) arg2;
    socket_fd = comm_server->comm_server_type.
                        comm_unix_server.socket_fd;
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
            LogCrit(COMPONENT_COMM, "failed to send the data socket %d,"
                    "error %d",
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
comm_unix_servresp_send_header(void *arg1, void *arg2) {
    ssize_t tot_len = 0;
    ssize_t write_len = 0;
    ssize_t write_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int socket_fd = 0;
    comm_server_t *comm_server = NULL;
    comm_msg_t *comm_msg = NULL;
    
    comm_server = (comm_server_t *) arg1;
    comm_msg = (comm_msg_t *) arg2;
    socket_fd = comm_server->comm_server_type.
                            comm_unix_server.socket_fd;
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
            LogCrit(COMPONENT_COMM, "failed to send the data socket %d,"
                    "error %d",
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
comm_unix_servresp_send_data(void  *arg1, void *arg2) {
    ssize_t tot_len = 0;
    ssize_t write_len = 0;
    ssize_t write_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int socket_fd = 0;
    comm_server_t *comm_server = NULL;
    comm_msg_t *comm_msg = NULL;
    
    comm_server = (comm_server_t *) arg1;
    comm_msg = (comm_msg_t *) arg2;
    socket_fd = comm_server->comm_server_type.
                            comm_unix_server.socket_fd;

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
comm_tcp_servresp_send(void *arg1, void *arg2) {
    ssize_t tot_len = 0;
    ssize_t write_len = 0;
    ssize_t write_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int socket_fd = 0;
    comm_server_t *comm_server = NULL;
    comm_msg_t *comm_msg = NULL;
    
    comm_server = (comm_server_t *) arg1;
    comm_msg = (comm_msg_t *) arg2;
    socket_fd = comm_server->comm_server_type.
                        comm_tcp_server.socket_fd;
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
            LogCrit(COMPONENT_COMM, "failed to send the data socket %d,"
                    "error %d",
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
comm_tcp_servresp_send_header(void *arg1, void *arg2) {
    ssize_t tot_len = 0;
    ssize_t write_len = 0;
    ssize_t write_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int socket_fd = 0;
    comm_server_t *comm_server = NULL;
    comm_msg_t *comm_msg = NULL;
    
    comm_server = (comm_server_t *) arg1;
    comm_msg = (comm_msg_t *) arg2;
    socket_fd = comm_server->comm_server_type.
                            comm_tcp_server.socket_fd;
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
            LogCrit(COMPONENT_COMM, "failed to send the data socket %d,"
                    "error %d",
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
comm_tcp_servresp_send_data(void *arg1, void *arg2) {
    ssize_t tot_len = 0;
    ssize_t write_len = 0;
    ssize_t write_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int socket_fd = 0;
    comm_server_t *comm_server = NULL;
    comm_msg_t *comm_msg = NULL;
    
    comm_server = (comm_server_t *) arg1;
    comm_msg = (comm_msg_t *) arg2;
    socket_fd = comm_server->
                    comm_server_type.comm_tcp_server.socket_fd;

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

int
comm_make_socket_non_blocking(int sfd) {
    int flags;
    int s;
    int ret = 0;

    flags = fcntl(sfd, F_GETFL, 0);
    if(flags == -1) {
        LogCrit(COMPONENT_COMM, "failed to get flags on the socket %d",
                sfd);
        ret = -1;
        goto end;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if(s == -1) {
        LogCrit(COMPONENT_COMM, "failed to set flags on the socket %d",
                sfd);
        ret = -1;
        goto end;
    }

end:
    return ret;
}

int
comm_create_and_bind(comm_serv_info_t *comm_thread_data) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd = -1;
    struct sockaddr_un addr;
    int reuseaddr = 1;

    if(comm_thread_data->ipc_type == COMM_TCP) {
        memset(&hints, 0, sizeof (struct addrinfo));
        hints.ai_family = AF_UNSPEC;     
        hints.ai_socktype = SOCK_STREAM; 
        hints.ai_flags = AI_PASSIVE;     

        s = getaddrinfo(NULL, 
                        comm_thread_data->comm_serv_info_type.
                                    comm_tcp_serv_info.port, 
                        &hints, &result);
        if(s != 0) {
            LogCrit(COMPONENT_COMM, "get addr info failed");
            sfd = -1;
            goto end;
        }

        for(rp = result; rp != NULL; rp = rp->ai_next) {
            sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if(sfd == -1) {
                continue;
            }
            if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
               &reuseaddr, sizeof(int)) == -1) {
                LogCrit(COMPONENT_COMM, "failed to set the reuse address");
            }
 
            s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
            if(s == 0) {
                break;
            }
            close (sfd);
        }

        if(rp == NULL) {
            sfd = -1;
            LogCrit(COMPONENT_COMM, "failed to bind tcp socket in sfp, exiting");
            exit(0);
            goto end;
        }

        freeaddrinfo(result);
    } else if (comm_thread_data->ipc_type == COMM_UNIX) {
        sfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if(sfd == -1) {
            LogCrit(COMPONENT_COMM, "failed to create the unix domain socket %s",
                    comm_thread_data->
                        comm_serv_info_type.comm_unix_serv_info.dom_sock_name);
            goto end;
        }      

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, 
                comm_thread_data->comm_serv_info_type.
                        comm_unix_serv_info.dom_sock_name, 
                strlen(comm_thread_data->
                        comm_serv_info_type.
                        comm_unix_serv_info.dom_sock_name));
        unlink(comm_thread_data->comm_serv_info_type.
                        comm_unix_serv_info.dom_sock_name);
        
        if(bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            LogCrit(COMPONENT_COMM, "bind failed on unix domain socket %s",
                    comm_thread_data->comm_serv_info_type.
                        comm_unix_serv_info.dom_sock_name);
            close(sfd);
            sfd = -1;
            goto end;
        }
    }
end:
    return sfd;
}

int 
comm_get_worker_index(comm_worker_info_t *worker_data) {
    int worker_index = 0;

    pthread_mutex_lock(&global_cur_worker_lock);
    worker_index = cur_worker_index;
    if(worker_data->worker_index == cur_worker_index) {
        cur_worker_index = cur_worker_index + 1;
        cur_worker_index = cur_worker_index % worker_data->num_workers;
    }
    pthread_mutex_unlock(&global_cur_worker_lock);

    return worker_index;
}

void * 
comm_worker_run(void * ctx) {
    comm_worker_info_t *worker_data = (comm_worker_info_t *) ctx;
    comm_queue_entry_t *queue_entry = NULL;
    struct epoll_event event = {0};
    struct epoll_event *events;
    int efd = 0;
    int n = 0;
    comm_msg_t *comm_msg = NULL;    
    ssize_t tot_len = 0;
    ssize_t read_len = 0;
    ssize_t read_count = 0;
    ssize_t count = 0;
    ssize_t ret = 0;
    int s = 0;
    int i = 0;
    char *p_msg_header = NULL;
    char *data = NULL;
    int worker_index = 0;
    comm_err_msg_t *comm_err = NULL;

    efd = epoll_create1(0);
    if(efd == -1) {
        LogCrit(COMPONENT_COMM, "epoll creation failed in ipc server");
        goto end;
    }

    events = calloc(MAXEVENTS, sizeof(event));

    if(!events) {
        LogCrit(COMPONENT_COMM, " failed to allocate the events");
        goto end;
    }
  

	while(1) {
        worker_index = comm_get_worker_index(worker_data);
        if(worker_index == worker_data->worker_index) {
            queue_entry = comm_queue_get(worker_data->comm_queue);
            if(queue_entry) {
                event.data.fd = queue_entry->socket_fd;
                event.events = EPOLLIN | EPOLLET;
                s = epoll_ctl(efd, EPOLL_CTL_ADD, queue_entry->socket_fd, &event);
                if(s == -1) {
                    LogCrit(COMPONENT_COMM, "epoll add failed for socket fd %d",
                            queue_entry->socket_fd);
                    free(queue_entry);
                    continue;
                }
        
                free(queue_entry);
                worker_data->num_fds = worker_data->num_fds + 1;
            }
        }

poll_again:
        n = epoll_wait(efd, events, MAXEVENTS, EPOLL_WAIT_TIMEOUT);
        
        if(n == -1) {
            continue;
        }

        for(i = 0; i < n; i++) {
            if((events[i].events & EPOLLERR) ||
                          (events[i].events & EPOLLHUP) ||
                      (!(events[i].events & EPOLLIN))) {
                /* An error has occured on this fd, or the socket is not
                   ready for reading (why were we notified then?) */
	      
                LogCrit(COMPONENT_COMM, "error on the socket %d",
                        events[i].data.fd);
                close(events[i].data.fd);
	            /* call the error function */
                if(worker_data->error_func) {
                    comm_err = malloc(sizeof(comm_err_msg_t));
                    if(comm_err) {
                        memset(comm_err, 0, sizeof(comm_err_msg_t));
                        if(worker_data->ipc_type == COMM_UNIX) {
                            comm_err->comm_server.comm_server_type.
                                            comm_unix_server.socket_fd = events[i].data.fd;
                        } else if(worker_data->ipc_type == COMM_TCP) {
                            comm_err->comm_server.comm_server_type.
                                    comm_tcp_server.socket_fd = events[i].data.fd;
                        } else {
                            assert(0);
                        }      
                        worker_data->error_func(comm_err);
                        free(comm_err);
                    } else {
                        LogCrit(COMPONENT_COMM, "memory allocation failed to call "
                                "err client callback %d", events[i].data.fd);
                    }       
                }
                continue;
            } else if(events[i].events & EPOLLIN) {
                /* now read from the socket */
                comm_msg = malloc(sizeof(comm_msg_t));
                if(!comm_msg) {
                    LogCrit(COMPONENT_COMM, "comm msg allocation failed");
                    assert(0);
                }
                memset(comm_msg, 0, sizeof(comm_msg_t));
                tot_len = sizeof(comm_msg_header_t);
                read_len = tot_len;
                read_count = 0;
                count = 0;
                p_msg_header = (char *) &comm_msg->msg_header;                
                while(count < tot_len) {
                    p_msg_header = p_msg_header + read_count;
                    read_count = read(events[i].data.fd, p_msg_header, 
                                      read_len);
                    if(read_count == 0) {
                        LogCrit(COMPONENT_COMM, "other end closed the connection");
                        free(comm_msg);
                        goto poll_again;
                    }     
                    if(read_count == -1) {
                        read_count = 0;
                        if(errno == EAGAIN ||
                           errno == EINTR) {
                            usleep(1);
                            continue;
                        }
                        LogCrit(COMPONENT_COMM, "failed to read on %d",
                                events[i].data.fd);
                        free(comm_msg);
                        goto poll_again;
                    }
                    count = count + read_count;
                    read_len = tot_len - count;
                }
                
                
                tot_len = comm_msg->msg_header.msg_len;
                read_len = tot_len;
                read_count = 0;
                count = 0;
                if(worker_data->ipc_type == COMM_UNIX) {
                    comm_msg->comm_server.comm_server_type.
                            comm_unix_server.socket_fd = events[i].data.fd;
                    comm_msg->comm_server.comm_server_type.
                            comm_unix_server.worker_index = 
                                worker_data->worker_index;
                    comm_msg->comm_server.
                              comm_servresp_send = &comm_unix_servresp_send;  
                    comm_msg->comm_server.
                              comm_servresp_send_header = 
                                    &comm_unix_servresp_send_header;  
                    comm_msg->comm_server.
                              comm_servresp_send_data = 
                                    &comm_unix_servresp_send_data;  
                } else if(worker_data->ipc_type == COMM_TCP) {
                    comm_msg->comm_server.comm_server_type.
                            comm_tcp_server.socket_fd = events[i].data.fd;
                    comm_msg->comm_server.comm_server_type.
                            comm_tcp_server.worker_index =
                                worker_data->worker_index;
                    comm_msg->comm_server.
                              comm_servresp_send = &comm_tcp_servresp_send;  
                    comm_msg->comm_server.
                              comm_servresp_send_header = 
                                    &comm_tcp_servresp_send_header;  
                    comm_msg->comm_server.
                              comm_servresp_send_data = 
                                    &comm_tcp_servresp_send_data;  
                } else {
                    assert(0);
                }       
                if(tot_len > 0) {
                    comm_msg->data = malloc(tot_len);
                    if(!comm_msg->data) {
                        LogCrit(COMPONENT_COMM, "failed to allcate memory for data");
                        assert(0); 
                    }
            
                    data = (char *) comm_msg->data;
                    while(count < tot_len) {
                        data = data + read_count; 
                        read_count = read(events[i].data.fd, (char *) data, 
                                          read_len);
                    
                        if(read_count == 0) {
                            LogCrit(COMPONENT_COMM, "other end closed the connection");
                            free(comm_msg->data);
                            free(comm_msg);
                            goto poll_again;
                        }     
                        if(read_count == -1) {
                            read_count = 0;
                            if(errno == EAGAIN ||
                                errno == EINTR) {
                                usleep(1);
                                continue;
                            }
                            LogCrit(COMPONENT_COMM, "failed to read on the fd %d",
                                    events[i].data.fd);
                            free(comm_msg->data);
                            free(comm_msg);
                            goto poll_again;
                        }
                    
                        count = count + read_count;
                        read_len = tot_len - count;
                    }
                }
                /* now call the client callback */
                if(worker_data->client_func) {
                    worker_data->client_func(comm_msg);   
                } else {
                    if(comm_msg->data) {
                        free(comm_msg->data);
                    }
                    free(comm_msg);
                }      
            }
        }
    } 

end:
    return NULL;
}

void *      
comm_serv_thread(void *ipc_thr_data) {
    int sfd, s;
    int efd;
    struct epoll_event event = {0};
    struct epoll_event *events;
    comm_serv_info_t *comm_thr_data = NULL;
    comm_queue_entry_t *queue_entry = NULL;

    comm_thr_data = (comm_serv_info_t *)
                                (ipc_thr_data);

    sfd = comm_create_and_bind(comm_thr_data);
    
    if(sfd == -1) {
        LogCrit(COMPONENT_COMM, "failed to create and bind the socket");
    }       

    s = comm_make_socket_non_blocking(sfd);
    if(s == -1) {
        LogCrit(COMPONENT_COMM, "failed to make the socket non blocking");
    }
    s = listen(sfd, SOMAXCONN);
    if(s == -1) {
        LogCrit(COMPONENT_COMM, "failed to listen on the socket");
    }

    efd = epoll_create1(0);
    if(efd == -1) {
        LogCrit(COMPONENT_COMM, "failed to do epoll create");
    }

    events = calloc(MAXEVENTS, sizeof(event));
    
    if(!events) {
        LogCrit(COMPONENT_COMM, "failed to allocate the events");
    }
    
    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if(s == -1) {
        LogCrit(COMPONENT_COMM, "epoll add on fd %d failed",
                sfd);
    }

    /* The event loop */
    while(1) {
        int n, i;

        n = epoll_wait(efd, events, MAXEVENTS, -1);
        for(i = 0; i < n; i++) {
	        if((events[i].events & EPOLLERR) ||
                          (events[i].events & EPOLLHUP) ||
                      (!(events[i].events & EPOLLIN))) {
                LogCrit(COMPONENT_COMM, "socket error %d", events[i].data.fd);
                close(events[i].data.fd);
	            continue;
	        } else if (sfd == events[i].data.fd) {
                /* We have a notification on the listening socket, which
                 means one or more incoming connections. */
                while(1) {
                    struct sockaddr in_addr = {0};
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST] = {0}, sbuf[NI_MAXSERV] = {0};

                    in_len = sizeof in_addr;
                    infd = accept(sfd, &in_addr, &in_len);
                    if(infd == -1) {
                        if((errno == EAGAIN) ||
                           (errno == EWOULDBLOCK)) {
                            /* We have processed all incoming
                               connections. */
                            break;
                        } else {
                            LogCrit(COMPONENT_COMM, "accept connection error");
                            break;
                        }
                    }
                    s = getnameinfo(&in_addr, in_len,
                                    hbuf, sizeof hbuf,
                                    sbuf, sizeof sbuf,
                                    NI_NUMERICHOST | NI_NUMERICSERV);
                    s = comm_make_socket_non_blocking(infd);
                    if(s == -1) {
                        LogCrit(COMPONENT_COMM, "failed to make socket non blocking %d",
                                infd);
                    }
                    
                    int flag = 1;
 
                    setsockopt(infd, IPPROTO_TCP, TCP_NODELAY,
                                (char *) &flag, sizeof(int));
 
                    queue_entry = malloc(sizeof(comm_queue_entry_t));
                    memset(queue_entry, 0, sizeof(queue_entry)); 
                    queue_entry->socket_fd = infd;
                    
                    comm_queue_insert(queue_entry, comm_thr_data->comm_queue);
               }
               continue;
            } else {
                /* We have data on the fd waiting to be read. Read and
                   display it. We must read whatever data is available
                   completely, as we are running in edge-triggered mode
                   and won't get a notification again for the same
                   data. */
                /* shouldnt come here, log it */
                assert(0);
            }
        }
    }
    
    free(events);

    close(sfd);

    return 0;
}

int 
comm_server_init(comm_serv_info_t *comm) {
    int rc = 0;
    int i = 0;
    pthread_attr_t attr;
    pthread_t thread_id;
    comm_worker_info_t *comm_worker_info = NULL;
    comm_serv_info_t *comm_server_info = NULL;

    if(pthread_attr_init(&attr) != 0) {
        LogCrit(COMPONENT_COMM, "pthread attr init failed");
        rc = -1;
        goto end;
    }

    if(pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM) != 0) {
        LogCrit(COMPONENT_COMM, "pthread attr scope failed");
        rc = -1;
        goto end;
    }

    if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) != 0) {
        LogCrit(COMPONENT_COMM, "pthread detached state failed");
        rc = -1;
        goto end;
    }

    comm_server_info = malloc(sizeof(comm_serv_info_t));
    if(!comm_server_info) {
        LogCrit(COMPONENT_COMM, "memory allcotion failed for server info");
        rc = -1;
        goto end;
    }       

    memcpy(comm_server_info, comm, 
               sizeof(comm_serv_info_t));

    comm_server_info->comm_queue = comm_init_queueroot();
    
    pthread_mutex_init(&global_cur_worker_lock, NULL);
    
    for(i = 0; i < comm->num_workers; i++) {
        comm_worker_info = malloc(sizeof(comm_worker_info_t));
        memset(comm_worker_info, 0, sizeof(comm_worker_info_t));
        if(comm->ipc_type == COMM_UNIX) {
            comm_worker_info->num_workers = 
                        comm->num_workers;
            comm_worker_info->worker_index = i;
            comm_worker_info->ipc_type = comm->ipc_type;
            comm_worker_info->client_func = comm->client_func;
            comm_worker_info->error_func = comm->error_func;
            comm_worker_info->comm_queue = comm_server_info->comm_queue;
        } else if(comm->ipc_type == COMM_TCP) {
            comm_worker_info->num_workers = 
                        comm->num_workers;
            comm_worker_info->worker_index = i;
            comm_worker_info->ipc_type = comm->ipc_type;
            comm_worker_info->client_func = comm->client_func;
            comm_worker_info->error_func = comm->error_func;
            comm_worker_info->comm_queue = comm_server_info->comm_queue;
        } else {
            assert(0);
        }

        rc = pthread_create(&thread_id, &attr, comm_worker_run,
                           (void *)comm_worker_info); 
        if(rc) {
            LogCrit(COMPONENT_COMM, "pthread creation of worker failed index %d",
                    i);
            goto end;
        }
    }        

    /* start the server threads */
    rc = pthread_create(&thread_id, &attr, comm_serv_thread,
                        (void *) comm_server_info);

    if(rc) {
        LogCrit(COMPONENT_COMM, "ipc server thread cretion failed");
    }

end:
    return rc;
}

