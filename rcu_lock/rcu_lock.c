#include "rcu_lock.h"

int reader_threads (void *args) {
	int thread_nr = *(int *) args;

	while(kthread_should_stop() == 0) {
		struct rcu_metadata *ptr = NULL;
		rcu_read_lock();
		ptr = rcu_dereference(rcu_data);
		if(ptr != NULL) {	
			dev_print("Reader thread %d, reads counter value : %d\n", thread_nr, ptr->counter);
		}
		rcu_read_unlock();
		msleep(1*1000);
	}
	
	dev_print("Reader Thread %d is exiting...\n", thread_nr);
	return 0;
}

void rcu_free_data_callback(struct rcu_head *data_head) {
	struct rcu_metadata *old_data = container_of(data_head, struct rcu_metadata, rcu_head);
	
	if (old_data) {
		kfree(old_data);
		old_data = NULL;
	}
	dev_print("Old data freed in rcu callback function\n");
	return;
}

int writer_thread (void *args) {
	int thread_nr = *(int *) args;

	while(kthread_should_stop() == 0) {
		struct rcu_metadata *old_data = NULL;
		struct rcu_metadata *new_data = NULL;

		new_data = kzalloc(sizeof(*new_data), GFP_KERNEL);
		if (new_data == NULL) {
			dev_print("Failed to allocate new data\n");
			return -ENOMEM;
		}

		new_data->counter = ++global_index;
		old_data = rcu_dereference_protected(rcu_data, 1);
		rcu_assign_pointer(rcu_data, new_data);

		synchronize_rcu();	// Let all the reader thread finsih reading the older data

		dev_print("Writer thread %d, updated counter value : %d\n", thread_nr, new_data->counter);

		if(old_data) {
			call_rcu(&rcu_data->rcu_head, rcu_free_data_callback);
		}
		msleep(5*1000);
	}

	dev_print("Writer %d is exiting...\n", thread_nr);
	return 0;
}

static int __init kthread_init(void) {
	dev_print("Module init ...\n");

	rcu_data = kzalloc(sizeof(*rcu_data), GFP_KERNEL);
	if (rcu_data == NULL) {
		dev_print("rcu_data allocation fails\n");
		goto cleanup;
	}

	rcu_data->counter = global_index; 

	for(int i  = 0; i < 2; i++) {
		reader_td[i] = kthread_run(reader_threads, &reader_id[i], "Reader_Thread_%d", (i+1));
		if(reader_td[i] == NULL) {
			dev_print("Unable to create reader thread - %d\n", reader_id[i]);
			goto cleanup;
		} else {
			dev_print("Reader thread[%d] - %d created successfully\n", reader_td[i]-> pid, reader_id[i]);
		}
	}

	//khread_run() : Creates kernel thread and start its execution
	writer_td = kthread_run(writer_thread, &writer_id, "writer_Thread");
	if(writer_td != NULL) {
		dev_print("Writer thread[%d]created successfully\n", writer_td->pid);
	} else {
		dev_print("Unable to create writer thread\n");
		goto cleanup;		// Send signal to first thread to stop its execution
	}

	dev_print("Module init done !!\n");
	
	return 0;
cleanup:
	for(int i=0; i < 2; i++) {
		if (reader_td[i] != NULL) {
			kthread_stop(reader_td[i]);
			reader_td[i] = NULL;
		}
	}

	if (writer_td != NULL) {
		kthread_stop(writer_td);
		writer_td = NULL;
	}

	if (rcu_data) {
		kfree(rcu_data);
	}

	dev_print("Module init failed !!\n");
	return -1;
}

static void __exit kthread_exitt(void) {
	dev_print("Module unloading ....\n");
	for(int i=0; i < 2; i++) {
                if (reader_td[i] != NULL) {
                        kthread_stop(reader_td[i]);
                        reader_td[i] = NULL;
                }
        }

        if (writer_td != NULL) {
                kthread_stop(writer_td);
                writer_td = NULL;
        }

        if (rcu_data) {
                kfree(rcu_data);
        }

	dev_print("Module unloading done\n");
	return;
}

module_init(kthread_init);	
module_exit(kthread_exitt);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Data sync using RCU lock");
