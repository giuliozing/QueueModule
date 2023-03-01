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
int my_strlen(char stringa[]){
    int i = 0;
    while(stringa[i]!='\0')
        i++;
    return i;
}
void  *my_memset(void *b, int c, int len)
{
    int i;
    unsigned char *p = b;
    i = 0;
    while(len > 0)
    {
        *p = c;
        p++;
        len--;
    }
    return(b);
}
#endif //DESKTOP_QUEUEMODULE_H
