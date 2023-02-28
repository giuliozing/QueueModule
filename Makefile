KERNEL_DIR ?= /lib/modules/`uname -r`/build

obj-m = queuemodule.o
queuemodule-objs = queue.o dev.o

all:
  make -C $(KERNEL_DIR) M=`pwd` modules

clean:
  make -C $(KERNEL_DIR) M=`pwd` clean
