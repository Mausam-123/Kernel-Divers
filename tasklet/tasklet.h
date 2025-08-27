#ifndef MY_TASKLET_H
#define MY_TASKLET_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>      // threads related functions
#include <linux/sched.h>        // task structs
#include <linux/delay.h>        // delays
#include <linux/smp.h>          // CPU #
#include <linux/interrupt.h>    // Tasklet operations

#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

struct tasklet_struct *tasklet;
struct task_struct *kthread;
int kern_data = 1;
int tasklet_data = 1;

//Prototypes
static int __init task_init(void);
int kern_thread(void *args);
void tasklet_handler(unsigned long data);
static void __exit task_exit(void);

#endif
