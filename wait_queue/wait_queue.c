#include "wait_queue.h"

//Kernel_thread_func
int kern_thread_1 (void *args) {
	int data = *(int *)args;

	dev_print("Creating thread_1\n");
	while(kthread_should_stop() == 0) {
		wait_event_interruptible(*kern_wait_queue, ((condition %2) != 0));
		dev_print("Thread 1 : counter value - %d\n", condition);
		condition++;
		wake_up_interruptible(kern_wait_queue);
		msleep(2*1000);
	}

	dev_print("Exiting from thread - %d\n", data);
	return 0;
}

int kern_thread_2 (void *args) {
	int data = *(int *) args;
	
	dev_print("Creating thread 2\n");

	while(kthread_should_stop() == 0) {
		wait_event_interruptible(*kern_wait_queue, ((condition %2) == 0));
		dev_print("Thread 2 : counter value - %d\n", condition);
		condition++;
		wake_up_interruptible(kern_wait_queue);
		msleep(2*1000);
	}

	dev_print("Exiting from thread - %d\n", data);
	return 0;
}

static int __init wait_queue_init(void) {
	int status = 0;

	kern_wait_queue = kzalloc(sizeof(*kern_wait_queue), GFP_KERNEL);
	if (kern_wait_queue == NULL) {
		dev_print("Unable to allocate waitqueue object\n");
		status = -ENOMEM;
		goto out;
	}

	//Initialize wait queue
	init_waitqueue_head(kern_wait_queue);

	//Create kthreads
	kthread_1 = kthread_run(kern_thread_1, &thread_data_1, "Kernel Thread 1");
	if (kthread_1 == NULL) {
		dev_print("Unable to create 1st thread\n");
		kfree(kern_wait_queue);
		goto out;
	}

	//msleep(2*1000);
	kthread_2 = kthread_run(kern_thread_2, &thread_data_2, "Kernel Thread 2");
	if (kthread_2 == NULL) {
		dev_print("Unable to create 2nd thread\n");
		kthread_stop(kthread_1);
		kfree(kern_wait_queue);
		goto out;
	}

out:
	return status;
}

static void __exit wait_queue_exit(void) {
	if (kthread_1) {
		kthread_stop(kthread_1);
	}

	if (kthread_2) {
		kthread_stop(kthread_2);
	}

	if(kern_wait_queue) {
		kfree(kern_wait_queue);
	}
	return;
}

module_init(wait_queue_init);
module_exit(wait_queue_exit);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Waitqueue program");
