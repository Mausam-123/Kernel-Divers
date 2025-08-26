#include "char_cpu_drv.h"

/* open() : When file is opened for the first time, VFS layer intercept the system call
 * Creates struct inode ---> allocate and initialize struct file ----> set fops field with cdev fops
 * struct inode is unique but struct file is per open instance
 */
static int char_dev_open (struct inode *inode, struct file *file){
	printk("Mausam - Opening the file\n");
	return 0;
}

static int char_dev_release(struct inode *inode, struct file *file) {
	printk("Mausam - Closing the file\n");
	return 0;
}

static ssize_t char_dev_read(struct file *file, char __user *user_buf, size_t len, loff_t *offset) {
	printk("Mausam - Invoking the read callback function\n");
	return 1;
 }

static ssize_t char_dev_write(struct file * file, const char __user *user_buf, size_t len, loff_t *offset) {
	printk("Mausam - Invoking the write callback function\n");
	return 1;
}

static int my_cpu_driver_probe(struct device *dev)
{
    printk("Mausam : My cpu driver probe called\n");
    struct cpufreq_policy *policy = cpufreq_cpu_get(0);
    if (!policy) {
        printk("cpufreq policy not found\n");
        return -ENODEV;
    }
    printk("CPU0 frequency at probe: %u kHz\n", policy->cur);
    cpufreq_cpu_put(policy);
    return 0;
}

static int my_cpu_driver_remove(struct device *dev)
{
    printk("Driver remove called\n");
    return 0;
}

static void my_char_device_release(struct device *dev) {
        printk("Mausam - Device release function is getting invoked\n");
        kfree(dev);
        return;
}

//Bus matching function called during device probe
static int char_bus_match (struct device *dev, const struct device_driver *drv) {
        printk("Match status : %d\n", strcmp(dev_name(dev), drv->name));
        return !strcmp(dev_name(dev), drv->name);
}

static const struct bus_type simple_bus = {
        .name = BUS_NAME,
        .match = char_bus_match,
};

static struct file_operations fops = {
	.open = char_dev_open,
	.release = char_dev_release,
	.read = char_dev_read,
	.write = char_dev_write,
};

static int __init my_chardev_init(void) {	
	int ret = 0;
	
	ret = bus_register(&simple_bus);
    	if (ret) {
		return ret;
	}

	//Dynamically allocate major and minor number
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		printk("Mausam - alloc_chrdev_region() FAILED  with error - %d\n", ret);
		return ret;
	}
	
	major_num = MAJOR(dev_num);
	minor_num = MINOR(dev_num);
	printk("Mausam = Major:Minor - %d : %d\n", major_num, minor_num);

	cdev_init(&char_dev, &fops);	// Initailze character driver with fops
	char_dev.owner = THIS_MODULE;
	
	/* Register character driver with the kernel internal character driver framework
	 * This registration links the cdev (character device) to the kernel’s device number system maintained in a device number to cdev mapping, 
	 * enabling the kernel to route file operations on /dev nodes with matching major/minor numbers to the registered cdev's file operations
	 */
	ret = cdev_add(&char_dev, dev_num, 1);
	if (ret < 0) {
		printk("Mausam - cdev_add() FAILED, device is not ready to use");
		goto fail;
		return ret;
	}

	/*
	 * class_create() : Creates sysfs class entries /sys/class, managed by kernel 
	 * It is used by udev for creating entries inside /dev/ during device_create()
	 * sysfs provies kernel objects (like devices, drivers, bus) to which user space can interact with.
	 * sysfs acts as a bridge between user program to interact with the kernel object model.
	 */
	my_char_class = class_create(CLASS_NAME);
	if (IS_ERR(my_char_class)) {
		printk("Mausam - class_create() FAILED, with error - %ld\n", PTR_ERR(my_char_class));
		ret = PTR_ERR(my_char_class);
		goto fail_1;
	} else {
		printk("Mausam - sys/class/%s is created \n", CLASS_NAME);
	}

	//Allocate struct device
	my_char_device = kzalloc(sizeof(*my_char_device), GFP_KERNEL);
	if(my_char_device == NULL) {
		printk("Mausam - Failed to allocate device structure\n");
		goto fail_2;
	}

	//Initialize member variables
	device_initialize(my_char_device);

	my_char_device->release = my_char_device_release;	//Invokes when device registration fails
	my_char_device->devt = dev_num;
	//my_char_device->class = my_char_class;
	my_char_device->bus = &simple_bus;	// Either you set bus or class
	
	//Set common name for both device and driver for bus matching
	dev_set_name(my_char_device, DEVICE_NAME);
 
	/* device_add() : Add a device to bus list of devices and triggers search in registered driver list based on token
	 * If found a driver, corresponding probe is called, here we are registering driver after device_add(), no probe 
	 * will be called, here device will be simply added to bus list
	 */
	ret = device_add(my_char_device);
	if (ret) {
		put_device(my_char_device);
    		printk("Mausam: device_add() FAILED\n");
		goto fail_2;
	} else {
		printk("Mausam - dev/%s is created \n", DEVICE_NAME);
	}
	
	memset(&my_cpu_driver, 0, sizeof(struct device_driver));
	my_cpu_driver.name = DEVICE_NAME;
	my_cpu_driver.bus = &simple_bus;
	my_cpu_driver.owner = THIS_MODULE;
	my_cpu_driver.probe = my_cpu_driver_probe;
	my_cpu_driver.remove = my_cpu_driver_remove; 
	
	/* driver_register() : Register a device driver with kernel device model and it to global driver list, linking it to specific bus type
	 * When driver is registered kernel iterates over bus list device and execute probe upon matching
	 */
	ret = driver_register(&my_cpu_driver);
	if(ret != 0) {
		printk("Mausam - driver_register() FAILED with error - %d\n", ret);
		goto fail_3;
	} else {
		printk("Mausam - driver registration success\n");
	}
	
	return 0;

fail_3:
	device_unregister(my_char_device);
fail_2:
	class_destroy(my_char_class);
fail_1:
	cdev_del(&char_dev);
fail:
	unregister_chrdev_region(dev_num, 1);
	bus_unregister(&simple_bus);
	return ret;
}

static void __exit my_chardev_exit(void) {
	driver_unregister(&my_cpu_driver);
	device_unregister(my_char_device);
	class_destroy(my_char_class);
	cdev_del(&char_dev);
	unregister_chrdev_region(dev_num, 1);
	bus_unregister(&simple_bus);
	printk("Mausam - Unloading the kernel module\n");
	return;
}

module_init(my_chardev_init);	
module_exit(my_chardev_exit);

MODULE_AUTHOR("Mausam");
MODULE_DESCRIPTION("A simple character driver");
MODULE_LICENSE("GPL");
