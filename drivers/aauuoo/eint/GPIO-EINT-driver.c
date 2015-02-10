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
#include <mach/gpio.h>
#include <mach/iomux-v3.h>
#include <mach/iomux-mx6q.h>


static iomux_v3_cfg_t mx6q_sabresd_GPIO[] = {
		//MX6Q_PAD_SD1_CLK__GPIO_1_20,  /** SD1_CLK GPIO***/
		MX6Q_PAD_CSI0_DATA_EN__GPIO_5_20,
};

#define DEV_NAME "GPIO_EINT"

static struct resource *resource;
static int dev_id = 10;
static unsigned int EINT_PIN = 0;

static void *GPIO5_int_status_register;

static void foo_dev_release(struct device* dev) {
	disable_irq(EINT_PIN);
	free_irq(EINT_PIN, (void*) dev_id);
}

static struct resource foo_resource[] = {
		[0] = {
				.start = IMX_GPIO_NR(5,20),
				.end = IMX_GPIO_NR(5,20),
				.flags = IORESOURCE_IRQ, }, };

static struct platform_device foo_device = {
		.name = DEV_NAME,
		.id = 1,
		.resource = foo_resource,
		.dev = {
				//.platform_data = &foo_pdata,
				.release = &foo_dev_release, }, };

static irqreturn_t zekezang_interrupt(int irq, void *dev_id) {
	unsigned long data;
	unsigned long value = 0;

	value = readl(GPIO5_int_status_register);

	printk("###############value:%08X\n",value);

	mdelay(15);
	data = gpio_get_value(IMX_GPIO_NR(5,20));
	if (!data) {
		printk("###############data:%lu\n",data);
	}else{
		printk("###############data:%lu\n",data);
	}
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int foo_remove(struct platform_device *dev) {
	disable_irq(EINT_PIN);
	free_irq(EINT_PIN, (void*) dev_id);
	return 0;
}

static int foo_probe(struct platform_device *dev) {
	int ret = 0;
	unsigned long value = 0;
	resource = platform_get_resource(dev, IORESOURCE_IRQ, 0);
	if (resource->flags == IORESOURCE_IRQ) {

		GPIO5_int_status_register = ioremap(0x020AC018, SZ_4);
		if (!GPIO5_int_status_register) {
			ret = -ENOMEM;
			goto failed_map;
		}
		value = readl(GPIO5_int_status_register);

		value |= 1<<20,

		writel(value, GPIO5_int_status_register);

		printk("###############value:%08X\n",value);

		EINT_PIN = gpio_to_irq(resource->start);

		printk("###############EINT_PIN:%d\n",EINT_PIN);
		ret = request_irq(EINT_PIN, zekezang_interrupt, IRQ_TYPE_EDGE_BOTH, DEV_NAME, (void*) dev_id);
		if (ret) {
			disable_irq(EINT_PIN);
			free_irq(EINT_PIN, (void*) dev_id);
			return ret;
		}
	}

	failed_map:
		release_mem_region(0x020AC018, SZ_4);

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
