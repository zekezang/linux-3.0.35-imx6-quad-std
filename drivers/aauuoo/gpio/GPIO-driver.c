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

#include <mach/gpio.h>
#include <mach/iomux-v3.h>
#include <mach/iomux-mx6q.h>


static iomux_v3_cfg_t mx6q_sabresd_GPIO[] = {
		MX6Q_PAD_SD1_CLK__GPIO_1_20,  /** SD1_CLK GPIO***/
};


#define CLASS_NAME "aauuoo_gpio"
#define DEV_NAME "sd1_clk_gpio"

#define DEVICE_MAJOR 248
#define FOO_NR_DEVS 1

static struct cdev foo_cdev;
static int foo_major = DEVICE_MAJOR;
static int foo_minor = 8;
static int foo_nr_devs = FOO_NR_DEVS;
static dev_t foo_devno;
static struct class* foo_class;
struct device *foo_dev;
static struct resource *resource;

static unsigned int GPIO_PIN = 0;

static void foo_dev_release(struct device* dev) {
}

static struct resource foo_resource[] = {
		[0] = {
				.start = IMX_GPIO_NR(1,20),
				.end = IMX_GPIO_NR(1,20),
				.flags = IORESOURCE_IO, }, };

static struct platform_device foo_device = {
		.name = DEV_NAME,
		.id = 1,
		.resource = foo_resource,
		.dev = {
				//.platform_data = &foo_pdata,
				.release = &foo_dev_release, }, };

static int foo_open(struct inode *inode, struct file *filp) {
	return 0;
}

static int foo_release(struct inode *inode, struct file *filp) {
	return 0;
}

static ssize_t foo_write(struct file *file, const char __user *buf, size_t count, loff_t *off) {
	char cmd[1];
	int ret;

	if (copy_from_user(cmd, buf, count)) {
		ret = -EFAULT;
	}

	gpio_request(GPIO_PIN, DEV_NAME);
	if (!memcmp(cmd, "1", 1)) {
		gpio_set_value(GPIO_PIN, 1);
	}

	if (!memcmp(cmd, "0", 1)) {
		gpio_set_value(GPIO_PIN, 0);
	}
	gpio_free(GPIO_PIN);

	return 0;
}

static ssize_t foo_read(struct file *file, char __user *user, size_t size, loff_t *off) {
	return 0;
}

static long foo_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	return 0;
}

static ssize_t foo_show(struct device *dev, struct device_attribute *attr, char *buf){
	int ret;
	gpio_request(GPIO_PIN, DEV_NAME);
	ret = gpio_get_value(GPIO_PIN);
	gpio_free(GPIO_PIN);
	if (ret == 0){
		strcpy(buf, "low\n");
	}else{
		strcpy(buf, "high\n");
	}
	return strlen(buf);
}

static ssize_t foo_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long value;
	int ret;

	ret = strict_strtoul(buf, 10, &value);
	if (ret)
		return ret;

	gpio_request(GPIO_PIN, DEV_NAME);
	if (value == 1) {
		gpio_set_value(GPIO_PIN, 1);
	}
	if (value == 0) {
		gpio_set_value(GPIO_PIN, 0);
	}
	gpio_free(GPIO_PIN);

	return count;
}
static DEVICE_ATTR(foo_state, S_IRUGO | S_IWUSR, foo_show, foo_store);

static struct file_operations foo_fops = {
		.owner = THIS_MODULE,
		.open = foo_open,
		.release = foo_release,
		.read = foo_read,
		.write = foo_write,
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
		GPIO_PIN = resource->start;
		gpio_request(GPIO_PIN, DEV_NAME);
		gpio_direction_output(GPIO_PIN, 1);
		gpio_set_value(GPIO_PIN, 0);
		gpio_free(GPIO_PIN);
	}

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
	foo_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(foo_class)) {
		return -EINVAL;
	}
	foo_dev = device_create(foo_class, NULL, foo_devno, NULL, DEV_NAME);
	if (foo_dev < 0){
		printk("cound not create sys node for foo_dev\n");
	}

	ret = device_create_file(foo_dev, &dev_attr_foo_state);
	if (ret < 0){
		printk("cound not create sys node for foo_state\n");
	}

	return ret;
}

// driver
static struct platform_driver foo_driver = {
		.probe = foo_probe,
		.remove = foo_remove,
		.driver = {
				.name = DEV_NAME,
				.owner = THIS_MODULE, }, };

static int __init GPIO_init(void) {
    int ret = 0;

    mxc_iomux_v3_setup_multiple_pads(mx6q_sabresd_GPIO, ARRAY_SIZE(mx6q_sabresd_GPIO));

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

static void __exit GPIO_exit(void) {
    platform_driver_unregister(&foo_driver);
    platform_device_unregister(&foo_device);
}

module_init(GPIO_init);
module_exit(GPIO_exit);

MODULE_AUTHOR("Late Lee");
MODULE_DESCRIPTION("Simple platform driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:foo");
