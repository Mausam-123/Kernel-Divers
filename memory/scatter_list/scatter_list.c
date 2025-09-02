#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>		// For allocations
#include <linux/mm.h>
#include <linux/scatterlist.h>	// For Scatterlists
#include <linux/gfp.h>

#define dev_print(fmt, ...)	printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define COUNT			(10)

struct page *kern_pages[COUNT];
struct scatterlist kern_sg[COUNT];

//Declarations
static int __init alloc_sg_list(void);
static void __exit dealloc_sg_list(void);

static int __init alloc_sg_list(void) {
	int index = 0;
	dev_print("Module init .....\n");

	//Initialise sg table
	sg_init_table(kern_sg, COUNT);

	//Allocate pages
	for(index =0 ; index < COUNT; index++) {
		kern_pages[index] = alloc_page(GFP_KERNEL);
		if (kern_pages[index] == NULL) {
			dev_print("Failed to allocate the pages\n");
			goto out;
		}

		//Create an entry inside sg table
		sg_set_page(&kern_sg[index], kern_pages[index], PAGE_SIZE, 0);

		// Mark the last page as a valid entry explicitly (optional)
		if(COUNT == 9) {
			sg_mark_end(&kern_sg[index]);
		}
	}
	
	dev_print("Sg table created with %d entries\n", COUNT);

	dev_print("Module init done !!\n");
	return 0;
out:
	while(--index>=0) {
		if (kern_pages[index] != NULL) {
			__free_page(kern_pages[index]);
		}
	}
	return -1;
}

static void __exit dealloc_sg_list(void) {
	int index = 0;
	dev_print("Module unloading ...\n");

	for(index =0; index < COUNT; index++) {
		if (kern_pages[index] != NULL) {
			__free_page(kern_pages[index]);
		}
	}

	dev_print("Module unloading done !!\n");
	return;
}

module_init(alloc_sg_list);	
module_exit(dealloc_sg_list);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Create Scatter Gather Table");
