#include "spin_lock.h"

//Declarations
int kern_thread (void *args);
static int __init kthread_init(void);
static void __exit kthread_exitt(void);

int kern_thread (void *args) {
	int data = *(int *) args;

	while(kthread_should_stop() == 0) {
		unsigned long flag;
		/* spin_lock_irqsave(spinlock_t *lock, unsigned long flag) : Saves the current IRQ state, disable it locally and acquire the lock
		 * spin_lock_irq(spinlock_t *lock) : Disables all the interrupt locally, don't save the current irq state like above.
		 * spin_lock(spinlock_t *lock) : Does not disable interrupt locally, if critical section is shared with interrupts, it might
		 * cause deadlock
		 */
		spin_lock_irqsave(&s_lock, flag);
		dev_print("Thread %d acquired spin lock the lock\n", data);
		for(int i = 0; i <5; i++) {
			dev_print("%d\n", global_buffer[i]);
		}

		/* spin_unlock_irqrestore(spinlock_t *lock, unsigned long flag) : Release lock and resume IRQ handling
		 * spin_unlock_irq(spinlock_t *lock) : Release lock and enable all the interrupts on local core, even disabled interrupts will be enabled.
		 * spin_unlock(spinlock_t *lock) : Release spinlock
		 */
		spin_unlock_irqrestore(&s_lock, flag);
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
	spin_lock_init(&s_lock);

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
