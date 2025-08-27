#ifndef SOFTIRQ_H
#define SOFTIRQ_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/softirq.h>

#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)
//#define SOFTIRQ_ID 20

struct softirq_action s_action;
struct task_struct *kthread;
int thread_id = 0;
int counter = 0;

void irq_action(void);
int kern_thread(void *args);
static int __init softirq_start(void);
static void __exit softirq_end(void);
#endif
