#include "./buzzer.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/device.h>

int major = 0;
gpio_t *virt_gpiob_addr;
unsigned int *virt_rcc_ahb4_addr;
struct class *cls;
struct device *dev;

int buzzer_open(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
long buzzer_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
	switch(cmd){
	case BUZZER_ON:
		SET_BUZZER_ON;
		break;
	case BUZZER_OFF:
		SET_BUZZER_OFF;
		break;
	}
	return 0;
}
int buzzer_close(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

struct file_operations fops = {
	.open = buzzer_open,
	.unlocked_ioctl = buzzer_ioctl,
	.release = buzzer_close,
};

static int __init buzzer_init(void)
{
	printk("%s:%d\n",__func__,__LINE__);
	/*1.注册字符设备驱动*/
	major = register_chrdev(0,CNAME,&fops);
	if(major < 0){
		printk("register_chrdev error\n");
		return major;
	}
	printk("major = %d\n",major);

	/*2.地址映射*/
	virt_rcc_ahb4_addr = ioremap(RCC_AHB4,4);
	if(virt_rcc_ahb4_addr == NULL){
		printk("__%d__ ioremap error",__LINE__);
		return -ENOMEM;
	}
	virt_gpiob_addr = ioremap(GPIOB_BASE_ADDR,sizeof(gpio_t));
	if(virt_gpiob_addr == NULL){
		printk("__%d__ ioremap error\n",__LINE__);
		return -ENOMEM;
	}
	// 使能
	*virt_rcc_ahb4_addr |= (0x1 << 1);
	// 输出模式
	virt_gpiob_addr->MODER &= ~(0x3 << 12);
	virt_gpiob_addr->MODER |= (0x1 << 12);
	virt_gpiob_addr->ODR &= ~(0x1 << 6);

	/*向上层提交目录名*/
	cls = class_create(THIS_MODULE,CNAME);
	if(IS_ERR(cls)){
		printk("%d: class_create error\n",__LINE__);
		return PTR_ERR(cls);
	}
	/*向上层提交创建设备节点的信息*/
	dev = device_create(cls,NULL,MKDEV(major,0),NULL,CNAME);
	if(IS_ERR(dev)){
		printk("%d: device_create error\n",__LINE__);
		return PTR_ERR(dev);
	}

	return 0;
}

static void __exit buzzer_exit(void)
{
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);
	/*取消映射*/
	iounmap(virt_gpiob_addr);
	iounmap(virt_rcc_ahb4_addr);
	/*注销字符设备驱动*/
	unregister_chrdev(major,CNAME);
	printk("%s:%d\n",__func__,__LINE__);
}


module_init(buzzer_init);
module_exit(buzzer_exit);
MODULE_LICENSE("GPL");
