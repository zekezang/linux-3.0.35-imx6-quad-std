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


#define CLASS_NAME "aauuoo_can"
#define DEV_NAME "flex_can"

static struct class* foo_class;
static struct device *foo_dev;

static unsigned int tmp_value = 0;
static void foo_dev_release(struct device* dev) { }

static struct platform_device foo_device = {
		.name = DEV_NAME,
		.id = 1,
		.dev = {
				//.platform_data = &foo_pdata,
				.release = &foo_dev_release, }, };

static ssize_t foo_show(struct device *dev, struct device_attribute *attr, char *buf){

	if (tmp_value == 0){
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

	if (value == 1) {
		tmp_value = 1;
	}
	if (value == 0) {
		tmp_value = 0;
	}

	return count;
}
static DEVICE_ATTR(foo_state, S_IRUGO | S_IWUSR, foo_show, foo_store);


static int foo_remove(struct platform_device *dev) {
	class_destroy(foo_class);
	return 0;
}

static int foo_probe(struct platform_device *dev) {
	int ret = 0;

	foo_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(foo_class)) {
		return -EINVAL;
	}
	foo_dev = device_create(foo_class, NULL, dev->dev.devt, NULL, DEV_NAME);
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

static int __init foo_init(void) {
    int ret = 0;

    ret = platform_device_register(&foo_device);
	if (ret)
		return ret;

    ret = platform_driver_register(&foo_driver);
    if (ret)
        return ret;

    printk("zekezang: %s init\n",DEV_NAME);
    return ret;
}

static void __exit foo_exit(void) {
    platform_driver_unregister(&foo_driver);
    platform_device_unregister(&foo_device);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_AUTHOR("Late Lee");
MODULE_DESCRIPTION("Simple platform driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:foo");
