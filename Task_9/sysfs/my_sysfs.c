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
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/sched.h>

static struct kobject *my_kobject_root;

static char author_id[] = "Nishad Kamdar";
static const ssize_t author_id_size = sizeof(author_id);
static char user_input[4096];


struct foo {
	char page[4096];
	struct mutex lock;
};

static struct foo foo_page;

static ssize_t id_show(struct kobject *kobj, struct kobj_attribute *attr,
		char *buf)
{
	return sprintf(buf, "%s\n", author_id);
}

static ssize_t id_store(struct kobject *kobj, struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	int retval;

	strncpy(user_input, buf, count);
	
	if (strncmp(author_id, user_input, 13) != 0) {
		printk(KERN_INFO "incorrect value written\n");
		retval = -1;
	} else {
		printk(KERN_INFO "correct value written\n");
		retval = count;
	}
	return retval;
}

static ssize_t jiffies_show(struct kobject *kobj, struct kobj_attribute *attr,
		char *buf)
{
	return sprintf(buf, "%lu\n", jiffies);
}

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	ssize_t retval = 0;

	if (mutex_lock_interruptible(&foo_page.lock))
		return -ERESTARTSYS;

	retval = sprintf(buf, "%s\n", foo_page.page);
	
	mutex_unlock(&foo_page.lock);
	return retval;
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	ssize_t retval = 0;

	if (mutex_lock_interruptible(&foo_page.lock))
		return -ERESTARTSYS;

	strncpy(foo_page.page, buf, count);
	retval = count;

	mutex_unlock(&foo_page.lock);
	return retval;
}

static struct kobj_attribute id_attribute =__ATTR(id, 0644, id_show, id_store);

static struct kobj_attribute jiffies_attribute =__ATTR_RO(jiffies);

static struct kobj_attribute foo_attribute =__ATTR(foo, 0644, foo_show, foo_store);

/*static int foo_open(struct inode *inode, struct file *filp)
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
*/
static int my_sysfs_init(void)
{
	int file_a, file_b, file_c;

	printk(KERN_INFO "my_sysfs init");

	my_kobject_root = kobject_create_and_add("eudyptula", kernel_kobj);

	if(!my_kobject_root)
		return -ENOMEM;

	file_a = sysfs_create_file(my_kobject_root, &id_attribute.attr);
	if (file_a)
		goto FAIL;

	file_b = sysfs_create_file(my_kobject_root, &jiffies_attribute.attr);
	if (file_b)
		goto FAIL;

	file_c = sysfs_create_file(my_kobject_root, &foo_attribute.attr);
	if (file_c)
		goto FAIL;
	
	/*mutex_init(&foo_page.lock);

	file_c = debugfs_create_file("foo", 0644, my_debugfs_root, NULL, &foo_fops);
	if (!file_c)
		goto FAIL;*/

	return 0;

FAIL:
	kobject_put(my_kobject_root);
	my_kobject_root = NULL;
	return -ENOENT;
}

static void my_sysfs_exit(void)
{
	printk(KERN_INFO "my_sysfs_exit");
	kobject_put(my_kobject_root);
}

module_init(my_sysfs_init);
module_exit(my_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nishad Kamdar");
MODULE_DESCRIPTION("sysfs file enumeration");
