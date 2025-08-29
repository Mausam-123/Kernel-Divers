#ifndef MAUSAM_TIMER_H
#define MAUSAM_TIMER_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>      // threads related functions
#include <linux/sched.h>        // task structs
#include <linux/delay.h>        // delays
#include <linux/slab.h>         // allocations
#include <linux/completion.h>   // wait and completion
#include <linux/atomic.h>       // atomic operations
#include <linux/time.h>         // timer operation

#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

struct task_struct *send_thread;
struct task_struct *rcv_thread;
int global_data;

struct completion signal_event;
struct timer_list timer;

//Declarations
int sender_threads (void *args);
int receiver_thread (void *args);
void timer_callback(struct timer_list *ticks);
static int __init kthread_init(void);
static void __exit kthread_close(void);

#endif
