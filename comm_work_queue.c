#include "comm_work_queue.h"

comm_queue_t *
comm_init_queueroot() {
    comm_queue_t *root = malloc(sizeof(comm_queue_t));
    static pthread_mutex_t foo_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&root->lock, NULL);
    root->cond  = (pthread_cond_t) PTHREAD_COND_INITIALIZER; 
    root->queue_in  = NULL;
    root->queue_out = NULL;
    return root;
}

void 
comm_queue_insert(comm_queue_entry_t *entry,
                      comm_queue_t *root) {
    while(1) {
        comm_queue_entry_t *in_queue = root->queue_in;
        entry->next = in_queue;
        if(__sync_bool_compare_and_swap(&root->queue_in, in_queue, 
                                        entry)) {
            break;
        }
    }
}

comm_queue_entry_t *
comm_queue_get(comm_queue_t *root) {
    comm_queue_entry_t *head = NULL , *curr = NULL, *next = NULL;
    pthread_mutex_lock(&root->lock);
    if(!root->queue_out) {
        while(1) {
            head = root->queue_in;
            if(__sync_bool_compare_and_swap(&root->queue_in, head, NULL)) {
                while(head) {
                    next = head->next;
                    head->next = root->queue_out;
                    root->queue_out = head;
                    head = next;
                }
                break;
            }
        }
    }
    if(root->queue_out) {
        head = root->queue_out;
        root->queue_out = head->next;
    } 
    pthread_mutex_unlock(&root->lock);
    return head;
}
