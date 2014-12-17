/*
 *  Created on: 2012-5-16
 *      Author: zekezang
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gpio.h>

#define DEVICE_NAME     "GPH32_EINT"

static int dev_id = 9;

static wait_queue_head_t zekezang_waitq;

static volatile int press = 0;

static irqreturn_t zekezang_interrupt(int irq, void *dev_id) {

	unsigned long data;
	int tmp;

	mdelay(15);

	data = s3c2410_gpio_getpin(S3C2410_GPG(0));

	tmp = !(data & (1 << 0));

	if (tmp) {
		wake_up_interruptible(&zekezang_waitq);
		press = 1;
	}

	//printk("zekezangs_interrupt - data:%lu---tmp:%d\n", data, tmp);

	return IRQ_RETVAL(IRQ_HANDLED);
}

static char* rets;

static int my_zekezang_open(struct inode *inode, struct file *file) {

	int err = 0;

	err = request_irq(S5P_EXT_INT3(2), zekezang_interrupt, IRQ_TYPE_EDGE_BOTH, "zekezang-read-int", (void*) dev_id);

	if (err) {
		disable_irq(IRQ_EINT8);
		free_irq(IRQ_EINT8, (void*) dev_id);
		return err;
	}

	rets = (char*) kmalloc(sizeof(char) * 1, GFP_KERNEL);
	strcpy(rets, "1");

	//printk("my_zekezangs_open############################\n");

	return 0;
}

static int my_zekezang_close(struct inode *inode, struct file *file) {

	disable_irq(IRQ_EINT8);
	free_irq(IRQ_EINT8, (void*) dev_id);
	return 0;
}

static int my_zekezang_read(struct file *filp, char __user *buff, size_t count, loff_t *offp) {
	int err = 0;

	if (!press) {
		if (filp->f_flags & O_NONBLOCK) {
			return -EAGAIN;
		} else {
			wait_event_interruptible(zekezang_waitq, press);
		}
	}
	press = 0;
	err = copy_to_user(buff, (void *) rets, 1);
	return err;
}

static unsigned int my_zekezang_poll(struct file *file, struct poll_table_struct *wait) {
	unsigned int mask = 0;
	poll_wait(file, &zekezang_waitq, wait);

	if (press)
		mask |= POLLIN | POLLRDNORM;
	return mask;
}

static struct file_operations dev_fops = {
		.owner = THIS_MODULE,
		.open = my_zekezang_open,
		.release = my_zekezang_close,
		.read = my_zekezang_read,
		.poll = my_zekezang_poll, };

static struct miscdevice misc = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = DEVICE_NAME,
		.fops = &dev_fops,
		.mode = S_IRWXU | S_IRWXG | S_IRWXO, };

static int __init dev_init(void)
{
	int ret;

	init_waitqueue_head(&zekezang_waitq);

	ret = misc_register(&misc);

	printk (DEVICE_NAME" \tinitialized\n");

	return ret;
}

static void __exit dev_exit(void)
{
	misc_deregister(&misc);
	printk("exit\n");
}

module_init( dev_init);
module_exit( dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zekezang Inc.");

