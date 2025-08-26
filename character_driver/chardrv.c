#include "chardrv.h"

/* open() : When file is opened for the first time, VFS layer intercept the system call
 * Creates struct inode ---> allocate and initialize struct file ----> set fops field with cdev fops
 * struct inode is unique but struct file is per open instance
 */
static int char_dev_open (struct inode *inode, struct file *file){
	dev_print("Opening the file\n");
	return 0;
}

static int char_dev_release(struct inode *inode, struct file *file) {
	dev_print("Closing the file\n");
	return 0;
}

static ssize_t char_dev_read(struct file *file, char __user *user_buf, size_t len, loff_t *offset) {
 	int not_copy=0;
	int delta = 0;
	int to_copy = 0;

	dev_print("Invoking the read callback function\n");
	to_copy = (len + *offset) < PAGE_SIZE ?  len : (PAGE_SIZE - *offset);
	
	if (*offset >= PAGE_SIZE) {
		dev_print("EOF is reached, offset : %lld and buf size : %ld\n", *offset, PAGE_SIZE);
		return 0;
	}

	not_copy = copy_to_user(user_buf, kernel_buf + *offset, to_copy);
	delta = to_copy - not_copy;
	*offset += delta;
	if(not_copy != 0) {
		dev_print("Expected read bytes : %ld, but only read : %d, current offset : %lld\n", len, delta, *offset);
	} else {
		dev_print("Expected read bytes : %ld, read all %d bytes, current offset : %lld\n", len, to_copy, *offset);
	}
	return delta;
 }

static ssize_t char_dev_write(struct file * file, const char __user *user_buf, size_t len, loff_t *offset) {
	int not_copy = 0;
	int delta = 0;
	int to_copy = 0;
	
	dev_print("Invoking the write callback function\n");
	to_copy = (len + *offset) < PAGE_SIZE? len : (PAGE_SIZE - *offset);

	if(*offset >= PAGE_SIZE) {
		dev_print("EOF is reached, offset : %lld and buf size : %ld\n", *offset, PAGE_SIZE);
		return 0;
	}

	not_copy = copy_from_user(kernel_buf + *offset, user_buf, to_copy);
	delta = to_copy - not_copy;
	*offset += delta;
	if(not_copy != 0) {
		dev_print("Expected write bytes : %ld, but only wrriten : %d, current offset : %lld\n", len, delta, *offset);
        } else {
		dev_print("Expected write bytes : %ld, written all %d bytes, current offset : %lld\n", len, to_copy, *offset);
	}
	
	return delta;
}

static int char_dev_mmap(struct file *file, struct vm_area_struct *vm_area) {
	int status = 0;
	
	dev_print("Invoking the mmap function call\n");
	
	/* virt_to_phy() : Get the physical page frame number
	 * kernel_buf is mapped to kernel address space, we need to get actual physical memory for mapping it into user space VM
         * i.e kernel VM ---> Physical Memory ----> User Space VM
	 */
	vm_area->vm_pgoff = virt_to_phys(kernel_buf) >> PAGE_SHIFT;
	
	/* vm_start and vm_end is already decided by the Linux MMU based on size agrument of user space mmap() call,
	 * Need to check kernel has allocated appropriate virtual memory for mapping or not
	 */
	if ((vm_area->vm_end - vm_area->vm_start) > PAGE_SIZE) {
		dev_print("Allowed allocation 1 page, but user has requested more then 1 pages\n");
		return -ENOMEM;
	}
	
	/* remap_pfn_range() : Map the physical page frame number into user virtual address space
	 * As vm_start and vm_end is already decided, map allocated PFN to this range
	 */
	status = remap_pfn_range(vm_area, vm_area->vm_start, vm_area->vm_pgoff, vm_area->vm_end - vm_area->vm_start, vm_area->vm_page_prot);
	
	if (status != 0) {
		dev_print("remap_pfn_range() FAILED to map page size : %ld\n", PAGE_SIZE);
		return -ENOMEM;
	}

	dev_print("vma_start: 0x%lx, vma_size: 0x%lx\n", vm_area->vm_start, vm_area->vm_end - vm_area->vm_start);
	return 0;
}

static loff_t char_dev_llseek(struct file *file, loff_t offset, int type) {
	loff_t position = 0;

	dev_print("Invoking the llseek callback function\n");
	switch(type) {
		//SEEK_SET — set position to offset
		case SEEK_SET:
			position = offset;
			break;
		//SEEK_CUR — set position to current position + offset
		case SEEK_CUR:
			position = file->f_pos + offset;
			break;
		//SEEK_END — set position relative to the end (if applicable, e.g., size of device data)
		case SEEK_END:
			position = PAGE_SIZE + offset;
			break;
		default:
			printk("Mausam(llseek) : Invalid llseek type\n");
			return -EINVAL;
	}

	if (position < 0 || position > PAGE_SIZE) {
		dev_print("Invalid Postion (%lld) llseek range: [0, (%ld)]\n", position, PAGE_SIZE);
		return -EINVAL;
	}

	file->f_pos = position;
	return position;
}

long char_dev_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long args) {
	int status  = 0;
	struct user_metadata metadata;


	//Allow only specific magic number operation
	if (_IOC_TYPE(cmd) != IOCTL_MAGIC) {
		dev_print("Invalid magic number %u\n", cmd);
		status = -EOPNOTSUPP;
		goto label;
	}

	switch(cmd) {
		case IOCTL_RD_DATA:
			status = copy_to_user((int *) args, &get_data, sizeof(int));
			if (status) {
				dev_print("Unable to write %d bytes\n", status);
				goto label;
			}
			dev_print("Kerne wrote %d into user buffer\n", get_data);
			break;
		case IOCTL_WR_DATA:
			status = copy_from_user(&get_data, (int *) args, sizeof(int));
			if(status) {
				dev_print("Unable to copy %d bytes\n", status);
				goto label;
			}
			dev_print("User wrote %d in kernel buffer\n", get_data);
			break;
		case IOCTL_WR_METADATA:
			status = copy_from_user(&metadata, (struct user_metadata *)args, sizeof(metadata));
			if (status) {
				dev_print("Unable for copy %d of metadata\n", status);
				goto label;
			}
			for(int i =0; i < metadata.count; i++) {
				dev_print("%s\n", metadata.name);
			}
			break;
		default:
			dev_print("Invalid command ID - %u\n", cmd);
			status = -EOPNOTSUPP;
	}
label:
	return status;
}

static struct file_operations fops = {
	.open = char_dev_open,
	.release = char_dev_release,
	.read = char_dev_read,
	.write = char_dev_write,
	.mmap = char_dev_mmap,
	.llseek = char_dev_llseek,
	.unlocked_ioctl = char_dev_unlocked_ioctl
};

static int __init my_chardev_init(void) {	
	int ret = 0;
	
	kernel_buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (kernel_buf == NULL) {
		dev_print("Failed to allocate %ld bytes\n", PAGE_SIZE);
		return -ENOMEM;
	}

	//Dynamically allocate major and minor number
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		dev_print("alloc_chrdev_region() FAILED  with error - %d\n", ret);
		return ret;
	} else {
		dev_print("alloc_chrdev_region() PASSED  with status - %d\n", ret);
	}
	
	major_num = MAJOR(dev_num);
	minor_num = MINOR(dev_num);
	dev_print("Major:Minor = %d : %d\n", major_num, minor_num);

	cdev_init(&char_dev, &fops);	// Initailze character driver with fops
	char_dev.owner = THIS_MODULE;
	
	/* Register character driver with the kernel internal character driver framework
	 * This registration links the cdev (character device) to the kernel’s device number system maintained in a device number to cdev mapping, 
	 * enabling the kernel to route file operations on /dev nodes with matching major/minor numbers to the registered cdev's file operations
	 */
	ret = cdev_add(&char_dev, dev_num, 1);
	if (ret < 0) {
		dev_print("cdev_add() FAILED, device is not ready to use");
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
		dev_print("class_create() FAILED, with error - %ld\n", PTR_ERR(my_char_class));
		ret = PTR_ERR(my_char_class);
		goto fail_1;
	} else {
		dev_print("sys/class/%s is created \n", CLASS_NAME);
	}

	/*
	 * device_create() : Creates stuct device object and register it with the kernel device model
	 * It populates sysfs entries and link with the /sys/device is parent is defined
	 * Afte that it triggers the KOBJ_ADD uevent to udev demon, which inturn creates entries inside /dev/<name>
	 * devfs is a virtual filesystem which provies the device nodes for file-operations like read, write, etc and devfs is managed by VFS layer.
	 * devfs can be thought of IO nodes to interact with the underneath device.
	 */
	my_char_device = device_create(my_char_class, NULL, dev_num, NULL, DEVICE_NAME);
	if(IS_ERR(my_char_device)) {
		dev_print("device_create() FAILED, with error - %ld\n", PTR_ERR(my_char_device));
		ret = PTR_ERR(my_char_device);
		goto fail_2;
	} else {
		dev_print("dev/%s is created \n", DEVICE_NAME);
	}
	return 0;

fail_2:
	class_destroy(my_char_class);
fail_1:
	cdev_del(&char_dev);
fail:
	unregister_chrdev_region(dev_num, 1);
	return ret;
}

static void __exit my_chardev_exit(void) {
	device_destroy(my_char_class, dev_num);
	class_destroy(my_char_class);
	cdev_del(&char_dev);
	unregister_chrdev_region(dev_num, 1);
	dev_print("Unloading the kernel module\n");
	return;
}

module_init(my_chardev_init);	
module_exit(my_chardev_exit);

MODULE_AUTHOR("Mausam");
MODULE_DESCRIPTION("A simple character driver");
MODULE_LICENSE("GPL");
