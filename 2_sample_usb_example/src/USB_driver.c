#include <linux/module.h>
#include <linux/kernel/h>
#include <linux/usb.h>

#define USB_VENDOR_ID 0xfff0
#define USB_PRODUCT_ID 0xfff0

static int mydev_probe(struct usb_interface *interface, 
			const struct usb_device_id *id) {
	printk(KERN_INFO "My dev (%04x:%04x)", 
			id->idVendor, id->idProduct);
}

static struct usb_device_id my_table[] = {
	{USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID)},
	{}
}
MODULE_DEVICE_TABLE(usb, my_table);

static struct usb_dtiver my_driver = {
	.name = "my_driver",
	.id_table = mydev_table,
	.probe = mydev_probe,
	.disconnect = mydev_disconnect,
}

static int __init my_usb_init(void) {
	return usb_register(&my_driver);
}

static void __exit my_usb_exit(void) {
	usb_deregister(&my_driver);
}

module_init(my_usb_init);
module_exit(my_usb_exit);

MODULE_LICENCE("GPL");
