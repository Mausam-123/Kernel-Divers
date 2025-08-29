#ifndef MAUSAM_WAIT_AND_COMPLETION_H
#define MAUSAM_WAIT_AND_COMPLETION_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>      // threads related functions
#include <linux/sched.h>        // task structs
#include <linux/delay.h>        // delays
#include <linux/slab.h>         // allocations
#include <linux/completion.h>   // wait and completion
#include <linux/atomic.h>       // atomic operations

#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

struct task_struct *waiter_td[4];
struct task_struct *updater_td;
int waiter_id[4] = {1, 2, 3, 4};
int updater_id = 5;
int global_data[4];
int waiter_thread_count = 4;
atomic_t vote;

struct completion comp_var;

//Declarations
int waiter_threads (void *args);
int updater_thread (void *args);
static int __init kthread_init(void);
static void __exit kthread_close(void);

#endif
