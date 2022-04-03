#include "./leds.h"

struct cdev *cdev;
unsigned int major = 0;
unsigned int minor = 0;
struct class *cls;
struct device *dev;
struct device_node * node;
char *name[3] = {"led1","led2","led3"};
int gpiono[3] = {0};
int led_open(struct inode *inode, struct file *file)
{
	printk("%s %d\n",__func__,__LINE__);
	return 0;
}
int led_close(struct inode *inode, struct file *file)
{
	printk("%s %d\n",__func__,__LINE__);
	return 0;
}
long led_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
	int i;
	switch(GET_CMD_NR(cmd)){
	case 1:
		for(i=0; i<3; i++){
			//4.设置gpio方向为输出，并且输出高电平
			gpio_direction_output(gpiono[i],1);
		}
		break;
	case 0:
		for(i=0; i<3; i++){
			//4.设置gpio方向为输出，并且输出低电平
			gpio_direction_output(gpiono[i],0);
		}
		break;
	default:
		printk("%d:未知错误\n",__LINE__);
	}
	return 0;
}

const struct file_operations fops = {
	.open = led_open,
	.unlocked_ioctl = led_ioctl,
	.release = led_close,
};
static int __init myleds_init(void)
{
	int i,ret;
	dev_t devno;
	printk("%s %d\n",__func__,__LINE__);
	/*分配对象*/
	cdev = cdev_alloc();
	if(cdev == NULL){
		printk("alloc cdev memory error\n");
		ret = -ENOMEM;
		return ret;
	}
	//对象的初始化
	cdev_init(cdev,&fops);
	//动态申请设备号
	ret = alloc_chrdev_region(&devno,minor,COUNT,CNAME);
	if(ret){
		printk("dynamic alloc device number error\n");
		return ret;
	}
	major = MAJOR(devno);
	minor = MINOR(devno);
	printk("major = %d,minor = %d\n",major,minor);
	//4.注册
	ret = cdev_add(cdev,MKDEV(major,minor),COUNT);
	if(ret){
		printk("register char device driver error\n");
		return ret;
	}
	//5.自动创建设备节点
	cls = class_create(THIS_MODULE,CNAME);
	if(IS_ERR(cls)){
		printk("class create error\n");
		ret = PTR_ERR(cls);
		printk("ret = %d\n",ret);
		return ret;
	}
	dev = device_create(cls,NULL,MKDEV(major,minor),NULL,CNAME);
	if(IS_ERR(dev)){
		printk("device create error\n");
		ret = PTR_ERR(dev);
		return ret;
	}

	//1.获取设备树节点
	node = of_find_node_by_path("/leds");
	if(node == NULL){
		printk("find node error\n");
		return -EAGAIN;
	}
	for(i=0;i<ARRAY_SIZE(gpiono);i++){
		//2.获取gpio编号
		gpiono[i] = of_get_named_gpio(node,name[i],0);
		if(gpiono[i] < 0){
			printk("get gpio no error\n");
			return -ENODEV;
		}
		gpio_free(gpiono[i]);

		//3.申请要使用的gpio
		ret = gpio_request(gpiono[i],NULL);
		if(ret){
			printk("gpio request error\n");
			return ret;
		}
	}

	return 0;
}
static void __exit myleds_exit(void)
{
	int i;
	for(i=0;i<ARRAY_SIZE(gpiono);i++){
		gpio_free(gpiono[i]);
	}
	device_destroy(cls,MKDEV(major,minor));
	class_destroy(cls);

	cdev_del(cdev);

	unregister_chrdev_region(MKDEV(major,minor),COUNT);

	kfree(cdev);
	printk("%s %d\n",__func__,__LINE__);

}

module_init(myleds_init);
module_exit(myleds_exit);
MODULE_LICENSE("GPL");
