/*
 * USB Skeleton driver - 2.2
 *
 * Copyright (C) 2001-2004 Greg Kroah-Hartman (greg@kroah.com)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 * This driver is based on the 2.6.3 version of drivers/usb/usb-skeleton.c
 * but has been rewritten to be easier to read and use.
 *
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/mutex.h>


/*name of the device file*/
#define MOD_NAME "My_gamepad_dev" 		

/* Define these values to match your devices */
#define USB_gamepad_VENDOR_ID	0x046d
#define USB_gamepad_PRODUCT_ID	0xc21d

/* table of devices that work with this driver */
static const struct usb_device_id gamepad_table[] = {
	{ USB_DEVICE(USB_gamepad_VENDOR_ID, USB_gamepad_PRODUCT_ID) },
	{ }					/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, gamepad_table);


/* Get a minor range for your devices from the usb maintainer */
#define USB_GAMEPAD_MINOR_BASE	192

/* our private defines. if this grows any larger, use your own .h file */
#define MAX_TRANSFER		(PAGE_SIZE - 512)
/* MAX_TRANSFER is chosen so that the VM is not stressed by
   allocations > PAGE_SIZE and the number of packets in a page
   is an integer 512 is the largest possible packet on EHCI */
#define WRITES_IN_FLIGHT	8
/* arbitrarily chosen */

/* Structure to hold all of our device specific stuff */
struct usb_gamepad {
	struct usb_device	*udev;			/* the usb device for this device */
	struct usb_interface	*interface;		/* the interface for this device */
	struct urb		*inte_in_urb;	
	unsigned char           *inte_in_buffer;	/* the buffer to receive data */
	size_t			inte_in_size;		/* the size of the receive buffer */
	__u8			inte_in_endpointAddr;	/* the address of the interrupt in endpoint */
	__u8			inte_out_endpointAddr;	/* the address of the interrupt out endpoint */
	int			errors;			/* the last request tanked */
	struct kref		kref;
};
#define to_gamepad_dev(d) container_of(d, struct usb_gamepad, kref)

static struct usb_driver gamepad_driver;

static void gamepad_delete(struct kref *kref)
{
	struct usb_gamepad *dev = to_gamepad_dev(kref);

	usb_put_dev(dev->udev);
	kfree(dev->inte_in_buffer);
	kfree(dev);
}

static int gamepad_open(struct inode *inode, struct file *file)
{
	struct usb_gamepad *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	printk(KERN_INFO "open!!!!!!\n");

	subminor = iminor(inode);

	interface = usb_find_interface(&gamepad_driver, subminor);
	if (!interface) {
		pr_err("%s - error, can't find device for minor %d\n",
			__func__, subminor);
		retval = -ENODEV;
		goto exit;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		retval = -ENODEV;
		pr_err("error, can't get interface data \n");
		goto exit;
	}

//	retval = usb_autopm_get_interface(interface);
//	if (retval) {
//		pr_err("autopm get interface error \n");
//		goto exit;
//	}
	/* increment our usage count for the device */
	kref_get(&dev->kref);

	/* save our object in the file's private structure */
	file->private_data = dev;

exit:
	return retval;

}

static int gamepad_release(struct inode *inode, struct file *file)
{
	struct usb_gamepad *dev;

	dev = file->private_data;
	if (dev == NULL)
		return -ENODEV;

	/* decrement the count on our device */
	kref_put(&dev->kref, gamepad_delete);
	return 0;

}
static void gamepad_read_io(struct urb *urb)
{
	int status;

	status = usb_submit_urb (urb, GFP_ATOMIC);
	if (status)
		printk(KERN_INFO "can't submit intr.... \n");
}

static ssize_t gamepad_read(struct file *file, char *buffer, size_t count,
			 loff_t *ppos)
{
	struct usb_gamepad *dev;
	int rv, retval, read_c;

	dev = file->private_data;

	/* errors must be reported */
	rv = dev->errors;
	if (rv < 0) {
		/* any error is reported once */
		dev->errors = 0;
		/* to preserve notifications about reset */
		rv = (rv == -EPIPE) ? rv : -EIO;
		/* report it */
		goto exit;
	}
	usb_fill_int_urb(dev->inte_in_urb,
			dev->udev,
			usb_rcvintpipe(dev->udev,
				dev->inte_in_endpointAddr),
			dev->inte_in_buffer,
			min(dev->inte_in_size, count),
			gamepad_read_io, dev, 4);
	gamepad_read_io(dev->inte_in_urb);
/*
	retval = usb_interrupt_msg(dev->udev,
			      usb_rcvintpipe(dev->udev, dev->inte_in_endpointAddr),
			      dev->inte_in_buffer,
			      min(dev->inte_in_size, count),
			      &read_c, HZ*10);  //return 0 (success)/-n(fail)
*/	
	printk(KERN_INFO "Return message: %d\n", retval);
	/* if the read was successful, copy the data to userspace */
	if (!retval) {
		if (copy_to_user(buffer, dev->inte_in_buffer, read_c))
			retval = -EFAULT;
		else
			retval = count;
	}


exit:
	return rv;
}

static const struct file_operations gamepad_fops = {
	.owner =	THIS_MODULE,
	.read =		gamepad_read,
	.open =		gamepad_open,
	.release =	gamepad_release,
};

/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static struct usb_class_driver gamepad_dev_class = {
	.name =		"My_gamepad%d",
	.fops =		&gamepad_fops,
	.minor_base =	USB_GAMEPAD_MINOR_BASE,
};

static int gamepad_probe(struct usb_interface *interface,
		      const struct usb_device_id *id)
{
	struct usb_gamepad *dev;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	size_t buffer_size;
	int i;
	int retval = -ENOMEM;

	printk(KERN_INFO "\nProbe start........\n");
	/* allocate memory for our device state and initialize it */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		goto error;
	kref_init(&dev->kref);

	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	/* set up the endpoint information */
	/* use only the first interrupt-in and interrupt-out endpoints */
	iface_desc = interface->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (!dev->inte_in_endpointAddr &&
		     usb_endpoint_is_int_in(endpoint)) {
			/* we found a interrupt in endpoint */
			buffer_size = usb_endpoint_maxp(endpoint);
			dev->inte_in_size = buffer_size;
			dev->inte_in_endpointAddr = endpoint->bEndpointAddress;
			dev->inte_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
			if (!dev->inte_in_buffer)
				goto error;
			dev->inte_in_urb = usb_alloc_urb(0, GFP_KERNEL);
			if (!dev->inte_in_urb)
				goto error;
		}

		if (!dev->inte_out_endpointAddr &&
		    usb_endpoint_is_int_out(endpoint)) {
			/* we found a interrupt out endpoint */
			dev->inte_out_endpointAddr = endpoint->bEndpointAddress;
		}
	}
	if (!(dev->inte_in_endpointAddr && dev->inte_out_endpointAddr)) {
		dev_err(&interface->dev,
			"Could not find both interrupt-in and interrupt-out endpoints\n");
		goto error;
	}

	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);

	/* we can register the device now, as it is ready */
	retval = usb_register_dev(interface, &gamepad_dev_class);
	if (retval) {
		/* something prevented us from registering this driver */
		dev_err(&interface->dev,
			"Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

	/* let the user know what node this device is now attached to */
	dev_info(&interface->dev,
		 "USB gamepad device now attached to USBgamepad-%d",
		 interface->minor);
	printk(KERN_INFO "endpoint in : %04x, endpoint out : %04x\n",
		dev->inte_in_endpointAddr, dev->inte_out_endpointAddr);
	printk(KERN_INFO "Probe success!!!!!!!!\n");
	return 0;

error:
	if (dev)
		/* this frees allocated memory */
		kref_put(&dev->kref, gamepad_delete);
	return retval;
}

static void gamepad_disconnect(struct usb_interface *interface)
{
	struct usb_gamepad *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &gamepad_dev_class);



	/* decrement our usage count */
	kref_put(&dev->kref, gamepad_delete);

	dev_info(&interface->dev, "USB gamepad #%d now disconnected", minor);
}

static struct usb_driver gamepad_driver = {
	.name =		"Your device name",
	.probe =	gamepad_probe,
	.disconnect =	gamepad_disconnect,
	.id_table =	gamepad_table,
};

MODULE_LICENSE("GPL");
module_usb_driver(gamepad_driver);



