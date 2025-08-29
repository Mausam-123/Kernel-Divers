#include "semaphore.h"

int reader_threads (void *args) {
	int thread_nr = *(int *) args;

	while(kthread_should_stop() == 0) {
		
		//Acquire semaphore lock
		if(down_interruptible(&sem_lock)) {
			dev_print("Thread - %d, interrupted by signal while waiting for lock\n", thread_nr);
			continue;
		}

		dev_print("Thread - %d, reads shares data - %d\n", thread_nr, global_data);

		//Release semaphore lock
		up(&sem_lock); 
		msleep(1*1000);		// Sleep for 1 sec
	}
	
	dev_print("Reader Thread %d is exiting...\n", thread_nr);
	return 0;
}

int writer_thread (void *args) {
	int thread_nr = *(int *) args;

	while(kthread_should_stop() == 0) {
		//Acquire all the lock
		for(int i = 0; i < owner_count; i++) {
			if(down_interruptible(&sem_lock)) {
				dev_print("Writer thread - %d interrupted by signal while acquring lock\n", thread_nr);
				//Release all the acquired lock so that reader can continue
				while(--i>=0){
					up(&sem_lock);
				}
			}
		}

		++global_data;
		dev_print("Writer thread %d, updated shared data : %d\n", thread_nr, global_data);

		//Release all held locks
		for(int i =0; i <owner_count; i++) {
			up(&sem_lock);
		}
		msleep(5*1000);		// Update after 5 sec
	}

	dev_print("Writer %d is exiting...\n", thread_nr);
	return 0;
}

static int __init kthread_init(void) {
	dev_print("Module init ...\n");


	//Initialize semaphore
	sema_init(&sem_lock, owner_count);

	for(int i  = 0; i < read_thread_count; i++) {
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
		goto cleanup;
	}

	dev_print("Module init done !!\n");
	
	return 0;
cleanup:
	for(int i=0; i < read_thread_count; i++) {
		if (reader_td[i] != NULL) {
			kthread_stop(reader_td[i]);
			reader_td[i] = NULL;
		}
	}

	if (writer_td != NULL) {
		kthread_stop(writer_td);
		writer_td = NULL;
	}

	dev_print("Module init failed !!\n");
	return -1;
}

static void __exit kthread_exitt(void) {
	dev_print("Module unloading ....\n");
	for(int i=0; i < read_thread_count; i++) {
                if (reader_td[i] != NULL) {
                        kthread_stop(reader_td[i]);
                        reader_td[i] = NULL;
                }
        }

        if (writer_td != NULL) {
                kthread_stop(writer_td);
                writer_td = NULL;
        }

	dev_print("Module unloading done\n");
	return;
}

module_init(kthread_init);	
module_exit(kthread_exitt);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Data sync using semphore lock");
