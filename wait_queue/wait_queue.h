#ifndef WAIT_QUEUE_H
#define WAIT_QUEUE_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>        // Set task states
#include <linux/kernel.h>
#include <linux/kthread.h>      // threads related functions
#include <linux/sched.h>        // task structs
#include <linux/delay.h>        // delays
#include <linux/interrupt.h>    // Tasklet operations
#include <linux/slab.h>         // Allocation
#include <linux/list.h>         // List
#include <linux/wait.h>         // Waitqueues

#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

wait_queue_head_t *kern_wait_queue;
struct task_struct *kthread_1;
struct task_struct *kthread_2;
int thread_data_1 = 1;
int thread_data_2 = 2;
int condition = 0;

//Function Prototypes
int kern_thread_1 (void *args);
int kern_thread_2 (void *args);
static int __init wait_queue_init(void);
static void __exit wait_queue_exit(void);

#endif
