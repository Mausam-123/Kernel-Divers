#include <linux/module.h>       // Core header for loading LKMs into the kernel
#include <linux/init.h>         // Macros for init and exit functions
#include <linux/fs.h>           // File operation structures
#include <linux/cdev.h>         // Character device registratio
#include <linux/uaccess.h>      // Copy to/from user
#include <linux/device.h>
#include <linux/err.h>
#include <asm/page.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <linux/slab.h>
#include "ioctl_control.h"

//Macros
#define DEVICE_NAME "my_char_dev"
#define CLASS_NAME "my_char_class"
#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

dev_t dev_num; 
struct cdev char_dev;
int major_num = 0;
int minor_num = 0;
int get_data = 0;
static struct class *my_char_class;
static struct device *my_char_device;
static void *kernel_buf;

//Function prototypes
static int __init my_chardev_init(void);
static void __exit my_chardev_exit(void);
static int char_dev_open (struct inode *inode, struct file *file);
static int char_dev_release(struct inode *inode, struct file *file);
static ssize_t char_dev_read(struct file *file, char __user *user_buf, size_t len, loff_t *offset);
static ssize_t char_dev_write(struct file * file, const char __user *user_buf, size_t len, loff_t *offset);
static int char_dev_mmap(struct file *file, struct vm_area_struct *vm_area);
static loff_t char_dev_llseek(struct file *file, loff_t offset, int type);
long char_dev_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long args);
