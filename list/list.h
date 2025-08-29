#ifndef MAUSAM_LIST_H
#define MAUSAM_LIST_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>         // For kernel allocations
#include <linux/list.h>         // For list 
#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

struct metadata {
        int index;
        int data;
        struct list_head list;
};

struct list_head global_list_head;

//Declarations
static int __init list_init(void);
static void __exit list_exit(void);

#endif
