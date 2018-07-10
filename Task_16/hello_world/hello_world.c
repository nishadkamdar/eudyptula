#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/byteorder/generic.h>

static int hello_data __initdata = 3;

int __init hello_init(void)
{
	char a = 12;
	__le16 b = cpu_to_le16(a);
		
	printk(KERN_INFO "converted value to __le16 id %hu\n", b);
	printk(KERN_ALERT"Hello, Kernel World!\n"); 
	return 0;
}

void __exit hello_exit(void) 
{
	printk(KERN_ALERT"Goodbye, Kernel World!\n"); 
}

MODULE_AUTHOR("NISHAD"); 
MODULE_LICENSE("GPL"); 
MODULE_DESCRIPTION("Hello Kernel World!!");

module_init(hello_init); 
module_exit(hello_exit); 
