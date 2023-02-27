
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

MODULE_AUTHOR("Giulio Zingrillo");
MODULE_DESCRIPTION("Misc queue device");
MODULE_LICENSE("GPL");

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

static int my_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
  return 0;
}

ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)//servono i mutex nella read? Nella write e immediato
{
    struct list_head *l, *tmp;
    struct node *n;
    int i =0, err, res;
    mutex_lock(&my_mutex);
    if(list_empty(&head)){
        printk("Queue is empty\n");
        mutex_unlock(&my_mutex);
        return 0;
    }
    if (len > 30) {
        res = 30;
    }
    else {
        res = len;
    }
        printk("Printing items in the queue...");
    list_for_each_safe(l, tmp, &head){
        n = list_entry(l, struct node, kl);
        printk("Value: %s\n", n->value);
        if(i==0){//only deletes the first element
            i=1;
            err = copy_to_user(buf, n->value, res);//copio il primo elemento nel buffer
            //utilizzo la funzione copy to user, perché già implementata
            if (err) {//controllo che la copia sia andata a buon fine
                return -EFAULT;
                }
            list_del(l);
            kfree(n);
            (elems)--;
        }
        printk("Sleeping for %d ms...\n", period);//the task sleeps for the set period
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(period * (HZ/1000));
        if (signal_pending(current)){
            mutex_unlock(&my_mutex);
            return -ERESTARTSYS;
        }
        printk("Process awake\n");
        }
    printk("First element successfully deleted. Elements are now %d\n", elems);
    if(elems == max_elems-1)
        wake_up_interruptible(&my_waitqueue);
    mutex_unlock(&my_mutex);
    return res;//ritorno il primo elemento della lista
}

static ssize_t my_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos){
    int err, len, res;
    struct node *n;
    mutex_lock(&my_mutex);
    //checks the input length
    if (count > 30){
        printk("The string you wrote is too long");//control in the input length
        return -1;
    }
    else
        len = count;
    //adds the task to a waitqueue if the queue has the max number of elements
    while(elems==max_elems){
        mutex_unlock(&my_mutex);
        res = wait_event_interruptible(my_waitqueue, (elems<max_elems));
        //signal handling
        if(res<0)
            return res;
        mutex_lock(&my_mutex);
    }
    n = kmalloc(sizeof(struct node), GFP_KERNEL);
    //kmalloc handling
    if (n == NULL) {
        printk("Failed to allocate memory\n");
        return -1;
    }
    err = copy_from_user(n->value, buf, len);
    //copy handling
    if (err) {
        mutex_unlock(&my_mutex);
        return -EFAULT;
    }
    n->value[len] = '\0';
    list_add_tail(&(n->kl), &head);
    (elems)++;
    printk("A new element was added to the queue! Elements are now %d.\n", elems);
    mutex_unlock(&my_mutex);
    return len +1;
}


static struct file_operations my_fops = {
  .owner =        THIS_MODULE,
  .read =         my_read,
  .open =         my_open,
  .write =        my_write,
  .release =      my_close,
#if 0
  .poll =         my_poll,
  .fasync =       my_fasync,
#endif
};

static struct miscdevice queue_device = {
  MISC_DYNAMIC_MINOR, "queue_device", &my_fops
};




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