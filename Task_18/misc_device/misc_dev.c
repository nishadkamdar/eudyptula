/*
 * misc_dev.c
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/slab.h>

static const char author_id[] = "Nishad Kamdar";
static const ssize_t author_id_size = sizeof(author_id);
static const char user_input[4096];
static struct task_struct *eudyptula_thread;
static unsigned long id_count;

DECLARE_WAIT_QUEUE_HEAD(wee_wait);

struct identity {
	char name[20];
	int id;
	bool busy;
	struct list_head mylist;
};

LIST_HEAD(my_id_list);

static struct identity identity_get(void)
{
	struct identity *id_str_ptr = NULL;
	static struct identity id_get;

	list_for_each_entry(id_str_ptr, &my_id_list, mylist) {
		id_get = *id_str_ptr;
		__list_del_entry(&id_str_ptr->mylist);
		kfree(id_str_ptr);
	}

	return id_get;
}


static int identity_create(char *name, int id)
{
	struct identity *temp_id;

	if (strlen(name)+1 > 20)
		return -ENOMEM;
	temp_id = kmalloc(sizeof(struct identity), GFP_KERNEL);
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
			kfree(id_str_ptr);
		}
	}
}

int kthread_fn(void *pv)
{
	DEFINE_WAIT(eu_wait);
	static struct identity *temp1;
	int ret = 0;

wait:
	temp1 = kmalloc(sizeof(struct identity), GFP_KERNEL);
	if (temp1 == NULL)
		return -ENOMEM;
	
	prepare_to_wait(&wee_wait, &eu_wait, TASK_INTERRUPTIBLE);
	if(!kthread_should_stop()) {
		printk(KERN_INFO "Kthread going to sleep\n");
		schedule();
	}

	printk(KERN_INFO "Kthread awakened\n");

	if (kthread_should_stop())
	{
		printk(KERN_INFO "Kthread stop funtion called\n");
	}
	else
	{
		*temp1 = identity_get();
	
		printk(KERN_INFO "process going to 5sec interruptible sleep\n");
		ret = wait_event_interruptible_timeout(wee_wait, 0, 5000);
		if (ret < 0) {
			printk(KERN_INFO, "process interrupted\n");
			return -ERESTARTSYS;
		}
		
		printk(KERN_INFO, "process woken up\n");
		printk(KERN_INFO "temp->name = %s\n", temp1->name);
		printk(KERN_INFO "temp->id = %d\n", temp1->id);
		
		kfree(temp1);
		goto wait;
	}
}

static int simple_dev_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "eudyptula opened\n");
	return 0;
}

static ssize_t simple_dev_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;

	printk(KERN_INFO "misc_dev: Device file is written at offset = %i, written bytes count = %u\n", (int)*f_pos, (unsigned int)count);

	if (*f_pos >= 19)
		return 0;

	if (*f_pos + count > 19)
		count = 19 - *f_pos;

	if (copy_from_user(user_input + *f_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	retval = count;
	*f_pos += count;

	/*if (strncmp(author_id, user_input, 13) != 0) {
		printk(KERN_INFO "incorrect value written\n");
		retval = -1;
	} else {
		printk(KERN_INFO "correct value written\n");
		retval = count;
	}*/

	identity_create(user_input, id_count);
	id_count++;

	wake_up_process(eudyptula_thread);
out:
	return retval;
}

static const struct file_operations simple_dev_fops = {
	.open =		simple_dev_open,
	.write =	simple_dev_write,
};

static struct miscdevice simple_dev = {
	.minor =	MISC_DYNAMIC_MINOR,
	.name =		"eudyptula",
	.fops =		&simple_dev_fops,
	.mode = 	0222,
};

static int misc_init(void)
{
	int error;
	struct identity *temp;
	struct identity temp1;
	
	error = misc_register(&simple_dev);
	if (error) {
		printk(KERN_INFO "can't misc_register");
		return error;
	}
	printk(KERN_INFO "misc device initialized! minor number is %i\n", simple_dev.minor);

	eudyptula_thread = kthread_run(kthread_fn, NULL, "eudyptula");
	if (eudyptula_thread) {
		printk(KERN_ERR "Kthread Created Successfully...\n");
	} else {
		printk(KERN_ERR "Cannot create kthread\n");
	}

	if (identity_create("Alice", 1) < 0)
		pr_debug("unable to allocate memory\n");

	if (identity_create("Bob", 2) < 0)
		pr_debug("unable to alocate memory\n");

	if (identity_create("Dave", 3) < 0)
		pr_debug("unable to alocate memory\n");

	if (identity_create("Gena", 4) < 0)
		pr_debug("unable to alocate memory\n");

	temp1 = identity_get();
	
	printk(KERN_INFO "temp->name = %s\n", temp1.name);
	printk(KERN_INFO "temp->id = %d\n", temp1.id);

	temp = identity_find(3);
	pr_debug("id 3 = %s\n", temp->name);

	temp = identity_find(42);
	if (temp == NULL)
		pr_debug("id 42 not found\n");

	identity_destroy(2);
	identity_destroy(1);
	identity_destroy(10);
	identity_destroy(42);
	identity_destroy(3);
	
	return 0;
}

static void misc_exit(void)
{
	int i;
	for (i = 0; i < id_count; i++)
	{
		identity_destroy(i);
	}

	printk(KERN_INFO "List destroyed.\n");
	
	printk(KERN_INFO "See you later.\n");
	kthread_stop(eudyptula_thread);
	misc_deregister(&simple_dev);
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nishad Kamdar");
MODULE_DESCRIPTION("char misc module");
