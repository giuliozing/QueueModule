#include <linux/poll.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/list.h>
#include <linux/mutex.h>

#include "queuemodule.h"

extern struct miscdevice queue_device;
period = 1000;
module_param(period, int, 0);//the parameter period can be set while inserting the module
max_elems = 5;
module_param(max_elems, int, 0);//the max_elems period can be set while inserting the module: sets the maximum length of the queue
static int dev_module_init(void)
{
    int res;
    res = misc_register(&queue_device);
    printk("Misc Register returned %d, the period set is %d and the maximum length of the queue is %d\n", res, period, max_elems);
    //initializing list, mutex and waitqueue
    INIT_LIST_HEAD(&head);
    printk("Queue was successfully initialized\n");
    mutex_init(&my_mutex);
    init_waitqueue_head(&my_waitqueue);
    elems = 0;
    return 0;
}

static void dev_module_exit(void)
{
    struct list_head *l, *tmp;
    struct node *n;
    unsigned int info = elems;
    printk("Cleaning up queue:\n");
    list_for_each_safe(l, tmp, &head){
        n = list_entry(l, struct node, kl);
        list_del(l);
        kfree(n);
    }
    misc_deregister(&queue_device);
    printk("Misc Device was correctly deregistered. Lenght of the queue: %d\n", info);
}

module_init(dev_module_init);
module_exit(dev_module_exit);
