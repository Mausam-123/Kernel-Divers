#ifndef MAUSAM_SEMAPHORE_H
#define MAUSAM_SEMAPHORE_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>      // threads related functions
#include <linux/sched.h>        // task structs
#include <linux/delay.h>        // delays
#include <linux/slab.h>         // allocations
#include <linux/semaphore.h>    // semaphore lock

#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

struct task_struct *reader_td[4];
struct task_struct *writer_td;
int reader_id[4] = {1, 2, 3, 4};
int writer_id = 5;
int global_data = 1;
int read_thread_count = 4;
int owner_count = 5;

struct semaphore sem_lock;

//Declarations
int reader_threads (void *args);
int writer_thread (void *args);
static int __init kthread_init(void);
static void __exit kthread_exitt(void);

#endif
