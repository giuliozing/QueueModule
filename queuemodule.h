#ifndef DESKTOP_QUEUEMODULE_H
#define DESKTOP_QUEUEMODULE_H

static int period = 1000;
module_param(period, int, 0);//the parameter period can be set while inserting the module
static int max_elems = 5;
module_param(max_elems, int, 0);//the max_elems period can be set while inserting the module: sets the maximum length of the queue
static unsigned int elems;//number of elems in the queue

//declaring the node of the queue
struct node {
    char value[31];
    struct list_head kl;
};
//declaring list, mutex, waitqueue
static struct list_head head;
static struct mutex my_mutex;
static wait_queue_head_t my_waitqueue;

#endif //DESKTOP_QUEUEMODULE_H
