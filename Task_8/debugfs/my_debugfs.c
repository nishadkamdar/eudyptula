/*
 *my_debugfs.c
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/sched.h>

static struct dentry *my_debugfs_root;

static char author_id[] = "Nishad Kamdar";
static const ssize_t author_id_size = sizeof(author_id);
static char user_input[4096];

struct foo {
	char page[4096];
	struct mutex lock;
};

static struct foo foo_page;

static int id_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "id opened\n");
	return 0;
}

static int foo_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "foo opened\n");
	return 0;
}

static ssize_t id_read(struct file *filp, char __user *buf,
			size_t count, loff_t *f_pos)
{
	ssize_t retval = 0;

	printk(KERN_INFO "id: Device file is read at offset = %i," 
			"read bytes count = %u\n", (int)*f_pos,
			(unsigned int)count);

	if (*f_pos >= author_id_size)
		return 0;

	if (*f_pos + count > author_id_size) {
		count = author_id_size - *f_pos;
	}

	if (copy_to_user(buf, author_id + *f_pos, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

out:
	return retval;
}

static ssize_t foo_read(struct file *filp, char __user *buf,
			size_t count, loff_t *f_pos)
{
	ssize_t retval = 0;

	if (mutex_lock_interruptible(&foo_page.lock))
		return -ERESTARTSYS;
	printk(KERN_INFO "foo: Device file is read at offset = %i,"
			"read bytes count = %u\n", (int)*f_pos,
			(unsigned int)count);

	if (*f_pos >= 4096) {	
		mutex_unlock(&foo_page.lock);
		return 0;
	}

	if (*f_pos + count > 4096) {
		count = 4096 - *f_pos;
	}

	if (copy_to_user(buf, foo_page.page + *f_pos, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

out:
	mutex_unlock(&foo_page.lock);
	return retval;
}

static ssize_t id_write(struct file *filp, const char __user *buf,
			size_t count, loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;

	printk(KERN_INFO "id: Device file is written at offset = %i,"
			"written bytes count = %u\n", (int)*f_pos,
			(unsigned int)count);

	if (*f_pos >= 4096)
		return 0;

	if (copy_from_user(user_input + *f_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;

	if (strncmp(author_id, user_input, 13) != 0) {
		printk(KERN_INFO "incorrect value written\n");
		retval = -1;
	} else {
		printk(KERN_INFO "correct value written\n");
		retval = count;
	}

out:
	return retval;
}

static ssize_t foo_write(struct file *filp, const char __user *buf,
			size_t count, loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;
	bool mtx = 0;

	mtx = mutex_is_locked(&foo_page.lock);
	printk(KERN_INFO "mutex status is %i\n", mtx);	

	if (mutex_lock_interruptible(&foo_page.lock))
		return -ERESTARTSYS;

	printk(KERN_INFO "foo: Device file is written at offset = %i,"
			 "written bytes count = %u\n", (int)*f_pos,
			 (unsigned int)count);

	if (*f_pos >= 4096) {
		mutex_unlock(&foo_page.lock);
		return 0;
	}

	if (copy_from_user(foo_page.page + *f_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	retval = count;

out:
	mutex_unlock(&foo_page.lock);
	return retval;
}

static struct file_operations id_fops = {
	.owner = THIS_MODULE,
	.open = id_open,
	.read = id_read,
	.write = id_write,
};

static struct file_operations foo_fops = {
	.owner = THIS_MODULE,
	.open = foo_open,
	.read = foo_read,
	.write = foo_write,
};

static int my_debugfs_init(void)
{
	struct dentry *file_a, *file_b, *file_c;

	printk(KERN_INFO "my_debugfs init");

	my_debugfs_root = debugfs_create_dir("eudyptula", NULL);

	if (!my_debugfs_root)
		return -ENOENT;

	file_a = debugfs_create_file("id", 0644, my_debugfs_root, NULL, &id_fops);
	if (!file_a)
		goto FAIL;

	file_b = debugfs_create_u64("jiffies", 0444, my_debugfs_root, (u64)&jiffies);
	if (!file_b)
		goto FAIL;

	mutex_init(&foo_page.lock);

	file_c = debugfs_create_file("foo", 0644, my_debugfs_root, NULL, &foo_fops);
	if (!file_c)
		goto FAIL;

	return 0;

FAIL:
	debugfs_remove_recursive(my_debugfs_root);
	my_debugfs_root = NULL;
	return -ENOENT;
}

static void my_debugfs_exit(void)
{
	printk(KERN_INFO "my_debugfs_exit");
	debugfs_remove_recursive(my_debugfs_root);	
}

module_init(my_debugfs_init);
module_exit(my_debugfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nishad Kamdar");
MODULE_DESCRIPTION("debugfs file enumeration");
