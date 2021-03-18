#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>

struct hi_dev {
	dev_t devt;
	struct class *cls;
	struct cdev cdev;
	struct device *dev;
};

static struct hi_dev hid;

static int hi_open(struct inode *inode, struct file *file)
{
	pr_info("hi: Device file opened\n");
	return 0;
}

static int hi_release(struct inode *inode, struct file *file)
{
	pr_info("hi: Device file closed\n");
	return 0;
}

static ssize_t hi_read(struct file *filp, char __user *buf, size_t len,
		       loff_t *off)
{
	pr_info("hi: Device file read\n");
	return 0;
}

static ssize_t hi_write(struct file *filp, const char __user *buf, size_t len,
			loff_t *off)
{
	pr_info("hi: Device file written\n");
	return len;
}

static const struct file_operations fops = {
	.owner		= THIS_MODULE,
	.read		= hi_read,
	.write		= hi_write,
	.open		= hi_open,
	.release	= hi_release,
};

static int __init hi_dev_init(struct hi_dev *hid)
{
	int err;

	err = alloc_chrdev_region(&hid->devt, 0, 1, "hi");
	if (err) {
		pr_err("hi: Failed to allocate major\n");
		goto error;
	}

	cdev_init(&hid->cdev, &fops);
	err = cdev_add(&hid->cdev, hid->devt, 1);
	if (err) {
		pr_err("hi: Failed to add device to the system\n");
		goto error_class;
	}

	hid->cls = class_create(THIS_MODULE, "hi_class");
	if (IS_ERR(hid->cls)) {
		pr_err("hi: Failed to create class\n");
		err = PTR_ERR(hid->cls);
		goto error_class;
	}

	hid->dev = device_create(hid->cls, NULL, hid->devt, NULL, "hi");
	if (IS_ERR(hid->dev)) {
		pr_err("hi: Failed to create device\n");
		err = PTR_ERR(hid->dev);
		goto error_device;
	}

	pr_info("hi: Initialized, major = %d\n", MAJOR(hid->devt));

	return 0;

error_device:
	class_destroy(hid->cls);
error_class:
	unregister_chrdev_region(hid->devt, 1);
error:
	return err;
}

static void __exit hi_dev_del(struct hi_dev *hid)
{
	device_destroy(hid->cls, hid->devt);
	class_destroy(hid->cls);
	cdev_del(&hid->cdev);
	unregister_chrdev_region(hid->devt, 1);

	pr_info("hi: Deinitialized\n");
}

static int __init hi_init(void)
{
	return hi_dev_init(&hid);
}

static void  __exit hi_exit(void)
{
	hi_dev_del(&hid);
}

module_init(hi_init);
module_exit(hi_exit);

MODULE_LICENSE("GPL");
