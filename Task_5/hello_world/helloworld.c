/*
 * helloworld.c
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

static int usb_kbd_hello_probe(struct usb_interface *iface,
		const struct usb_device_id *id)
{
	pr_debug("usb_kbd_hello (%04X:%04X) plugged\n", id->idVendor,
			id->idProduct);
	return 0;
}

static void usb_kbd_hello_disconnect(struct usb_interface *iface)
{
	pr_debug("usb_kbd_hello removed\n");
}

static const struct usb_device_id usb_kbd_hello_table[] = {
	{ .match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_INTERFACE_CLASS_HID },
	{ }
};

/*static const struct usb_device_id usb_kbd_hello_table[] = {
 *	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID,
 *	USB_INTERFACE_SUBCLASS_BOOT,
 *		USB_INTERFACE_PROTOCOL_KEYBOARD) },
 *	{ }
 * };
 */

MODULE_DEVICE_TABLE(usb, usb_kbd_hello_table);

static struct usb_driver usb_kbd_hello_driver = {
	.name =		"usb_kbd_hello",
	.probe =	usb_kbd_hello_probe,
	.disconnect =	usb_kbd_hello_disconnect,
	.id_table =	usb_kbd_hello_table,
};

static int hello_init(void)
{
	pr_debug("Hello World!\n");
	return usb_register(&usb_kbd_hello_driver);
}

static void hello_exit(void)
{
	pr_debug("See you later.\n");
	usb_deregister(&usb_kbd_hello_driver);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("7c1caf2f50d1");
MODULE_DESCRIPTION("Just a module");
