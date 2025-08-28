#ifndef MAUSAM_MUTEX_H
#define MAUSAM_MUTEX_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>      // kernel thread
#include <linux/sched.h>        // task structs
#include <linux/delay.h>        // delays
#include <linux/mutex.h>        // mutex locks
#include <linux/slab.h>         // kernel allocations

#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

struct task_struct *kthread_1;
struct task_struct *kthread_2;
int thread_1 = 1;
int thread_2 = 2;
struct mutex m_lock;
int *global_buffer = NULL;

//Declarations
int kern_thread (void *args);
static int __init kthread_init(void);
static void __exit kthread_exitt(void);

#endif
