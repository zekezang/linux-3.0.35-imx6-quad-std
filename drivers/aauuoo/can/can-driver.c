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

static struct class* foo_class;
struct device *foo_dev;

static unsigned int GPIO_PIN = 0;


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


static int foo_remove(struct platform_device *dev) {
	unregister_chrdev_region(foo_devno, 1);
	cdev_del(&foo_cdev);
	device_destroy(foo_class, foo_devno);
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
