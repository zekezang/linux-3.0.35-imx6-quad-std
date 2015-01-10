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
#include <linux/io.h>

#include <mach/gpio.h>
#include <mach/iomux-v3.h>
#include <mach/iomux-mx6q.h>
#include <linux/mutex.h>

#define flex_can_0_io_base 0x02090000

#define CLASS_NAME "aauuoo_can"
#define DEV_NAME "flex_can_0"

#define FLEXCAN_ATTR_MAX 18

enum {
	FLEXCAN_ATTR_STATE = 0,
	FLEXCAN_ATTR_MCR,
	FLEXCAN_ATTR_CTRL,
	FLEXCAN_ATTR_TIMER,
	FLEXCAN_ATTR_RXGMASK,
	FLEXCAN_ATTR_RX14MASK,
	FLEXCAN_ATTR_RX15MASK,
	FLEXCAN_ATTR_ECR,
	FLEXCAN_ATTR_ESR,
	FLEXCAN_ATTR_IMASK2,
	FLEXCAN_ATTR_IMASK1,
	FLEXCAN_ATTR_IFLAG2,
	FLEXCAN_ATTR_IFLAG1,
	FLEXCAN_ATTR_CRL2,
	FLEXCAN_ATTR_ESR2,
	FLEXCAN_ATTR_CRCR,
	FLEXCAN_ATTR_RXFGMASK,
	FLEXCAN_ATTR_RXFIR,
};

/* Structure of the message buffer */
struct flexcan_mb {
	u32 can_ctrl;
	u32 can_id;
	u32 data[2];
};

/* Structure of the hardware registers */
struct flexcan_regs {
	u32 mcr;		/* 0x00 */
	u32 ctrl;		/* 0x04 */
	u32 timer;		/* 0x08 */
	u32 _reserved1;		/* 0x0c */
	u32 rxgmask;		/* 0x10 */
	u32 rx14mask;		/* 0x14 */
	u32 rx15mask;		/* 0x18 */
	u32 ecr;		/* 0x1c */
	u32 esr;		/* 0x20 */
	u32 imask2;		/* 0x24 */
	u32 imask1;		/* 0x28 */
	u32 iflag2;		/* 0x2c */
	u32 iflag1;		/* 0x30 */
	u32 crl2;		/* 0x34 */
	u32 esr2;		/* 0x38 */
	u32 _reserved2[2];
	u32 crcr;		/* 0x44 */
	u32 rxfgmask;		/* 0x48 */
	u32 rxfir;		/* 0x4c */
	u32 _reserved3[12];
	struct flexcan_mb cantxfg[64];
};

static struct class* foo_class;
static struct device *foo_dev;

static struct flexcan_regs __iomem *regs_base;

static unsigned int tmp_value = 0;
static void foo_dev_release(struct device* dev) { }
static struct mutex attr_lock;

static ssize_t flexcan_show_attr(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t flexcan_set_attr(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

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

static struct device_attribute flexcan_dev_attr[FLEXCAN_ATTR_MAX] = {
	[FLEXCAN_ATTR_STATE] = __ATTR(flexcan_state, 0444, foo_show, foo_store),
	[FLEXCAN_ATTR_MCR] = __ATTR(flexcan_mcr, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_CTRL] = __ATTR(flexcan_ctrl, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_TIMER] = __ATTR(flexcan_timer, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_RXGMASK] = __ATTR(flexcan_rxgmask, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_RX14MASK] = __ATTR(flexcan_rx14mask, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_RX15MASK] = __ATTR(flexcan_rx15mask, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_ECR] = __ATTR(flexcan_ecr, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_ESR] = __ATTR(flexcan_esr, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_IMASK2] = __ATTR(flexcan_imask2, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_IMASK1] = __ATTR(flexcan_imask1, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_IFLAG2] = __ATTR(flexcan_iflag2, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_IFLAG1] = __ATTR(flexcan_iflag1, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_CRL2] = __ATTR(flexcan_crl2, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_ESR2] = __ATTR(flexcan_esr2, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_CRCR] = __ATTR(flexcan_crcr, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_RXFGMASK] = __ATTR(flexcan_rxfgmask, 0644, flexcan_show_attr, flexcan_set_attr),
	[FLEXCAN_ATTR_RXFIR] = __ATTR(flexcan_rxfir, 0644, flexcan_show_attr, flexcan_set_attr),
};

static ssize_t flexcan_show_attr(struct device *dev, struct device_attribute *attr, char *buf)
{
	int attr_id;
	u32 reg;

	attr_id = attr - flexcan_dev_attr;

	switch (attr_id) {
	case FLEXCAN_ATTR_MCR:
		reg = readl(&regs_base->mcr);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_CTRL:
		reg = readl(&regs_base->ctrl);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_TIMER:
		reg = readl(&regs_base->timer);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_RXGMASK:
		reg = readl(&regs_base->rxgmask);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_RX14MASK:
		reg = readl(&regs_base->rx14mask);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_RX15MASK:
		reg = readl(&regs_base->rx15mask);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_ECR:
		reg = readl(&regs_base->ecr);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_ESR:
		reg = readl(&regs_base->esr);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_IMASK2:
		reg = readl(&regs_base->imask2);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_IMASK1:
		reg = readl(&regs_base->imask1);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_IFLAG2:
		reg = readl(&regs_base->iflag2);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_IFLAG1:
		reg = readl(&regs_base->iflag1);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_CRL2:
		reg = readl(&regs_base->crl2);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_ESR2:
		reg = readl(&regs_base->esr2);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_CRCR:
		reg = readl(&regs_base->crcr);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_RXFGMASK:
		reg = readl(&regs_base->rxfgmask);
		return sprintf(buf, "%08X\n", reg);
	case FLEXCAN_ATTR_RXFIR:
		reg = readl(&regs_base->rxfir);
		return sprintf(buf, "%08X\n", reg);
	default:
		return sprintf(buf, "%s:%p->%p\n", __func__, flexcan_dev_attr, attr);
	}
}


static ssize_t flexcan_set_attr(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int attr_id, tmp;

	attr_id = attr - flexcan_dev_attr;

	mutex_lock(&attr_lock);

	tmp = simple_strtoul(buf, NULL, 0);
	switch (attr_id) {
	case FLEXCAN_ATTR_MCR:
		printk("########can-driver: FLEXCAN_ATTR_MCR\n");
		break;
	}

	mutex_unlock(&attr_lock);
	return count;
}


static int foo_remove(struct platform_device *dev) {
	class_destroy(foo_class);
	return 0;
}

static int foo_probe(struct platform_device *dev) {
	int ret = 0, err;
	int i, num;
	u32 reg;

	// because flexcan driver already mapped this io
//	if (!request_mem_region(flex_can_0_io_base, SZ_16K, DEV_NAME)) {
//		err = -EBUSY;
//		goto failed_get;
//	}

	regs_base = ioremap(flex_can_0_io_base, SZ_16K);
	if (!regs_base) {
		err = -ENOMEM;
		goto failed_map;
	}

	reg = readl(&regs_base->mcr);
	//printk("########can-driver: %d %04X\n", __LINE__, reg);

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


	num = ARRAY_SIZE(flexcan_dev_attr);
	for (i = 0; i < num; i++) {
		if (device_create_file(foo_dev, flexcan_dev_attr + i)) {
			printk(KERN_ERR "Create attribute file fail!\n");
			ret = -1;
			break;
		}
	}
	if (i != num) {
		for (; i >= 0; i--)
			device_remove_file(foo_dev, flexcan_dev_attr + i);
	}

	return ret;

failed_map:
	release_mem_region(flex_can_0_io_base, SZ_16K);
//failed_get:
	return err;
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

    mutex_init(&attr_lock);

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
