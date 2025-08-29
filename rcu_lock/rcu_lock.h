#ifndef MAUSAM_RCU_LOCK_H
#define MAUSAM_RCU_LOCK_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>      // threads related functions
#include <linux/sched.h>        // task structs
#include <linux/delay.h>        // delays
#include <linux/slab.h>         // allocations
#include <linux/rcupdate.h>     // rcu lock

#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

struct task_struct *reader_td[2];
struct task_struct *writer_td;
int reader_id[2] = {1, 2};
int writer_id = 3;
int global_index = 1;

struct rcu_metadata {
        int counter;
        struct rcu_head rcu_head;
};

struct rcu_metadata *rcu_data;

//Declarations
int reader_threads (void *args);
int writer_thread (void *args);
void rcu_free_data_callback(struct rcu_head *data_head);
static int __init kthread_init(void);
static void __exit kthread_exitt(void);

#endif
