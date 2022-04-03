#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/fs.h>
#include "./m74hc595.h"

#define NAME "m74hc595"
#define GET_CMD_SIZE(cmd) ((cmd>>16) & 0x3FFF)
int major = 0;
struct class *cls;
struct device *dev;
struct spi_device *myspi;

int m74hc595_open(struct inode *inode, struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}
long m74hc595_ioctl(struct file *file,unsigned int cmd, unsigned long args)
{
	unsigned char buf[2] = {0};
	copy_from_user(buf,(void *)args,GET_CMD_SIZE(cmd));

	switch(cmd){
	case SPI_DATA_2:
		spi_write(myspi,buf,2);
		break;
	default:
		printk("ioctl error\n");
		break;
	}
	return 0;
}
int m74hc595_close(struct inode *inode, struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}
struct file_operations fops = {
	.open = m74hc595_open,
	.unlocked_ioctl = m74hc595_ioctl,
	.release = m74hc595_close,
};
int m74hc595_probe(struct spi_device *spi)
{
	printk("%s:%d\n",__func__,__LINE__);
	myspi = spi;
	major =register_chrdev(0,NAME,&fops);
	if(major < 0){
		printk("%d:register_chrdev error\n",__LINE__);
		return major;
	}
	cls = class_create(THIS_MODULE,NAME);
	if(IS_ERR(cls)){
		printk("class create  error\n");
		return PTR_ERR(cls);
	}
	dev = device_create(cls,NULL,MKDEV(major,0),NULL,NAME);
	if(IS_ERR(dev)){
		printk("device create  error\n");
		return PTR_ERR(dev);
	}
	return 0;
}
int	m74hc595_remove(struct spi_device *spi)
{
	printk("%s:%d\n",__func__,__LINE__);
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major,NAME);
	return 0;
}

const struct of_device_id of_match[] = {
	{.compatible = "HQYJ,m74hc595",},
	{},
};
MODULE_DEVICE_TABLE(of,of_match);

const struct spi_device_id id_match[] = {
	{"HQYJ,m74hc595",0},
	{},
};
MODULE_DEVICE_TABLE(spi,id_match);

struct spi_driver m74hc595 = {
	.probe = m74hc595_probe,
	.remove = m74hc595_remove,
	.driver = {
		.name = "my_m74hc595",
		.of_match_table = of_match,
	},
	.id_table = id_match,
};
module_spi_driver(m74hc595);
MODULE_LICENSE("GPL");
