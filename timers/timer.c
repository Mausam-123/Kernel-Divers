#include "timer.h"

//Declarations
int sender_threads (void *args);
int receiver_thread (void *args);
void timer_callback(struct timer_list *ticks);
static int __init kthread_init(void);
static void __exit kthread_close(void);

void timer_callback(struct timer_list *ticks) {
	dev_print("Timer callback is called\n");
	//Send a completion signal to the receiver thread
	complete_all(&signal_event);

	//Set exipry time to next 5 sec
	mod_timer(&timer, jiffies + msecs_to_jiffies(5*1000));

	return;
}

int sender_threads (void *args) {
	//Initialize timer
	timer_setup(&timer, timer_callback, 0);

	// Star the timer
	mod_timer(&timer, jiffies + msecs_to_jiffies(5*1000));
	
	while(kthread_should_stop() == 0) {
		msleep(5*1000);	// Sleep for 5 sec
	}

	// Delete the timer and wait for any running handler to complete
	del_timer_sync(&timer);
	
	dev_print("Sender thread is exiting ...\n");
	return 0;
}

int receiver_thread (void *args) {

	while(kthread_should_stop() == 0) {
		// Wait for the completion signal
		wait_for_completion_interruptible(&signal_event);

		// Print the metadata
		dev_print("Read data - %d in receiver thread\n", global_data++);
		
		// Re Initialize the completion variable
		reinit_completion(&signal_event);
	}

	dev_print("Receiver thread is exiting...\n");
	return 0;
}

static int __init kthread_init(void) {
	dev_print("Module init ...\n");

	//Initialize completion variable
	init_completion(&signal_event);

	send_thread = kthread_run(sender_threads, NULL, "Waiter_Thread");
	if(send_thread == NULL) {
		dev_print("Unable to create sender thread\n");
		goto cleanup;
	} else {
		dev_print("Sender thread[%d] created successfully\n", send_thread->pid);
	}

	rcv_thread = kthread_run(receiver_thread, NULL, "updater_Thread");
	if(rcv_thread != NULL) {
		dev_print("Writer thread[%d] created successfully\n", rcv_thread->pid);
	} else {
		dev_print("Unable to create receiver thread\n");
		goto cleanup;
	}

	dev_print("Module init done !!\n");
	
	return 0;
cleanup:
	if (send_thread != NULL) {
		kthread_stop(send_thread);
		send_thread = NULL;
	}

	if (rcv_thread != NULL) {
		kthread_stop(rcv_thread);
		rcv_thread = NULL;
	}

	dev_print("Module init failed !!\n");
	return -1;
}

static void __exit kthread_close(void) {
	dev_print("Module unloading ....\n");
	
	if (send_thread != NULL) {
		kthread_stop(send_thread);
		send_thread = NULL;
	}

        if (rcv_thread != NULL) {
                kthread_stop(rcv_thread);
                rcv_thread = NULL;
        }

	dev_print("Module unloading done\n");
	return;
}

module_init(kthread_init);	
module_exit(kthread_close);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Data updation using kernel timer");
