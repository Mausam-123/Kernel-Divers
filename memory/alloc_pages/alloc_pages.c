#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>		// For allocations
#include <linux/highmem.h>	// For kmap, kunmap
#include <linux/mm.h>

#define dev_print(fmt, ...)	printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define COUNT			(16)
#define PAGE_SIZE_4K		(4096)
#define PAGES_16		(COUNT*PAGE_SIZE_4K)

struct page *pages;
void *page_addr[COUNT] = {};

//Declarations
static int __init alloc_init(void);
static void __exit alloc_exit(void);

static int __init alloc_init(void) {
	int page_count = 0;
	int order = 0;
	long unsigned pfn_index = 0;

	dev_print("Module init .....\n");

	page_count = PAGES_16 / PAGE_SIZE_4K;
	order = ilog2(page_count);

	//allocate 2nd order contigious pages 
	pages = alloc_pages(GFP_KERNEL, order);
	if(pages == NULL) {
		dev_print("Unable to allocate the physical pages\n");
		return -ENOMEM;
	}

	//Map those physical pages
	for(int i = 0; i < COUNT; i++) {
		struct page *curr_page = pages + i;
 
		pfn_index = page_to_pfn(curr_page);
		page_addr[i] = kmap(curr_page);

		if(page_addr[i] == NULL) {
			dev_print("Unable to map %d index page with pfn %lu\n", i, pfn_index);

			//Unmap all the allocated pages so far
			while(--i>= 0) {
				kunmap(page_addr[i]);
			}

			// Free all the pages
			__free_pages(pages, order);
			return -ENOMEM;
		}

		//Mark even pages as dirty
		if(i%2 == 0) {
			memset(page_addr[i], (int)0xFF, PAGE_SIZE_4K);
			set_page_dirty(curr_page);

			dev_print("Page index : %d - Virtual address : %p is mapped to %lu pfn - Marked dirty\n",
				i, page_addr[i], pfn_index);
		} else {
			memset(page_addr[i], 0x00, PAGE_SIZE_4K);
			dev_print("Page index : %d - Virtual address : %p is mapped to %lu pfn\n", 
				i, page_addr[i], pfn_index);
		}
	}

	dev_print("Module init done !!\n");
	return 0;
}

static void __exit alloc_exit(void) {
	dev_print("Module unloading ...\n");

	//Unmap all the pages first
	for(int i = 0; i < COUNT; i++) {
		if(page_addr[i] != NULL) {
			kunmap(page_addr[i]);
		}
	}

	if(pages != NULL) {
		__free_pages(pages, 4);
	}

	dev_print("Module unloading done !!\n");
	return;
}

module_init(alloc_init);	
module_exit(alloc_exit);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Data allocation program");
