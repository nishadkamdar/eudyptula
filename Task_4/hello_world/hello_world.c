#include <linux/init.h>
#include <linux/module.h>

static int hello_data __initdata = 3;

int __init hello_init(void)
{
	pr_debug("Hello, Kernel World!\n");
	return 0;
}

void __exit hello_exit(void)
{
	pr_debug("Goodbye, Kernel World!\n");
}

MODULE_AUTHOR("NISHAD");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello Kernel World!!");

module_init(hello_init);
module_exit(hello_exit);
