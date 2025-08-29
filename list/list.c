#include "list.h"

static int __init list_init(void) {
	struct metadata *node;

	dev_print("Creating List....\n");

	//Initialize global list head
	INIT_LIST_HEAD(&global_list_head);

	for(int i = 0; i < 5; i++) {
		node = kzalloc(sizeof(*node), GFP_KERNEL);
		if (node == NULL) {
			dev_print("Failed to allocate node\n");
			goto cleanup;
		}

		node->index = (i+1);
		node->data = (i+1)*2;
		list_add_tail(&node->list, &global_list_head);
		dev_print("Added node#%d with data - %d\n", node->index, node->data);
	}

	dev_print("List creation done !!\n");
	return 0;
cleanup:
	struct metadata *node_del;

	while(list_empty(&global_list_head) == 0) {
		node_del = list_first_entry(&global_list_head, struct metadata, list);
		list_del(&node_del->list);
		kfree(node_del);
	}

	if(node != NULL) {
		node = NULL;
	}

	if (node_del != NULL) {
		node_del = NULL;
	}
	return -1;
}

static void __exit list_exit(void) {
	struct list_head *iterator;
	struct metadata *node;

	//Print all the nodes
	dev_print("Print all the nodes\n");
	list_for_each(iterator, &global_list_head) {
		node = list_entry(iterator, struct metadata, list);	// anonymous to container_of()
		dev_print("node #%d has data %d\n",node->index, node->data);
	}

	iterator = NULL;
	dev_print("List deletion started ....\n");
	while(list_empty(&global_list_head) == 0) {
		node = list_first_entry(&global_list_head, struct metadata, list);
		dev_print("Deleting node#%d having data %d\n", node->index, node->data);
		list_del(&node->list);
		kfree(node);
	}

	dev_print("List deletion done !!\n");

	return;
}

module_init(list_init);	
module_exit(list_exit);

MODULE_AUTHOR("Mausam");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linked List program");
