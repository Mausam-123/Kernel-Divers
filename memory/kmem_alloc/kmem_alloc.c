#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>		// For allocations


#define dev_print(fmt, ...)	printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define count			(5)
struct metadata {
	int counter;
	char letter;
};

struct metadata *kmalloc_nodes[count];
struct metadata *kmem_nodes[count];

struct kmem_cache *cache_object;

//Declarations
static int alloc_using_kmalloc(void);
static int alloc_using_kmem(void); 
static int __init alloc_init(void);
static void __exit alloc_exit(void);

static int alloc_using_kmalloc(void) {
	int status  = 0;

	for(int i = 0; i < count; i++) {
		kmalloc_nodes[i] = kmalloc(sizeof(struct metadata), GFP_KERNEL);
		if(kmalloc_nodes[i] == NULL) {
			dev_print("Unable to allocate node : %d via kmalloc() API\n", i);
			status = -ENOMEM;

			//Free all the allocated nodes
			while(--i>=0) {
				kfree(kmalloc_nodes[i]);
			}
			goto out;
		}
		kmalloc_nodes[i]->letter = 'A' + i;
	}

	dev_print("Printing the data from kmalloc nodes \n");
	for(int i = 0; i < count; i++) {
		dev_print("Node[%d] - letter : %c\n", i, kmalloc_nodes[i]->letter);
	}

out:
	return status;	
}

static int alloc_using_kmem(void) {
	int status = 0;

	//Create kmem cache
	cache_object = kmem_cache_create("cache_object", 10*sizeof(struct metadata), 0, SLAB_HWCACHE_ALIGN, NULL);
	if(cache_object == NULL) {
		dev_print("Failed to allocate cache_object\n");
		status = -ENOMEM;
		goto out;
	}

	for(int i=0; i < count; i++) {
		kmem_nodes[i] = kmem_cache_alloc(cache_object, GFP_KERNEL);
		if(kmem_nodes[i] == NULL) {
			dev_print("Unable to allocate node :%d via kmem_cache_alloc\n", i);
			status = -ENOMEM;

			//Free all the allocated nodes
			while(--i>=0) {
				kmem_cache_free(cache_object, kmem_nodes[i]);
			}
			kmem_cache_destroy(cache_object);
			goto out;
		}
		kmem_nodes[i]->counter = (i)*10;
	}

	dev_print("Printing the data from kmem cachce nodes \n");
	for(int i = 0; i < count; i++) {
		dev_print("Node[%d] - data : %d\n", i, kmem_nodes[i]->counter);
	}
out:
	return status;
}

static int __init alloc_init(void) {
	int status = 0;

	dev_print("Module init .....\n");
	status =  alloc_using_kmalloc();
	if(status != 0) {
		goto out;
	}

	status = alloc_using_kmem();
	if(status != 0) {
		goto out;
	}
	dev_print("Module init done !!\n");
out:
	return status;
}

static void __exit alloc_exit(void) {
	// Free all the kmem objects
	int index = count;

	dev_print("Module unloading ...\n");
	while(index-- >= 0) {
		if(kmem_nodes[index] != NULL) {
			kmem_cache_free(cache_object, kmem_nodes[index]);
		}
		kmem_cache_destroy(cache_object);
	}	

	index = count;
	while(index-- >=0) {
		if(kmalloc_nodes[index] != NULL) {
			kfree(kmalloc_nodes[index]);
		}
	}

	dev_print("Module unloading done !!\n");
	return;
}

module_init(alloc_init);	
module_exit(alloc_exit);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Data allocation program");
