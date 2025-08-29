#export CC=x86_64-linux-gnu-gcc-13
obj-m += rcu_lock.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
