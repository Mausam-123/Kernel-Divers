#include "key_signal.h"

static int key_device_open (struct inode *inode, struct file *file) {
	dev_print("Calling the driver open callback function\n");
	return 0;
}

static int key_device_release(struct inode *inode, struct file *file) {
	dev_print("Calling the driver release callback function\n");
	return 0;
}

static ssize_t key_device_read(struct file *file, char __user *user_buf, size_t count, loff_t *offset) {
	char val;
	int status = 1;

	dev_print("Calling the driver read callback function\n");
	if (count < 1) {
        	status = -EINVAL;
		goto label;
	}

    	val = alt_key_pressed ? '1' : '0';

	status = copy_to_user(user_buf, &val, 1);
	if (status) {
        	dev_print("copy_to_user(), FAILED with status : %d\n", status);
	} else {
		dev_print("Copied '%c'(1:Alt pressed, 0: Others) to user space buffer\n", val);
	}
label:
    	return status;
}

unsigned int key_device_poll(struct file *file, struct poll_table_struct *key_wait) {
	unsigned int mask = 0;

	dev_print("Calling the driver poll callback function\n");
	//Add "key_wait" entry in "key_waitqueue"
	poll_wait(file, key_waitqueue, key_wait);

#if 0	// Not to be used in poll
	/* After wakeup, check ALT key is pressed or not if not, then wait queue infrastructure 
	 * will put current task into same "key_waitqueue" while CPU is free to execute other tasks
	 * when CPU executes this task again, it will check "alt_key_press" condition if fasle, it
	 * will continue to schedule
	 */
	while (alt_key_pressed == 0) {
    		schedule();
	}
#endif
	if (alt_key_pressed) {	
		dev_print("ALT key is pressed\n");
		mask |= POLLIN | POLLRDNORM;
	}
	return mask;
}
static struct file_operations fops = {
	.open = key_device_open,
	.release = key_device_release,
	.read = key_device_read,
	.poll = key_device_poll
};

// Input event handler for keyboard ALT keys
static void key_alt_input_event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
	if (type == EV_KEY && (code == KEY_LEFTALT || code == KEY_RIGHTALT)) {
		bool prev_state = alt_key_pressed;
		alt_key_pressed = (value == 1) ? 1 : 0;

 		if (alt_key_pressed && !prev_state) {
			// Send a signal to wakeup sleeping tasks in "key_waitqueue"
			wake_up_interruptible(key_waitqueue);
			dev_print("ALT key pressed, waking up wait queue\n");
		}
	}
}

// Input connect: called when the input device matches
static int key_input_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id)
{
	struct input_handle *handle;
	int status;
    
	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle) {
		status = -ENOMEM;
		goto label;
	}

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "alt_key_handle";

	status = input_register_handle(handle);
	if (status != 0) {
 		goto label_1;
	}

	status = input_open_device(handle);
	if (status) {
		goto label_2;
	}

	handler->private = handle;
	dev_print("Key input handler connected to device: %s\n", dev_name(&dev->dev));
	return 0;

label_2:
	input_unregister_handle(handle);
label_1:
	kfree(handle);
label:
	return status;
}

// Input disconnect
static void key_input_disconnect(struct input_handle *handle)
{
        dev_print("Key input handler disconnected from device: %s\n", dev_name(&handle->dev->dev));

        input_close_device(handle);
        input_unregister_handle(handle);
        kfree(handle);
}

// Input device IDs: match all keyboards
static const struct input_device_id key_input_ids[] = {
	{ .driver_info = 1, .evbit = { BIT_MASK(EV_KEY) }, },
	{ },
};

static struct input_handler key_input_handler = {
	.event = key_alt_input_event,
	.connect = key_input_connect,
	.disconnect = key_input_disconnect,
	.name = "key_input_handler",
	.id_table = key_input_ids,
};

static int __init key_device_init(void) {
	int status = 0;
	
	// Allocate and initialize wait queue dynamically
	key_waitqueue = kmalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
	if (!key_waitqueue) {
        	dev_print("Failed to allocate wait queue\n");
        	status = -ENOMEM;
		goto label_0;
    	}

	init_waitqueue_head(key_waitqueue);

	status = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if (status < 0) {
		dev_print("alloc_chrdev_region() FAILED  with error - %d\n", status);
		goto label;
        } else {
                dev_print("alloc_chrdev_region() PASSED  with status - %d\n", status);
        }

	dev_print("Major:Minor = %d : %d\n", MAJOR(dev_num), MINOR(dev_num));
	cdev_init(&char_key_device, &fops);
	char_key_device.owner = THIS_MODULE;
	
	status = cdev_add(&char_key_device, dev_num, 1);
        if (status < 0) {
		dev_print("cdev_add() FAILED, with status : %d", status);
		goto label_1;
	}

	key_class = class_create(CLASS_NAME);
        if (IS_ERR(key_class)) {
                dev_print("class_create() FAILED, with error - %ld\n", PTR_ERR(key_class));
                status = PTR_ERR(key_class);
                goto label_2;
        } else {
                dev_print("sys/class/%s is created \n", CLASS_NAME);
        }

	key_device = device_create(key_class, NULL, dev_num, NULL, DEVICE_NAME);
        if(IS_ERR(key_device)) {
                dev_print("device_create() FAILED, with error - %ld\n", PTR_ERR(key_device));
                status = PTR_ERR(key_device);
                goto label_3;
        } else {
                dev_print("dev/%s is created \n", DEVICE_NAME);
        }

	// Register input handler to receive keyboard events
	status = input_register_handler(&key_input_handler);
	if(status) {
		dev_print("input_register_handler() FAILED, with status - %d\n", status);
		goto label_4;
	} else {
		dev_print("key handler driver registered\n");
	}

	return 0;
label_4:
	device_destroy(key_class, dev_num);
label_3:
        class_destroy(key_class);
label_2:
        cdev_del(&char_key_device);
label_1:
	unregister_chrdev_region(dev_num, 1);
label:
	kfree(key_waitqueue);
label_0:
	return status;
}

static void __exit key_device_exit(void) {
	input_unregister_handler(&key_input_handler);
	device_destroy(key_class, dev_num);
 	class_destroy(key_class);
	cdev_del(&char_key_device);
	unregister_chrdev_region(dev_num, 1);
	kfree(key_waitqueue);
	dev_print("Unloading the kernel module\n");
	return;
}

module_init(key_device_init);
module_exit(key_device_exit);

MODULE_AUTHOR("Mausam");
MODULE_DESCRIPTION("Trap a keyboard Input");
MODULE_LICENSE("GPL");
