include <linux/sched.h>        // Set task states
#include <linux/kernel.h>
#include <linux/kthread.h>      // threads related functions
#include <linux/sched.h>        // task structs
#include <linux/delay.h>        // delays
#include <linux/interrupt.h>    // Tasklet operations
#include <linux/slab.h>         // Allocation
#include <linux/list.h>         // List
#include <linux/wait.h>         // Waitqueues

#define dev_print(fmt, ...)     printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

wait_queue_head_t kern_wait_queue;
struct task_struct *kthread_1;
struct task_struct *kthread_2;
int thread_data_1 = 1;
int thread_data_2 = 2;
int condition = 0;

//Function Prototypes
int kern_thread_1 (void *args);
int kern_thread_2 (void *args);
static int __init wait_queue_init(void);
static void __exit wait_queue_exit(void);


//Kernel_thread_func
int kern_thread_1 (void *args) {
	int data = *(int *)args;
	int counter = 0;
	dev_print("Creating thread - 1\n");

	DEFINE_WAIT(wait_entry_node);
	
	while(kthread_should_stop() == 0) {
		dev_print("Processing prepare_to_wait\n");
		prepare_to_wait(&kern_wait_queue, &wait_entry_node, TASK_INTERRUPTIBLE);
		dev_print("prepare_to_wait() is done\n");

		while(condition != 1);
		dev_print("Condtion meet [%d] and counter value is %d\n", condition, counter++);
		condition = 0;
	}

	finish_wait(&kern_wait_queue, &wait_entry_node);
	dev_print("Exiting from thread - %d\n", data);
	return 0;
}

int kern_thread_2 (void *args) {
	int data = *(int *) args;
	
	dev_print("Creating thread - 2\n");

	while(kthread_should_stop() == 0) {
		condition = 1;
		dev_print("From thread 2, setting condition : %d\n", condition);
		wake_up_interruptible(&kern_wait_queue);
		msleep(5*1000);
	}
	
	dev_print("Exiting from thread - %d \n", data);
	return 0;
}

static int __init wait_queue_init(void) {
	int status = 0;

	//Initialize wait queue
	init_waitqueue_head(&kern_wait_queue);

	//Create kthreads
	kthread_1 = kthread_run(kern_thread_1, &thread_data_1, "Kernel Thread 1");
	if (kthread_1 == NULL) {
		dev_print("Unable to create 1st thread\n");
		goto out;
	}

	msleep(2*1000);
	kthread_2 = kthread_run(kern_thread_2, &thread_data_2, "Kernel Thread 2");
	if (kthread_2 == NULL) {
		dev_print("Unable to create 2nd thread\n");
		kthread_stop(kthread_1);
		goto out;
	}

out:
	return status;
}

static void __exit wait_queue_exit(void) {
	dev_print("Calling the exit function....\n");
	
	if (kthread_2) {
                dev_print("Sending signal to 2nd thread\n");
                kthread_stop(kthread_2);
        }

	if (kthread_1) {
		dev_print("Sending signal to 1st thread\n");
		kthread_stop(kthread_1);
	}
	return;
}

module_init(wait_queue_init);
module_exit(wait_queue_exit);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple tasklet program");
