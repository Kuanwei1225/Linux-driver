#include <linux/module.h>	//module stuff
#include <linux/kernel.h>	//printk
#include <linux/init.h>		//__init
#include <linux/cdev.h>		//region char device
#include <linux/ioctl.h>
#include <linux/err.h>		//Error checking macros
//#include <asm/uaccess.h>	//translation from userspace ptr to kernelspace
#include <linux/fs.h>		//File operation structures
#include <linux/device.h>	//Device class stuff

#include <linux/types.h>	//uintxx_t

#define MOD_NAME "hello_dev" 				//name of the device file

static struct file_operations hello_fops = {
	.owner			= THIS_MODULE,
};

static int hello_major = 0;           /*dynamic*/
static int hello_minor = 0;
static int hello_devs = 1;           /* device count */

static struct class *my_class = NULL;
static struct cdev hello_cdev;

static int __init hello_init(void)
{
	int alloc_ret = 0, cdev_err = 0, major;
	struct device *my_dev;

	dev_t dev = MKDEV(hello_major, 0);

	printk(KERN_INFO " : Startup\n");
	//register char device
	alloc_ret = 
		alloc_chrdev_region(&dev, 0, hello_devs, MOD_NAME);//character device
	if (alloc_ret) {
		printk(KERN_ALERT " : Device Cannot Register\n");
		return -1;
	}
	hello_major = major = MAJOR(dev);
	cdev_init(&hello_cdev, &hello_fops);
	hello_cdev.owner = THIS_MODULE;
	hello_cdev.ops = &hello_fops;
	cdev_err = cdev_add(&hello_cdev, MKDEV(hello_major, hello_minor), hello_devs);
	if (cdev_err) {
		printk(KERN_ALERT " : cdev initial faild\n");
		goto error_i;
	}
	/*register class*/
	my_class = class_create(THIS_MODULE, "my_class");
	if (IS_ERR(my_class) ) {
		printk(KERN_ALERT " : Cannot creat class\n");
		goto error_i;
	}
	my_dev = device_create (
						my_class,
						NULL,
						MKDEV(hello_major, 0),
						NULL, MOD_NAME );
	if (IS_ERR(my_dev)) {
		printk(KERN_ALERT " : Cannot create device\n");
		class_destroy(my_class);
		goto error_i;
	}
	return 0;
error_i:
	if (cdev_err == 0)
		cdev_del(&hello_cdev);
	if (alloc_ret == 0)
		unregister_chrdev_region(dev, hello_devs);
	return -1;
}

static void __exit hello_exit(void)
{
	device_destroy(my_class, MKDEV(hello_major, 0));
	class_destroy(my_class);
	unregister_chrdev(hello_major, MOD_NAME);

	printk(KERN_NOTICE " : Goodbye\n");
}
MODULE_LICENSE("Dual BSD/GPL");
module_init(hello_init);
module_exit(hello_exit);

