// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>

struct identity {
	char name[20];
	int id;
	bool busy;
	struct list_head mylist;
};

LIST_HEAD(my_id_list);
static struct kmem_cache *kll_cache;

static int identity_create(char *name, int id)
{
	struct identity *temp_id;

	if (strlen(name)+1 > 20)
		return -ENOMEM;
	temp_id = kmem_cache_alloc(kll_cache, GFP_KERNEL);
	if (temp_id == NULL)
		return -ENOMEM;

	strcpy(temp_id->name, name);
	temp_id->id = id;
	temp_id->busy = false;

	list_add(&temp_id->mylist, &my_id_list);
	return 0;
}

struct identity *identity_find(int id)
{
	struct identity *id_str_ptr = NULL;

	list_for_each_entry(id_str_ptr, &my_id_list, mylist) {
		if (id_str_ptr->id == id)
			return id_str_ptr;
	}

	return NULL;
}

void identity_destroy(int id)
{
	struct identity *id_str_ptr = NULL;

	list_for_each_entry(id_str_ptr, &my_id_list, mylist) {
		if (id_str_ptr->id == id) {
			__list_del_entry(&id_str_ptr->mylist);
			kmem_cache_free(kll_cache, id_str_ptr);
		}
	}
}
static void kll_const(void *addr)
{
	memset(addr, 0, sizeof(struct identity));
}

int __init linked_list_init(void)
{
	struct identity *temp;

	kll_cache = kmem_cache_create("kll", sizeof(struct identity), 0,
			SLAB_HWCACHE_ALIGN, kll_const);
	if (!kll_cache) {
		pr_debug("unable to allocate memory\n");
		return -ENOMEM;
	}

	if (identity_create("Alice", 1) < 0)
		pr_debug("unable to allocate memory\n");

	if (identity_create("Bob", 2) < 0)
		pr_debug("unable to alocate memory\n");

	if (identity_create("Dave", 3) < 0)
		pr_debug("unable to alocate memory\n");

	if (identity_create("Gena", 4) < 0)
		pr_debug("unable to alocate memory\n");

	temp = identity_find(3);
	pr_debug("id 3 = %s\n", temp->name);

	temp = identity_find(42);
	pr_debug("id 42 not found\n");

	identity_destroy(2);
	identity_destroy(1);
	identity_destroy(10);
	identity_destroy(42);
	identity_destroy(3);

	return 0;
}

void __exit linked_list_exit(void)
{
	kmem_cache_destroy(kll_cache);
	pr_debug("Goodbye, Kernel World!\n");
}

MODULE_AUTHOR("NISHAD");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello Kernel linked list!!");

module_init(linked_list_init);
module_exit(linked_list_exit);
