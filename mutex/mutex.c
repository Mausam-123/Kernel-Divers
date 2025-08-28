#include "mutex.h"

//Declarations
int kern_thread (void *args);
static int __init kthread_init(void);
static void __exit kthread_exitt(void);

int kern_thread (void *args) {
	int data = *(int *) args;

	while(kthread_should_stop() == 0) {
		/* mutex_lock(struct mutex *mutex) : Acquire mutex lock - blocking and uninterruptible
		 * 
		 * mutex_trylock(struct mutex *mutex) : It tries to acquire lock, if succeed return 1, else return 0.
		 * userful for the user who wanted to try with mutex lock, else take alternative path - It is non-blocking and uninterruptible
		 * 
		 * mutex_lock_interruptible(struct mutex *mutex) : If lock is unavailable then put task to sleeping queue
		 * if signal arrives during sleep, return -EINTR - it is interruptible
		 */
		mutex_lock(&m_lock);
		dev_print("Thread %d acquired the lock\n", data);
		for(int i = 0; i <5; i++) {
			dev_print("%d\n", global_buffer[i]);
		}
		mutex_unlock(&m_lock);
		msleep(5 * 1000);
	}
	
	dev_print("Thread %d is exiting\n", data);
	return 0;
}

static int __init kthread_init(void) {
	int status = 0;

	global_buffer = kzalloc(5*sizeof(int), GFP_KERNEL);
	if (global_buffer == NULL) {
		dev_print("Failed to allocate global buffer\n");
		status = -ENOMEM;
		goto out;
	} else {
		for (int i = 0; i < 5; i++) {
			global_buffer[i] = i;
		}
	}	
	
	//Initialize mutex lock
	mutex_init(&m_lock);

	// kthread_create() : Creates Kernel thread but don't start its execution
	kthread_1 = kthread_run(kern_thread, &thread_1, "Kernel_Thread_1");
	if(kthread_1 != NULL) {
		dev_print("Thread 1 with PID : %d is created and running\n", kthread_1->pid);
	} else {
		dev_print("Thread 1 creation fails\n");
		kfree(global_buffer);
		goto out;
	}

	//khread_run() : Creates kernel thread and start its execution
	kthread_2 = kthread_run(kern_thread, &thread_2, "Kernel_Thread_2");
	if(kthread_2 != NULL) {
		dev_print("Thread 2 with PID : %d is created and running\n", kthread_2->pid);
	} else {
		dev_print("Thread 2 creation fails\n");
		kthread_stop(kthread_1);	// Send signal to first thread to stop its execution
		kfree(global_buffer);
		status = -1;
	}

	dev_print("Thread creation DONE !!\n");
out:	
	return status;
}

static void __exit kthread_exitt(void) {
	kthread_stop(kthread_1);
	kthread_stop(kthread_2);
	kfree(global_buffer);
	dev_print("Module exit\n");
	return;
}

module_init(kthread_init);	
module_exit(kthread_exitt);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Mutex Lock Implementation");
