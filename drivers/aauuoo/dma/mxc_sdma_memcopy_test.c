/*
 * Copyright 2006-2012 Freescale Semiconductor, Inc. All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mman.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <mach/dma.h>

#include <linux/dmaengine.h>
#include <linux/device.h>

#include <linux/io.h>
#include <linux/delay.h>

static int gMajor; /* major number of device */
static struct class *dma_tm_class;
u32 *wbuf, *wbuf2, *wbuf3, *wbuf4;
u32 *rbuf, *rbuf2, *rbuf3, *rbuf4;

struct dma_chan *dma_m2m_chan;

struct completion dma_m2m_ok;

struct scatterlist sg[4], sg2[4];

#define SDMA_BUF_SIZE  1024*60



static bool dma_m2m_filter(struct dma_chan *chan, void *param)
{
	if (!imx_dma_is_general_purpose(chan))
		return false;
	chan->private = param;
	return true;
}

int sdma_open(struct inode *inode, struct file *filp)
{
	dma_cap_mask_t dma_m2m_mask;
	struct imx_dma_data m2m_dma_data;

	init_completion(&dma_m2m_ok);

	dma_cap_zero(dma_m2m_mask);
	dma_cap_set(DMA_SLAVE, dma_m2m_mask);
	m2m_dma_data.peripheral_type = IMX_DMATYPE_MEMORY;
	m2m_dma_data.priority = DMA_PRIO_HIGH;
	dma_m2m_chan = dma_request_channel(dma_m2m_mask, dma_m2m_filter, &m2m_dma_data);
	if (!dma_m2m_chan) {
		printk("Error opening the SDMA memory to memory channel\n");
		return -EINVAL;
	}

	//printk("########################%d\n", __LINE__);

	wbuf = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!wbuf) {
		printk("error wbuf !!!!!!!!!!!\n");
		return -1;
	}

	//printk("########################%d\n", __LINE__);

	wbuf2 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!wbuf2) {
		printk("error wbuf2 !!!!!!!!!!!\n");
		return -1;
	}

	wbuf3 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!wbuf3) {
		printk("error wbuf3 !!!!!!!!!!!\n");
		return -1;
	}

	wbuf4 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!wbuf4) {
		printk("error wbuf4 !!!!!!!!!!!\n");
		return -1;
	}

	rbuf = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!rbuf) {
		printk("error rbuf !!!!!!!!!!!\n");
		return -1;
	}

	rbuf2 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!rbuf2) {
		printk("error rbuf2 !!!!!!!!!!!\n");
		return -1;
	}

	rbuf3 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!rbuf3) {
		printk("error rbuf3 !!!!!!!!!!!\n");
		return -1;
	}

	rbuf4 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!rbuf4) {
		printk("error rbuf4 !!!!!!!!!!!\n");
		return -1;
	}
	//printk("########################%d\n", __LINE__);
	return 0;
}

int sdma_release(struct inode * inode, struct file * filp)
{
	dmaengine_terminate_all(dma_m2m_chan);
	dma_release_channel(dma_m2m_chan);
	dma_m2m_chan = NULL;
	kfree(wbuf);
	kfree(wbuf2);
	kfree(wbuf3);
	kfree(wbuf4);
	kfree(rbuf);
	kfree(rbuf2);
	kfree(rbuf3);
	kfree(rbuf4);
	return 0;
}

ssize_t sdma_read (struct file *filp, char __user * buf, size_t count,
								loff_t * offset)
{
	int i;

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf+i) != *(wbuf+i)) {
			printk("buffer 1 copy falled!\n");
			return 0;
		}
	}
	printk("buffer 1 copy passed!\n");

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf2+i) != *(wbuf2+i)) {
			printk("buffer 2 copy falled!\n");
			return 0;
		}
	}
	printk("buffer 2 copy passed!\n");

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf3+i) != *(wbuf3+i)) {
			printk("buffer 3 copy falled!\n");
			return 0;
		}
	}
	printk("buffer 3 copy passed!\n");

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf4+i) != *(wbuf4+i)) {
			printk("buffer 4 copy falled!\n");
			return 0;
		}
	}
	printk("buffer 4 copy passed!\n");

	return 0;
}

static void dma_m2m_callback(void *data)
{
	complete(&dma_m2m_ok);
	return ;
}

ssize_t sdma_write(struct file * filp, const char __user * buf, size_t count, loff_t * offset)
{
	u32 *index1, *index2, *index3, *index4, i, ret;
	struct dma_slave_config dma_m2m_config;
	struct dma_async_tx_descriptor *dma_m2m_desc;
	struct timeval end_time;
	unsigned long end, start;
	index1 = wbuf;
	index2 = wbuf2;
	index3 = wbuf3;
	index4 = wbuf4;

	//printk("########################%d\n", __LINE__);

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		*(index1 + i) = 0x12121212;
	}

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		*(index2 + i) = 0x34343434;
	}

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		*(index3 + i) = 0x56565656;
	}

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		*(index4 + i) = 0x78787878;
	}

	//printk("########################%d\n", __LINE__);

	dma_m2m_config.direction = DMA_MEM_TO_MEM;
	//printk("########################%d\n", __LINE__);
	dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	//printk("########################%d\n", __LINE__);
	dmaengine_slave_config(dma_m2m_chan, &dma_m2m_config);

	//printk("########################%d\n", __LINE__);

	sg_init_table(sg, 4);
	sg_set_buf(&sg[0], wbuf, SDMA_BUF_SIZE);
	sg_set_buf(&sg[1], wbuf2, SDMA_BUF_SIZE);
	sg_set_buf(&sg[2], wbuf3, SDMA_BUF_SIZE);
	sg_set_buf(&sg[3], wbuf4, SDMA_BUF_SIZE);
	ret = dma_map_sg(NULL, sg, 4, dma_m2m_config.direction);

	//printk("########################%d\n", __LINE__);

	sg_init_table(sg2, 4);
	sg_set_buf(&sg2[0], rbuf, SDMA_BUF_SIZE);
	sg_set_buf(&sg2[1], rbuf2, SDMA_BUF_SIZE);
	sg_set_buf(&sg2[2], rbuf3, SDMA_BUF_SIZE);
	sg_set_buf(&sg2[3], rbuf4, SDMA_BUF_SIZE);
	ret = dma_map_sg(NULL, sg2, 4, dma_m2m_config.direction);

	//printk("########################%d\n", __LINE__);

	dma_m2m_desc = dma_m2m_chan->device->device_prep_dma_sg(dma_m2m_chan, sg2, 4, sg, 4, 0);
	//printk("########################%d\n", __LINE__);
	dma_m2m_desc->callback = dma_m2m_callback;
	//printk("########################%d\n", __LINE__);

	do_gettimeofday(&end_time);
	start = end_time.tv_sec*1000000 + end_time.tv_usec;

	//printk("########################%d\n", __LINE__);

	dmaengine_submit(dma_m2m_desc);
	dma_async_issue_pending(dma_m2m_chan);

	//printk("########################%d\n", __LINE__);

	wait_for_completion(&dma_m2m_ok);

	//printk("########################%d\n", __LINE__);
	do_gettimeofday(&end_time);
	end = end_time.tv_sec*1000000 + end_time.tv_usec;
	printk("end - start = %lu\n", end - start);

	dma_unmap_sg(NULL, sg, 4, dma_m2m_config.direction);
	dma_unmap_sg(NULL, sg2, 4, dma_m2m_config.direction);
	//printk("########################%d\n", __LINE__);

	return 0;
}

struct file_operations dma_fops = {
	open:		sdma_open,
	release:	sdma_release,
	read:		sdma_read,
	write:		sdma_write,
};

int __init sdma_init_module(void)
{
	struct device *temp_class;
	int error;

	/* register a character device */
	error = register_chrdev(0, "sdma_test", &dma_fops);
	if (error < 0) {
		printk("SDMA test driver can't get major number\n");
		return error;
	}
	gMajor = error;
	printk("SDMA test major number = %d\n",gMajor);

	dma_tm_class = class_create(THIS_MODULE, "sdma_test");
	if (IS_ERR(dma_tm_class)) {
		printk(KERN_ERR "Error creating sdma test module class.\n");
		unregister_chrdev(gMajor, "sdma_test");
		return PTR_ERR(dma_tm_class);
	}

	temp_class = device_create(dma_tm_class, NULL, MKDEV(gMajor, 0), NULL, "sdma_test");
	if (IS_ERR(temp_class)) {
		printk(KERN_ERR "Error creating sdma test class device.\n");
		class_destroy(dma_tm_class);
		unregister_chrdev(gMajor, "sdma_test");
		return -1;
	}

	printk("SDMA test Driver Module loaded\n");
	return 0;
}

static void sdma_cleanup_module(void)
{
	unregister_chrdev(gMajor, "sdma_test");
	device_destroy(dma_tm_class, MKDEV(gMajor, 0));
	class_destroy(dma_tm_class);

	printk("SDMA test Driver Module Unloaded\n");
}


module_init(sdma_init_module);
module_exit(sdma_cleanup_module);

MODULE_AUTHOR("Freescale Semiconductor");
MODULE_DESCRIPTION("SDMA test driver");
MODULE_LICENSE("GPL");
