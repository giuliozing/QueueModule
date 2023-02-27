#ifndef DESKTOP_QUEUEMODULE_H
#define DESKTOP_QUEUEMODULE_H

int period, max_elems;
unsigned int elems;//number of elems in the queue

//declaring the node of the queue
struct node {
    char value[31];
    struct list_head kl;
};
//declaring list, mutex, waitqueue
struct list_head head;
struct mutex my_mutex;
wait_queue_head_t my_waitqueue;

#endif //DESKTOP_QUEUEMODULE_H
