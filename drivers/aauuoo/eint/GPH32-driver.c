/**
 * @file   foo_drv.c
 * @author zeke zang <zekezang@gmail.com>
 * @date   Tue Nov 12 22:21:19 2013
 * @zekezang  platform模型示例
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/gpio.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/poll.h>

#define DEV_NAME "GPH32_EINT"
#define DEVICE_MAJOR 248
#define FOO_NR_DEVS 1

static struct cdev foo_cdev;
static int foo_major = DEVICE_MAJOR;
static int foo_minor = 32;
static int foo_nr_devs = FOO_NR_DEVS;
static dev_t foo_devno;
static struct class* foo_class;
static struct resource *resource;
static char* rets;
static int dev_id = 9;
static wait_queue_head_t zekezang_waitq;
static volatile int press = 0;
static unsigned int EINT_PIN = 0;

static void foo_dev_release(struct device* dev) {
	disable_irq(EINT_PIN);
	free_irq(EINT_PIN, (void*) dev_id);
}

static struct resource foo_resource[] = {
		[0] = {
				.start = S5PV210_GPH3(2),
				.end = S5PV210_GPH3(2),
				.flags = IORESOURCE_IO, }, };

static struct platform_device foo_device = {
		.name = DEV_NAME,
		.id = 1,
		.resource = foo_resource,
		.dev = {
				//.platform_data = &foo_pdata,
				.release = &foo_dev_release, }, };

static int foo_open(struct inode *inode, struct file *filp) {
	rets = (char*) kmalloc(sizeof(char) * 1, GFP_KERNEL);
	return 0;
}

static int foo_release(struct inode *inode, struct file *filp) {
	return 0;
}

static ssize_t foo_write(struct file *file, const char __user *buf, size_t count, loff_t *off) {
	return 0;
}

static ssize_t foo_read(struct file *filp, char __user *buff, size_t count, loff_t *offp) {
	int err = 0;

	if (!press) {
		if (filp->f_flags & O_NONBLOCK) {
			return -EAGAIN;
		} else {
			wait_event_interruptible(zekezang_waitq, press);
		}
	}
	if(press == 1){
		strcpy(rets, "1");
		err = copy_to_user(buff, (void *) rets, 1);
	}
	if(press == 2){
		strcpy(rets, "0");
		err = copy_to_user(buff, (void *) rets, 1);
	}
	press = 0;
	return err;
}

static unsigned int foo_poll(struct file *file, struct poll_table_struct *wait) {
	unsigned int mask = 0;
	poll_wait(file, &zekezang_waitq, wait);

	if (press)
		mask |= POLLIN | POLLRDNORM;
	return mask;
}

static long foo_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	return 0;
}

static irqreturn_t zekezang_interrupt(int irq, void *dev_id) {
	unsigned long data;
	mdelay(15);
	data = gpio_get_value(S5PV210_GPH3(2));
	if (!data) {
		wake_up_interruptible(&zekezang_waitq);
		press = 1;
	}else{
		wake_up_interruptible(&zekezang_waitq);
		press = 2;
	}
	return IRQ_RETVAL(IRQ_HANDLED);
}

static struct file_operations foo_fops = {
		.owner = THIS_MODULE,
		.open = foo_open,
		.release = foo_release,
		.read = foo_read,
		.write = foo_write,
		.poll = foo_poll,
		.unlocked_ioctl = foo_ioctl, };

static int foo_remove(struct platform_device *dev) {
	unregister_chrdev_region(foo_devno, 1);
	cdev_del(&foo_cdev);
	device_destroy(foo_class, foo_devno);
	class_destroy(foo_class);
	return 0;
}

static int foo_probe(struct platform_device *dev) {
	int ret = 0;

	resource = platform_get_resource(dev, IORESOURCE_IO, 0);
	if (resource->flags == IORESOURCE_IO) {
		EINT_PIN = gpio_to_irq(resource->start);
		ret = request_irq(EINT_PIN, zekezang_interrupt, IRQ_TYPE_EDGE_BOTH, DEV_NAME, (void*) dev_id);
		if (ret) {
			disable_irq(EINT_PIN);
			free_irq(EINT_PIN, (void*) dev_id);
			return ret;
		}
	}

	init_waitqueue_head(&zekezang_waitq);

	cdev_init(&foo_cdev, &foo_fops);
	foo_cdev.owner = THIS_MODULE;
	if (foo_major) {
		foo_devno = MKDEV(foo_major, foo_minor);
		ret = register_chrdev_region(foo_devno, foo_nr_devs, DEV_NAME);
	} else {
		ret = alloc_chrdev_region(&foo_devno, foo_minor, foo_nr_devs, DEV_NAME); /* get devno */
		foo_major = MAJOR(foo_devno); /* get major */
	}
	if (ret < 0) {
		return -EINVAL;
	}
	ret = cdev_add(&foo_cdev, foo_devno, 1);
	if (ret < 0) {
		return -EINVAL;
	}
	foo_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(foo_class)) {
		return -EINVAL;
	}
	device_create(foo_class, NULL, foo_devno, NULL, DEV_NAME);

	return ret;
}

// driver
static struct platform_driver foo_driver = {
		.probe = foo_probe,
		.remove = foo_remove,
		.driver = {
				.name = DEV_NAME,
				.owner = THIS_MODULE, }, };

static int __init GPIO_init(void)
{
    int ret = 0;

    foo_device.num_resources = sizeof(foo_resource)/sizeof(struct resource);
    ret = platform_device_register(&foo_device);
	if (ret)
		return ret;

    ret = platform_driver_register(&foo_driver);
    if (ret)
        return ret;

    printk("zekezang: %s init\n",DEV_NAME);
    return ret;
}

static void __exit GPIO_exit(void)
{
    platform_driver_unregister(&foo_driver);
    platform_device_unregister(&foo_device);
}

module_init(GPIO_init);
module_exit(GPIO_exit);

MODULE_AUTHOR("Late Lee");
MODULE_DESCRIPTION("Simple platform driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:foo");
