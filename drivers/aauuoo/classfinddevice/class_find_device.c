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
#include <linux/slab.h>

#define CLASS_NAME "test_find_devices"
#define DEV_NAME "zekezang"


static struct class* foo_class;
struct device *foo_dev;
static dev_t foo_devno;

struct addr_list {
	char dev_addr[8];
	struct list_head list;
};

struct addr_list _addr_list;
struct addr_list *_tmp;

void insert_addr_list(char *dev_adr, int dev_len){
	_tmp = (struct addr_list *) kmalloc(sizeof(struct addr_list), GFP_KERNEL);
	memcpy(_tmp->dev_addr, dev_adr, dev_len);
	list_add(&(_tmp->list), &(_addr_list.list));
}
//EXPORT_SYMBOL(insert_addr_list);

static ssize_t foo_show(struct device *dev, struct device_attribute *attr, char *buf){
	int ret;
	struct addr_list *xtmp;
	strcpy(buf, "low\n");

	list_for_each_entry(xtmp, &_addr_list.list, list) {
		printk("xtmp= %s \n", xtmp->dev_addr);
	}

	return strlen(buf);
}

static ssize_t foo_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long value;
	int ret;

	ret = strict_strtoul(buf, 10, &value);
	if (value == 1){
		printk("zekezang: to run device .......\n");
	}
	return count;
}
static DEVICE_ATTR(foo_state, S_IRUGO | S_IWUSR, foo_show, foo_store);


static int __init GPIO_init(void) {
    int ret = 0;

    INIT_LIST_HEAD(&_addr_list.list);

    foo_devno = MKDEV(248, 1);
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


    printk("zekezang: %s init\n",DEV_NAME);
    return ret;
}

static void __exit GPIO_exit(void) {

}

module_init(GPIO_init);
module_exit(GPIO_exit);

MODULE_AUTHOR("Late Lee");
MODULE_DESCRIPTION("Simple platform driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:foo");
