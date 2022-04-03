#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include "./si7006.h"

unsigned int major;
struct class *cls;
struct device *dev;
/*总线驱动和设备驱动匹配成功之后创建的对象*/
struct i2c_client *si_client;

int i2c_read_tmp_hum(unsigned char reg)
{
	/*温度 reg=0xe3  湿度reg=0xe5*/
	unsigned short data;
	int ret;
	char rbuf[] = {reg};
	/*封装消息*/
	struct i2c_msg msgs[] = {               
		[0] = {
			.addr = si_client->addr, 	// __u16 addr;	
			.flags = 0, 				// __u16 flags;
			.len = 1,      				// __u16 len;
			.buf = rbuf, 				// __u8 *buf;
		},
		[1] = {
			.addr = si_client->addr,
			.flags = 1,
			.len = 2,
			.buf = (__u8 *)&data,
		},
	};
	/*发送消息*/
	ret = i2c_transfer(si_client->adapter,msgs,ARRAY_SIZE(msgs));
	if(ret != ARRAY_SIZE(msgs)){
		printk("%d i2c_transfer error\n",__LINE__);
		printk("ret = %d\n",ret);
		return -EAGAIN;
	}
	/*数据高低位转化*/
	data = data >> 8 | data << 8;
	return data;
}

int si7006_open(struct inode *inode, struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}

long si7006_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
	int ret,data = 0;
	switch(cmd){
	case GET_TEMP_DATA:
		data = i2c_read_tmp_hum(TEMP_ADDR);
		if(data < 0){
			printk("%d i2c_read_tmp_hum error\n",__LINE__);
			return data;
		}
		break;
	case GET_HUM_DATA:
		data = i2c_read_tmp_hum(HUM_ADDR);
		if(data < 0){
			printk("%d i2c_read_tmp_hum error\n",__LINE__);
			return data;
		}
		break;
	}
	/*拷贝数据到用户空间*/
	ret = copy_to_user((void *)args,&data,sizeof(int));
	if(ret != 0){
		printk("%d copy_to_user error",__LINE__);
		return -EINVAL;
	}
	return 0;
}

int si7006_close(struct inode *inode, struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}
/*操作方法结构体*/
const struct file_operations fops = {
	.open = si7006_open,
	.unlocked_ioctl = si7006_ioctl,
	.release = si7006_close,
};

int si7006_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	si_client = client;
	printk("%s:%d\n",__func__,__LINE__);

	/*注册字符设备驱动*/
	major = register_chrdev(0,I2C_NAME,&fops);
	if(major < 0){
		printk("%d register_chrdev error\n",__LINE__);
		ret = -ENOSYS;
		goto ERR1;
	}
	printk("major = %d\n",major);
	/*提交目录*/
	cls = class_create(THIS_MODULE,I2C_NAME);
	if(IS_ERR(cls)){
		printk("%d class_create error\n",__LINE__);
		ret = PTR_ERR(cls);
		goto ERR2;
	}
	/*提交设备节点信息*/
	dev = device_create(cls,NULL,MKDEV(major,0),NULL,"si7006_0");
	if(IS_ERR(dev)){
		printk("%d device_create error\n",__LINE__);
		ret = PTR_ERR(dev);
		goto ERR3;
	}
	return 0;

ERR3:
	device_destroy(cls,MKDEV(major,0));
ERR2:
	class_destroy(cls);
ERR1:
	unregister_chrdev(major,I2C_NAME);
	return ret;
}

int si7006_remove(struct i2c_client *client)
{
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major,I2C_NAME);
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}

const struct of_device_id oftable[] = {
	{.compatible = "HQYJ,si7006",},
	{},
};
struct i2c_driver si7006 = {
	.probe = si7006_probe,
	.remove = si7006_remove,
	.driver = {
		.name = "www.hqyj",				//不用参与匹配，必须填充
		.of_match_table = oftable,		//设备树匹配	
	},
};

module_i2c_driver(si7006);
MODULE_LICENSE("GPL");

