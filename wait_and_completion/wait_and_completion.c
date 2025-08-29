#include "wait_and_completion.h"

int waiter_threads (void *args) {
	int thread_nr = *(int *) args;

	while(kthread_should_stop() == 0) {
		/* wait_for_completion() : Blocking, non -interruptible
		 * wait_for_completion_timeout : Blocking till timeout, non -interruptible, if timeout expires it returns 0 else it returns +ve value
		 * wait_for_completion_interruptible() : Return 0 on success, -ERESTARTSYS if interrupted by signal
		 * wait_for_completion_interruptible_timeout() : Interruptible with timeout
		 */
		wait_for_completion_interruptible(&comp_var);
		//dev_print("sinal received by waiter thread[%d] - %d %d %d %d\n", thread_nr, global_data[0], global_data[1], global_data[2], global_data[3]);
		if (global_data[thread_nr-1] == thread_nr) {
			dev_print(" Waiter Thread %d received signal\n", thread_nr);
			// Reset the global data
			global_data[thread_nr-1] = 0;
		} 	
	}
	
	dev_print("Waiter Thread %d is exiting, current vote value : %d ...\n", thread_nr, atomic_read(&vote));
	atomic_dec(&vote);
	return 0;
}

int updater_thread (void *args) {
	int thread_nr = *(int *) args;

	while(kthread_should_stop() == 0) {
		for(int i = 0; i < 4; i++) {
			global_data[i] = i+1;
		}
		dev_print("Wake up all the waiter thread - %d %d %d %d\n", global_data[0], global_data[1], global_data[2], global_data[3]);
		/* complete() : Signal one thread that event is complete
		 * complete_all() : Signal all the thread that signal is complete, set done var to INT_MAX
		 */
		complete_all(&comp_var);

		// Exit when all the consumer exits - rare case
		if (atomic_read(&vote) == 0) {
			dev_print("Votes received from all threads waiter threads for exit\n");
			break;
		}
		msleep(5*1000);		// Update after 5 sec
	}

	dev_print("Updater %d is exiting, current vote value : %d...\n", thread_nr, atomic_read(&vote));
	return 0;
}

static int __init kthread_init(void) {
	dev_print("Module init ...\n");

	//Initialize completion variable
	init_completion(&comp_var);

	// Initialize atomic variable
	atomic_set(&vote, 4);

	for(int i  = 0; i < waiter_thread_count; i++) {
		waiter_td[i] = kthread_run(waiter_threads, &waiter_id[i], "Waiter_Thread_%d", (i+1));
		if(waiter_td[i] == NULL) {
			dev_print("Unable to create waiter thread - %d\n", waiter_id[i]);
			goto cleanup;
		} else {
			dev_print("Waiter thread[%d] - %d created successfully\n", waiter_td[i]-> pid, waiter_id[i]);
		}
	}

	//khread_run() : Creates kernel thread and start its execution
	updater_td = kthread_run(updater_thread, &updater_id, "updater_Thread");
	if(updater_td != NULL) {
		dev_print("Writer thread[%d]created successfully\n", updater_td->pid);
	} else {
		dev_print("Unable to create writer thread\n");
		goto cleanup;
	}

	dev_print("Module init done !!\n");
	
	return 0;
cleanup:
	for(int i=0; i < waiter_thread_count; i++) {
		if (waiter_td[i] != NULL) {
			kthread_stop(waiter_td[i]);
			waiter_td[i] = NULL;
		}
	}

	if (updater_td != NULL) {
		kthread_stop(updater_td);
		updater_td = NULL;
	}

	dev_print("Module init failed !!\n");
	return -1;
}

static void __exit kthread_close(void) {
	dev_print("Module unloading ....\n");
	for(int i=0; i < waiter_thread_count; i++) {
                if (waiter_td[i] != NULL) {
                        kthread_stop(waiter_td[i]);
                        waiter_td[i] = NULL;
                }
        }

        if (updater_td != NULL) {
                kthread_stop(updater_td);
                updater_td = NULL;
        }

	dev_print("Module unloading done\n");
	return;
}

module_init(kthread_init);	
module_exit(kthread_close);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Data sharing with wait and completion variables");
