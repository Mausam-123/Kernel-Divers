#include "softirq.h"

void irq_action(void) {
	dev_print("Handler global data : %d\n", counter);
	return;
}

int kern_thread(void *args) {
	int data = *(int *)args;

	while(kthread_should_stop() == 0) {
		counter++;

		//Raise softriq to print counter variable
		raise_softirq(HI_SOFTIRQ);
	}

	dev_print("Exiting from thread - %d\n", data);
	return 0;
}

static int __init softirq_start(void) {
	int status = 0;

	dev_print("Softirq module init...\n");

	//Create a kernel thread
	kthread = kthread_run(kern_thread, &thread_id, "SoftIrq_kernel_thread");
	if(kthread == NULL) {
		dev_print("Unable to create kernel thread\n");
		goto out;
	}

	//Register SoftIRQ
	open_softirq(HI_SOFTIRQ, irq_action);
	dev_print("Softirq module init done !!\n");
out:
	return status;
	
}

static void __exit softirq_end(void) {
	dev_print("SoftIRQ module exit...\n");

	if(kthread) {
		kthread_stop(kthread);
	}

	dev_print("SoftIRQ module exit done !!\n");
	return;
}

module_init(softirq_start);
module_exit(softirq_end);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Softirq program");
