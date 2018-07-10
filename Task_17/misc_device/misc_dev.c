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

static const char author_id[] = "Nishad Kamdar";
static const ssize_t author_id_size = sizeof(author_id);
static const char user_input[4096];
static struct task_struct *eudyptula_thread;

DECLARE_WAIT_QUEUE_HEAD(wee_wait);

int kthread_fn(void *pv)
{
	DEFINE_WAIT(eu_wait);

wait:
	prepare_to_wait(&wee_wait, &eu_wait, TASK_INTERRUPTIBLE);
	if(!kthread_should_stop()) {
		printk(KERN_INFO "Kthread going to sleep\n");
		schedule();
	}

	if (kthread_should_stop()) {
		printk(KERN_INFO "Kthread stop funtion called\n");
	}
	else
		goto wait;
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
		printk(KERN_INFO "incorrect value written\n");
		retval = -1;
	} else {
		printk(KERN_INFO "correct value written\n");
		retval = count;
	}

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

	return 0;
}

static void misc_exit(void)
{
	printk(KERN_INFO "See you later.\n");
	kthread_stop(eudyptula_thread);
	misc_deregister(&simple_dev);
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nishad Kamdar");
MODULE_DESCRIPTION("char misc module");
