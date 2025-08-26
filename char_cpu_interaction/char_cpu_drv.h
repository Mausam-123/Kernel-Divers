#include <linux/module.h>       // Core header for loading LKMs into the kernel
#include <linux/init.h>         // Macros for init and exit functions
#include <linux/fs.h>           // File operation structures
#include <linux/cdev.h>         // Character device registratio
#include <linux/uaccess.h>      // Copy to/from user
#include <linux/device.h>
#include <linux/err.h>
#include <linux/cpufreq.h>

//Macros
#define DEVICE_NAME "simple_char_dev"
#define CLASS_NAME "simple_char_class"
#define BUS_NAME "simple_char_bus"
#define dev_print(fmt, ...) 	printk(KERN_DEFAULT "Mausam : " fmt, ##__VA_ARGS__)

dev_t dev_num;
struct cdev char_dev;
int major_num = 0;
int minor_num = 0;
static struct class *my_char_class;
static struct device *my_char_device;
static struct device_driver my_cpu_driver;

//Function prototypes
static int __init my_chardev_init(void);

static void __exit my_chardev_exit(void);

static int char_dev_open (struct inode *inode, struct file *file);

static int char_dev_release(struct inode *inode, struct file *file);

static int char_bus_match (struct device *dev,
				const struct device_driver *drv);

static void my_char_device_release(struct device *dev);

static int my_cpu_driver_probe(struct device *dev);

static int my_cpu_driver_remove(struct device *dev);

