#include "comm_i.h"
#include "comm_unix_client.h"
#include "comm_tcp_client.h"

comm_client_t *
comm_client_init(comm_client_info_t *comm) {
    comm_client_t *comm_client = NULL;
    int rc = 0;

    comm_client = malloc(sizeof(comm_client_t));
    if(!comm_client) {
        LogCrit(COMPONENT_COMM, "failed to allocate the memory for ipc "
                " client");
        goto end;
    }
    
    if(comm->ipc_type == COMM_UNIX) {
        rc = comm_unix_client_init(comm_client, comm);
    } else if(comm->ipc_type == COMM_TCP) {
        rc = comm_tcp_client_init(comm_client, comm);
    } else {
        assert(0);
    }        
    
    if(rc) {
        LogCrit(COMPONENT_COMM, "failed to init the ipc client type - %d"
                               " error - %d", comm->ipc_type,
                               rc);
        free(comm_client);
        comm_client = NULL;
    }

end:    
    return comm_client;
}

void
comm_client_finish(comm_client_t *comm) {
    if(comm) {
        (comm->comm_client_finish)
                   (comm);
        free(comm);
    }
    return;
}

ssize_t
comm_client_send(comm_client_t *comm_client, 
                         comm_msg_t *comm_msg) {
    return((comm_client->comm_client_send)
           (comm_client, (void *) comm_msg));
}

ssize_t
comm_client_send_header(comm_client_t *comm_client, 
                         comm_msg_t *comm_msg) {
    return((comm_client->comm_client_send_header)
           (comm_client, (void *) comm_msg));
}

ssize_t
comm_client_send_data(comm_client_t *comm_client, 
                         comm_msg_t *comm_msg) {
    return((comm_client->comm_client_send_data)
           (comm_client, (void *) comm_msg));
}

ssize_t
comm_servresp_send(comm_server_t *comm_server,
                           comm_msg_t *comm_msg) {
    
    return((comm_server->comm_servresp_send)
           (comm_server, (void *)comm_msg));
}

ssize_t 
comm_client_wait(comm_client_t *comm_client, 
                         comm_msg_t *comm_msg) {
    return((comm_client->comm_client_wait)
           (comm_client, (void *) comm_msg));
} 

ssize_t
comm_servresp_send_header(comm_server_t *comm_server,
                           comm_msg_t *comm_msg) {
    
    return((comm_server->comm_servresp_send_header)
           (comm_server, (void *)comm_msg));
}

ssize_t
comm_servresp_send_data(comm_server_t *comm_server,
                           comm_msg_t *comm_msg) {
    
    return((comm_server->comm_servresp_send_data)
           (comm_server, (void *)comm_msg));
}
