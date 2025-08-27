#include "tasklet.h"

//Kernel thread
int kern_thread(void *args) {
	int counter = 0;
	int data = *(int *)args;
	
	while(kthread_should_stop() == 0) {
		tasklet_data++;
		//Update the data, else it will always print intial data set during tasklet_init()
		tasklet->data = (unsigned long)tasklet_data; 
		dev_print("Scheduling the tasklet - %d\n", counter++);

		//Schedule the tasklet
		tasklet_schedule(tasklet);
		msleep(2*1000);
	}

	dev_print("Exiting the thread - %d\n", data);
	return 0;
}

//Tasklet handler
void tasklet_handler(unsigned long data) {
	int cpu_nu = 0;
#if 0
	status = tasklet_trylock(tasklet);
	if(status == 0) {
		dev_print("Current tasklet is already running on other CPU\n");
		return;
	} else {
		cpu_nu = smp_processor_id();
		dev_print("Tasklet is running on cpu core - %d\n", cpu_nu);
	}
#endif
	cpu_nu = smp_processor_id();		
	dev_print("User has written : %ld, tasklet is running on %d cpu code\n", data, cpu_nu);
	
	tasklet_unlock(tasklet);
#if 0
	if(test_bit(TASKLET_STATE_SCHED, &tasklet->state)) {
		dev_print("Tasklet were re-scheduled during execution\n");
	}
#endif
	return;
}

static int __init task_init(void) {
	int status = 0;

	//Allocate the tasklet structure
	tasklet = kzalloc(sizeof(*tasklet), GFP_KERNEL);
	if (tasklet == NULL) {
		status = -ENOMEM;
		goto out;
	}
	
	//Initialize tasklet with handler and initial data
	tasklet_init(tasklet, tasklet_handler, tasklet_data);

	//Create kernel thread
	kthread = kthread_create(kern_thread, &kern_data, "Kernel Thread");
	if (kthread != NULL) {
		dev_print("Wake up kthread\n");
		wake_up_process(kthread);     // Wake up kthread
		dev_print("Kthread pid - %d is running\n", kthread->pid);
	} else {
		dev_print("Kthread cannot be created\n");
		tasklet_kill(tasklet);
		kfree(tasklet);
	}
out:
	return 0;
}

static void __exit task_exit(void) {
	if (kthread) {
		kthread_stop(kthread);
	}

	tasklet_kill(tasklet);

	if(tasklet) {
		kfree(tasklet);
	}
	
	dev_print("All resources are freed, unloading the module\n");
	return;
}

module_init(task_init);      
module_exit(task_exit);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple tasklet program");

