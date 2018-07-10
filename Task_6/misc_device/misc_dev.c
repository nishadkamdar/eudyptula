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

static const char author_id[] = "Nishad Kamdar";
static const ssize_t author_id_size = sizeof(author_id);
static const char user_input[4096];

static int simple_dev_open(struct inode *inode, struct file *filp)
{
	pr_debug("eudyptula opened\n");
	return 0;
}

static ssize_t simple_dev_read(struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos)
{
	ssize_t retval = 0;

	pr_debug("misc_dev: Device file is read at offset = %i,
			read bytes count = %u\n", (int)*f_pos,
			(unsigned int)count);

	if (*f_pos >= author_id_size)
		return 0;

	if (*f_pos + count > author_id_size)
		count = author_id_size - *f_pos;

	if (copy_to_user(buf, author_id + *f_pos, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

out:
	return retval;
}

static ssize_t simple_dev_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;

	pr_debug("misc_dev: Device file is written at offset = %i,
			written bytes count = %u\n", (int)*f_pos,
			(unsigned int)count);

	if (*f_pos >= 4096)
		return 0;

	if (*f_pos + count > 4096)
		count = 4096 - *f_pos;

	if (copy_from_user(user_input + *f_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;

	if (strncmp(author_id, user_input, 13) != 0) {
		pr_debug("incorrect value written\n");
		retval = -1;
	} else {
		pr_debug("correct value written\n");
		retval = count;
	}

out:
	return retval;
}

static const struct file_operations simple_dev_fops = {
	.open =		simple_dev_open,
	.read =		simple_dev_read,
	.write =	simple_dev_write,
};

static struct miscdevice simple_dev = {
	.minor =	MISC_DYNAMIC_MINOR,
	.name =		"eudyptula",
	.fops =		&simple_dev_fops,
};

static int misc_init(void)
{
	int error;

	error = misc_register(&simple_dev);
	if (error) {
		pr_debug("can't misc_register");
		return error;
	}
	pr_debug("misc device initialized! minor number is %i\n",
			simple_dev.minor);
	return 0;
}

static void misc_exit(void)
{
	pr_debug("See you later.\n");
	misc_deregister(&simple_dev);
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nishad Kamdar");
MODULE_DESCRIPTION("char misc module");
