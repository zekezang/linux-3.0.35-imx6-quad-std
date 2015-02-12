/*
 I'm writing a Linux kernel driver that must transfer up to 512KB of data from an FPGA (connected via the i.mx53's EIM) to a memory buffer.
 I have it working with memcpy() right now, but need to use DMA to offload the copying from the CPU.
 The FPGA has been mapped such that it can be read as a 512KB contiguous region of memory, and the EIM is set up so that it does not do burst/page accesses to the FPGA.
 The 512KB destination buffer has been allocated using dma_alloc_coherent(), and the FPGA's read address range has been made available using dev_ioremap_nocache().
 I  would like to use the generic Linux DMA layer (the dmaengine) rather than talk to the imx-sdma driver for best portability in the future.

 However, my attempts to get DMA working have been met with kernel crashes and/or a lack of access to the FPGA (I have an oscilloscope connected that will show when the FPGA RAM addresses are accessed).

 I'm using the 2.6.38 kernel as provided by the FSL LTIB BSP, if it matters.

 This is a greatly simplified version of my code (with error handling and other stuff removed) that shows the DMA-related operations.
 The production code checks all error and return codes, of course.

 Ned Konz, ned@productcreationstudio.com
 */

#define SAMPLE_BUFFER_SIZE (512*1024)
#define FPGA_RAMBASE 0xF2000000

struct mymodule_info {
	struct device *dev;

	/* FPGA addresses */
	unsigned long phys_base; /* main FPGA physical base address */
	void __iomem *baseaddr; /* main FPGA virtual base address */
	u16 __iomem *fpga_rambase; /* FPGA RAM base address */

	void __iomem *sample_buffer; /* virtual base address of sample RAM */
	dma_addr_t dma_handle; /* physical address of sample_buffer */
	struct dma_chan *dma_channel; /* allocated by mymodule_dma_alloc() */
	struct sg_table dma_sgtable;
	unsigned dma_sg_len;	/* length of dma_sg[] */
	unsigned dma_sg_used;	/* number we're using for DMA */
	struct dma_async_tx_descriptor *dma_txd;
};

static struct mymodule_info *s_mymodule_info;

static bool mymodule_dma_filter(struct dma_chan *chan, void *param)
{
	if (!imx_dma_is_general_purpose(chan))
		return false;
	chan->private = param;
	return true;
}

/* Allocate a channel for a single DMA request line */
static struct dma_chan *mymodule_dma_alloc(u32 dma_req)
{
	dma_cap_mask_t mask;
	struct imx_dma_data dma_data;
	struct dma_chan *chan;

	dma_data.peripheral_type = IMX_DMATYPE_MEMORY;
	dma_data.priority = DMA_PRIO_MEDIUM;
	dma_data.dma_request = dma_req;

	/* Try to grab a DMA channel */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);
	chan = dma_request_channel(mask, mymodule_dma_filter, &dma_data);

	return chan;
}

static struct dma_async_tx_descriptor *mymodule_dma_config(struct mymodule_info *info,
		u32 dma_addr, void *buf_addr, unsigned int buf_len)
{
	struct dma_slave_config slave_config;
	int ret, i;
	struct dma_async_tx_descriptor *desc = NULL;
	struct scatterlist *sg;
	void *bufp;
	unsigned int remaining;

	if (info->dma_channel == NULL)
		goto err_nochannel;

	slave_config.direction = DMA_FROM_DEVICE;
	slave_config.src_addr = dma_addr;	/* physical address */
	slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	slave_config.src_maxburst = 1; /* SDMA watermark_level */

	ret = dmaengine_slave_config(info->dma_channel, &slave_config);
	if (ret) {
		dev_err(info->dev, "dmaengine_slave_config returns %d\n", ret);
		goto err_slaveconfig;
	}

	info->dma_sg_len = (buf_len + (PAGE_SIZE >> 1)) >> PAGE_SHIFT;

	ret = sg_alloc_table(&info->dma_sgtable, info->dma_sg_len, GFP_KERNEL);
	if (ret) {
		dev_err(info->dev, "sg_alloc_table(%d) returns %d\n", info->dma_sg_len, ret);
		goto err_alloc_table;
	}

	remaining = buf_len;
	bufp = buf_addr;
	BUG_ON(offset_in_page(bufp));

	for_each_sg(info->dma_sgtable.sgl, sg, info->dma_sg_len, i) {
		unsigned int mapbytes;
		if (remaining < PAGE_SIZE)
			mapbytes = remaining;
		else
			mapbytes = PAGE_SIZE;
		sg_set_buf(sg, bufp, mapbytes);
		bufp += mapbytes;
		remaining -= mapbytes;
	}

	info->dma_sg_used = dma_map_sg(info->dma_channel->device->dev,
	                info->dma_sgtable.sgl, info->dma_sg_len,
	                DMA_FROM_DEVICE);
	if (!info->dma_sg_used) {
		dev_err(info->dev, "dma_map_sg failed\n");
		goto err_map_sg;
	}

	desc = info->dma_channel->device->device_prep_slave_sg(
	                info->dma_channel, info->dma_sgtable.sgl, info->dma_sg_used,
	                DMA_FROM_DEVICE, DMA_PREP_INTERRUPT | DMA_COMPL_SKIP_SRC_UNMAP | DMA_CTRL_ACK);
	if (desc)
		return desc;

	dev_err(info->dev, "device_prep_slave_sg failed\n");
	dma_unmap_sg(info->dma_channel->device->dev, info->dma_sgtable.sgl, info->dma_sg_len, DMA_FROM_DEVICE);
err_map_sg:
	sg_free_table(&info->dma_sgtable);
err_alloc_table:
err_slaveconfig:
err_nochannel:
	return NULL;
}

/* This is being called at the end of the first DMA operation: */
static void mymodule_dma_done_callback(void *param)
{
	struct mymodule_info *info = (struct mymodule_info *)param;
	dma_sync_sg_for_cpu(info->dma_channel->device->dev, info->dma_sgtable.sgl, info->dma_sg_len, DMA_FROM_DEVICE);
	dma_unmap_sg(info->dma_channel->device->dev, info->dma_sgtable.sgl, info->dma_sg_len, DMA_FROM_DEVICE);
}

/* This is called to actually do the data transfer: */
static int mymodule_transfer_data_dma(struct mymodule_info *info, unsigned long total_words)
{
	struct dma_async_tx_descriptor *desc;
	int retval = 0;
	enum dma_status dmastat;
	dma_cookie_t cookie;

	desc = mymodule_dma_config(info, FPGA_RAM_DATA_START, info->sample_buffer,
	                total_words * 2);
	if (desc) {
		desc->callback = mymodule_dma_done_callback;
		desc->callback_param = info;
		cookie = desc->tx_submit(desc);
		if (dma_submit_error(cookie)) {
			dev_err(info->dev, "tx_submit error\n");
			retval = -EINVAL;
		}

		dmastat = dma_wait_for_async_tx(desc);
		if (dmastat != DMA_SUCCESS) {
			retval = -EINVAL;
		}
	} else {
		dev_err(info->dev, "mymodule_dma_config failed\n");
		retval = -EINVAL;
	}
	mymodule_transfer_done(info);
	return retval;
}

static int mymodule_open(struct inode *in, struct file *fi)
{
	struct mymodule_info *info;
	unsigned long flags;

	fi->private_data = info = s_mymodule_info;

	if (!no_dma) {
		struct dma_chan *dma_channel;
		dma_channel = mymodule_dma_alloc(0);

		if (dma_channel == NULL) {
			no_dma = 1;
			dev_err(info->dev,
					"mymodule_dma_alloc failed; not using DMA\n");
		} else {
			info->dma_channel = dma_channel;
		}
	}

	return 0;
}

static int mymodule_probe(struct platform_device *pdev)
{
	info->fpga_rambase = devm_ioremap_nocache(&pdev->dev, FPGA_RAMBASE, SAMPLE_BUFFER_SIZE);
	                                          
	err = dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));
	if (err) {
		err = dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(32));
		if (err) {
			dev_err(&pdev->dev, "No usable DMA configuration, setting no_dma=1\n");
			goto error;
		}
	}

	info->sample_buffer = dmam_alloc_coherent(&pdev->dev, SAMPLE_BUFFER_SIZE, &info->dma_handle, GFP_DMA | GFP_KERNEL);
}
	                                                                                     
