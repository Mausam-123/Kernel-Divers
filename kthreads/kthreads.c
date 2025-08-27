#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>	// threads related functions
#include <linux/sched.h>	// task structs
#include <linux/delay.h>	// delays

#define dev_print(fmt, ...)	printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)

struct task_struct *kthread_1;
struct task_struct *kthread_2;
int thread_1 = 1;
int thread_2 = 2;

//Declarations
int kern_thread (void *args);

int kern_thread (void *args) {
	int counter = 0;
	int thread_nr = *(int *) args;

	while(kthread_should_stop() == 0) {
		dev_print("Executing the %d thread, counter value : %d\n", thread_nr, counter++);
		msleep(thread_nr * 1000);
	}
	
	dev_print("Thread %d is exiting\n", thread_nr);
	return 0;
}

static int __init kthread_init(void) {
	dev_print("Creating threads\n");
	
	// kthread_create() : Creates Kernel thread but don't start its execution
	kthread_1 = kthread_create(kern_thread, &thread_1, "Kernel_Thread_1");
	if(kthread_1 != NULL) {
		dev_print("Thread 1 with PID : %d is created\n", kthread_1->pid);
		wake_up_process(kthread_1);	// Wake up kthread_1
	} else {
		dev_print("Thread 1 creation fails\n");
		goto label;
	}

	//khread_run() : Creates kernel thread and start its execution
	kthread_2 = kthread_run(kern_thread, &thread_2, "Kernel_Thread_2");
	if(kthread_2 != NULL) {
		dev_print("Thread 2 with PID : %d is created and running\n", kthread_2->pid);
	} else {
		dev_print("Thread 2 creation fails\n");
		kthread_stop(kthread_1);	// Send signal to first thread to stop its execution
	}

	dev_print("Thread creation DONE !!\n");
label:	
	return 0;
}

static void __exit kthread_exitt(void) {
	kthread_stop(kthread_1);
	kthread_stop(kthread_2);
	dev_print("Module exit\n");
	return;
}

module_init(kthread_init);	
module_exit(kthread_exitt);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple kernel threads program");
