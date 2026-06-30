# QueueModule

A Linux kernel module implementing a FIFO queue as a misc character device, built using kernel linked lists, mutexes, and wait queues.

> **Academic recognition:** This project was submitted and evaluated as part of a kernel programming exam at [Scuola Superiore Sant'Anna](https://www.santannapisa.it/), Pisa, under Professor Luca Abeni. Grade: **30/30**.

---

## Overview

`QueueModule` exposes a character device (`/dev/queue_device`) that behaves as a bounded FIFO queue of strings. User-space processes interact with the queue by reading and writing to the device file using standard I/O system calls.

- **Writers** enqueue a string (up to 30 characters). If the queue is full, the writer sleeps on a wait queue until a slot becomes available.
- **Readers** dequeue the oldest string (head of the queue). If the queue is empty, the read returns immediately with 0 bytes. After dequeuing, the reader sleeps for a configurable period between processing each remaining element.
- A **mutex** serializes all access to the queue, making the module safe for concurrent use.

---

## Architecture

The module is split into two compilation units:

| File | Role |
|---|---|
| `dev.c` | Module entry/exit: registers the misc device, initializes the list head, mutex, and wait queue |
| `queue.c` | File operations: `open`, `release`, `read`, `write` |
| `queuemodule.h` | Shared data structures, global variables, and the `my_strlen` helper |
| `Makefile` | Out-of-tree kernel module build rules |

### Internal data structures

```
struct node {
    char value[31];       /* null-terminated string payload */
    struct list_head kl;  /* embedded kernel list node      */
};

struct list_head head;        /* queue head (doubly-linked circular list) */
struct mutex my_mutex;        /* protects all queue operations            */
wait_queue_head_t my_waitqueue; /* writers sleep here when queue is full  */
```

The queue is backed by the kernel's built-in doubly-linked circular list (`linux/list.h`). Elements are appended at the tail (`list_add_tail`) and removed from the head (`list_del` + `kfree`).

---

## Module Parameters

Both parameters can be set at `insmod` time:

| Parameter | Default | Description |
|---|---|---|
| `period` | `1000` | Sleep duration (ms) between processing each element during a `read` |
| `max_elems` | `5` | Maximum number of elements in the queue |

Example:
```sh
sudo insmod queuemodule.ko period=500 max_elems=10
```

---

## Build

Requires kernel headers for the running kernel (e.g. `linux-headers-$(uname -r)` on Debian/Ubuntu).

```sh
make
```

To clean build artifacts:
```sh
make clean
```

---

## Usage

### Load the module

```sh
sudo insmod queuemodule.ko
```

Check the device was registered:
```sh
ls -l /dev/queue_device
dmesg | tail
```

### Write (enqueue)

```sh
echo "hello" | sudo tee /dev/queue_device
```

If the queue is full (i.e. `elems == max_elems`), the write blocks until a reader frees a slot.

### Read (dequeue)

```sh
sudo cat /dev/queue_device
```

Returns the oldest element (FIFO order) and removes it from the queue. Returns 0 bytes if the queue is empty.

### Remove the module

```sh
sudo rmmod queuemodule
```

The exit handler drains and frees all remaining nodes before deregistering the device.

---

## Kernel concepts demonstrated

- **Out-of-tree kernel module** — build system, `module_init`/`module_exit`, `MODULE_LICENSE`
- **Misc device** — lightweight character device registration via `misc_register`
- **Kernel linked lists** — `list_head`, `list_add_tail`, `list_for_each_safe`, `list_entry`
- **Mutex** — `mutex_lock`/`mutex_unlock` for mutual exclusion in sleepable context
- **Wait queues** — `wait_event_interruptible` / `wake_up_interruptible` for producer-consumer blocking
- **Module parameters** — `module_param` for runtime configuration
- **Safe user-space I/O** — `copy_to_user` / `copy_from_user`
- **Signal handling** — `TASK_INTERRUPTIBLE` + `signal_pending` + `ERESTARTSYS`

---

## Author

**Giulio Zingrillo** — [gzingrillo@ethz.ch](mailto:gzingrillo@ethz.ch)

Professor: **Luca Abeni** — Scuola Superiore Sant'Anna, Pisa

---

## License

This project is released under the [GPL-2.0 License](LICENSE), consistent with the Linux kernel's licensing requirements for loadable modules.
