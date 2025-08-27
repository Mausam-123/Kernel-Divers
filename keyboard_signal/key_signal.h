#ifndef KEY_SIGNAL_H
#define KEY_SIGNAL_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/cdev.h>

#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CLASS_NAME	"key_class"
#define DEVICE_NAME	"key_device"

dev_t dev_num;
struct cdev char_key_device;
struct device *key_device;
struct class *key_class;

// Dynamically allocated wait queue for ALT key
static wait_queue_head_t *key_waitqueue = NULL;

// ALT key press state flag
static int alt_key_pressed = 0;

//Function prototypes
static int __init key_device_init(void);
static void __exit key_device_exit(void);
static int key_device_open (struct inode *inode, struct file *file);
static int key_device_release(struct inode *inode, struct file *file);
static ssize_t key_device_read(struct file *file, char __user *user_buf, size_t len, loff_t *offset);
unsigned int key_device_poll(struct file *file, struct poll_table_struct *key_wait);

//Keyboard functions
static void key_alt_input_event(struct input_handle *handle, unsigned int type, unsigned int code, int value);
static int key_input_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id);
static void key_input_disconnect(struct input_handle *handle);

#endif
