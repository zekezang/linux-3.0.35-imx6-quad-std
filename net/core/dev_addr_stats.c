#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>

#define MAX(a,b) ((a>b)?(a):(b))

struct addr_list {
	char dev_name[IFNAMSIZ];
	char dev_addr[8];
	struct list_head list;
};

struct addr_list _addr_list;
struct addr_list *_tmp;
struct addr_list *xtmp;

static struct class* dev_addr_class;
struct device *dev_addr_dev;
static dev_t dev_addr_devno;
#define CLASS_NAME "defend"
#define DEV_NAME "dev_addr"
static char split_char = ':';

static char dev_selected[32];
static char dev_xor_keygen[32];
static char dev_selected_addr[32];

ssize_t dev_addr_show(struct device *dev, struct device_attribute *attr, char *buf){

	int i;
	u8 tmp_addr_value;
	int keygen_len, addr_len, max_len;
	char *target_value_str;
	char format_str[2];
	memset(dev_selected_addr, 0, 32);
	memset(format_str, 0, 2);

	list_for_each_entry(xtmp, &_addr_list.list, list) {
//		printk("xtmp= %s %02X:%02X:%02X:%02X:%02X:%02X \n",xtmp->dev_name,
//				xtmp->dev_addr[0],xtmp->dev_addr[1],xtmp->dev_addr[2],
//				xtmp->dev_addr[3],xtmp->dev_addr[4],xtmp->dev_addr[5]);
		if(!memcmp(xtmp->dev_name, dev_selected, strlen(dev_selected))){
//			printk("####selected %s %02X:%02X:%02X:%02X:%02X:%02X  len:%d\n",xtmp->dev_name,
//					xtmp->dev_addr[0],xtmp->dev_addr[1],xtmp->dev_addr[2],
//					xtmp->dev_addr[3],xtmp->dev_addr[4],xtmp->dev_addr[5],strlen(xtmp->dev_addr));
			memcpy(dev_selected_addr, xtmp->dev_addr, strlen(xtmp->dev_addr));
			break;
		}
	}

	target_value_str = (char*)kmalloc(32, GFP_KERNEL);
	memset(target_value_str, 0, 32);

	keygen_len = strlen(dev_xor_keygen);
	addr_len = strlen(dev_selected_addr);
	max_len = MAX(keygen_len,addr_len);

	//printk("######keygen_len:%d addr_len:%d max_len:%d\n",keygen_len,addr_len,max_len);

	for(i=0; i<max_len; i++){
		//printk("######dev_xor_keygen:%02X dev_selected_addr:%02X\n",dev_xor_keygen[i], dev_selected_addr[i]);
		tmp_addr_value = dev_xor_keygen[i] ^ dev_selected_addr[i];
		sprintf(format_str, "%02X", tmp_addr_value);
		//printk("xtmp= %s\n",format_str);
		target_value_str = strncat(target_value_str, format_str, 2);
		//printk("%p target_value_str= %s\n",target_value_str, target_value_str);
	}

	target_value_str = strncat(target_value_str, "\0\n",2);

	//printk("target_value_str=len :%d\n",strlen(target_value_str));

	strncpy(buf, target_value_str, strlen(target_value_str));

	kfree(target_value_str);
	return strlen(buf);
}

ssize_t dev_addr_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char *split_index;

	memset(dev_selected, 0, 32);
	memset(dev_xor_keygen, 0, 32);

	split_index = strstr(buf, &split_char);
	if(split_index){
		strncpy(dev_selected, buf, split_index-buf);
		strncpy(dev_xor_keygen, split_index+1, strlen(buf)-(split_index-buf)-2);
	}else{ //wlan0 default checked
		strncpy(dev_selected, "wlan0", 5);
		strncpy(dev_xor_keygen, buf, strlen(buf)-1);
	}

	//printk("to run device dev_selected:%s##\n", dev_selected);
	//printk("to run device dev_xor_keygen:%s##\n", dev_xor_keygen);

//	ret = strict_strtoul(buf, 10, &value);

	return count;
}
DEVICE_ATTR(mac_defend, S_IRUGO | S_IWUGO, dev_addr_show, dev_addr_store);

void insert_addr_list(struct net_device *dev){

	list_for_each_entry(xtmp, &_addr_list.list, list) {
		if(!memcmp(xtmp->dev_addr, dev->dev_addr, dev->addr_len)){
			return;
		}
		if(!memcmp(xtmp->dev_name, dev->name, strlen(dev->name))){
			return;
		}
	}

	_tmp = (struct addr_list *) kmalloc(sizeof(struct addr_list), GFP_KERNEL);
	memset(_tmp, 0, sizeof(struct addr_list));

//	printk("xtmp= %s %02X:%02X:%02X:%02X:%02X:%02X \n",dev->name,
//			dev->dev_addr[0],dev->dev_addr[1],dev->dev_addr[2],
//			dev->dev_addr[3],dev->dev_addr[4],dev->dev_addr[5]);
//	printk("##################dev->addr_len:%d\n",dev->addr_len);

	memcpy(_tmp->dev_addr, dev->dev_addr, dev->addr_len);
	memcpy(_tmp->dev_name, dev->name, IFNAMSIZ);
	list_add(&(_tmp->list), &(_addr_list.list));
}
EXPORT_SYMBOL(insert_addr_list);

int dev_addr_stats_init(void) {
    int ret = 0;

    memset(&_addr_list, 0, sizeof(struct addr_list));
    INIT_LIST_HEAD(&_addr_list.list);

    dev_addr_devno = MKDEV(248, 1);
	dev_addr_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(dev_addr_class)) {
		return -EINVAL;
	}
	dev_addr_dev = device_create(dev_addr_class, NULL, dev_addr_devno, NULL, DEV_NAME);
	if (dev_addr_dev < 0){
		printk("cound not create sys node for dev_addr_dev\n");
	}

	ret = device_create_file(dev_addr_dev, &dev_attr_mac_defend);
	if (ret < 0){
		printk("cound not create sys node for dev_addr_state\n");
	}


    printk("zekezang: %s init\n",DEV_NAME);
    return ret;
}

EXPORT_SYMBOL(dev_addr_stats_init);
