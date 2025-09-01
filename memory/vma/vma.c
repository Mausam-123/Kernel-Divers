#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>		// For allocations
#include <linux/mm.h>
#include <linux/rbtree.h>	// For rb tress operations

#define dev_print(fmt, ...)	printk(KERN_DEFAULT "Mausam[%s:%d] : " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define ANON_VMA_START  	0x60000000UL
#define ANON_VMA_SIZE   	(2 * PAGE_SIZE)

struct mm_struct *test_mm = NULL;
struct vm_area_struct *test_vma = NULL;

//Declarations
static int __init test_vma_init(void);
static void __exit test_vma_exit(void);
void test_vma_open(struct vm_area_struct *vma);
void test_vma_close(struct vm_area_struct *vma);
vm_fault_t test_vma_fault(struct vm_fault *vma_fault);
int insert_vma_to_mm(struct mm_struct *mm, struct vm_area_struct *vma);

//Called during mmap(), clone(), fork()
void test_vma_open(struct vm_area_struct *vma) {
	dev_print("vma_open() callback function is called\n");
	return;
}

void test_vma_close(struct vm_area_struct *vma) {
	dev_print("vma_close() callback function is called\n");
	return;
}

// Below callback is called by kernel page fault handler
vm_fault_t test_vma_fault(struct vm_fault *vma_fault) {
	dev_print("vma_fault() callback is called\n");
	return VM_FAULT_NOPAGE;
}

struct vm_operations_struct test_vma_ops  = {
	.open = test_vma_open, 
	.close = test_vma_close,
	.fault = test_vma_fault,
};

// Static template to initialize anon_vma with read-only vm_flags set
const struct vm_area_struct test_vma_metadata = {
    .vm_start = ANON_VMA_START,
    .vm_end = ANON_VMA_START + ANON_VMA_SIZE,
    .vm_flags = VM_READ | VM_WRITE,
    .vm_ops = &test_vma_ops,
};

// Insert the VMA into mm_struct properly using kernel API
int insert_vma_to_mm(struct mm_struct *mm, struct vm_area_struct *vma)
{
	struct vm_area_struct *prev = NULL;

	// Optionally find a suitable previous VMA for ordering
	// For simplicity, NULL means insert without a known previous VMA
	// If you have an address to insert after, you can use find_vma_prev()
	// example: prev = find_vma_prev(mm, vma->vm_start);

	// Insert the VMA safely into the mm_struct's VMA tree/list
	// For secruity reason, vma_link() is not exposed to loadable modules so we cannot use it
	// For inserting vma node use mmap() call from the user space
	//return vma_link(mm, prev, vma);
	return 1;
}

static int __init test_vma_init(void) {
	unsigned long valid_addr = ANON_VMA_START + ANON_VMA_SIZE/2;
	unsigned long bad_addr = 0x50000000UL;
	struct vm_area_struct *vma_node = NULL;

	dev_print("Module init .....\n");

	//Create mm stuct 
	test_mm = kmalloc(sizeof(*test_mm), GFP_KERNEL);
	if (test_mm == NULL) {
		dev_print("Failed to allocate test_mm structure\n");
		test_vma = NULL;
		return -ENOMEM;
	}

	memset(test_mm, 0, sizeof(*test_mm));

	//Create vma node
	test_vma = kmalloc(sizeof(*test_vma), GFP_KERNEL);		//kzalloc() can also be used
	if (test_vma == NULL) {
		dev_print("Failed to allocate test_vma structure\n");
		if(test_mm != NULL) {
			kfree(test_mm);
			test_mm = NULL;
		}
		test_vma = NULL;
		return -ENOMEM;
	}

	memset(test_vma, 0 , sizeof(*test_vma));
	
	memcpy(test_vma, &test_vma_metadata, sizeof(*test_vma));
	test_vma->vm_page_prot = pgprot_noncached(test_vma->vm_page_prot);
	

	//Insert vma node into mm struct
	// As below call will not insert vma into mm, so all the above address will be invalid
	insert_vma_to_mm(test_mm, test_vma);

	//Search for valid address
	vma_node = find_vma(test_mm, valid_addr);
	if (vma_node && (test_vma->vm_start <= valid_addr) && (valid_addr < test_vma->vm_end)) {
		dev_print("0x%lx is a valid address present inside vma\n", valid_addr);
	} else {
		dev_print("0x%lx is a invalid address not present inside vma\n", valid_addr);
	}

	vma_node = NULL;

	//Search for invalid address
	vma_node = find_vma(test_mm, bad_addr);
	if (vma_node && (test_vma->vm_start <= bad_addr) && (bad_addr < test_vma->vm_end)) {
		dev_print("0x%lx is a valid address present inside vma\n", bad_addr);
	} else {
		dev_print("0x%lx is a invalid address not present inside vma\n", bad_addr);
	}	

	dev_print("Module init done !!\n");

	return 0;
}

static void __exit test_vma_exit(void) {
	dev_print("Module unloading ...\n");

	if(test_mm != NULL) {
		kfree(test_mm);
	}

	if (test_vma != NULL) {
		kfree(test_vma);
	}	

	dev_print("Module unloading done !!\n");
	return;
}

module_init(test_vma_init);	
module_exit(test_vma_exit);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("VMA Operations");
