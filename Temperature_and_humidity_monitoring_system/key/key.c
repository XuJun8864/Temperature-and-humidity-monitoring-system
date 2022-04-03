#include "./key.h"
struct cdev *cdev;
struct class *cls;
struct device *dev;
struct fasync_struct *fapp;
struct device_node *node;
struct timer_list mytimer;
struct timer_list mytimer_2;
unsigned int irqno;
unsigned int irqno_2;
unsigned int irqno_3;
unsigned int irqno_temp;
unsigned int major = 0;
unsigned int minor = 0;;
int gpiono;
int gpiono_2;
int gpiono_3;
int data = 0;
int flag_timer = 0;
int flag_timer_2 = 0;
int flag_timer_count = 0;
/*定时器2的回调函数*/
void irq_timer_function_2(struct timer_list *timer)
{
	flag_timer_count += 1;
	switch(data){
	case 1:
		if(!gpio_get_value(gpiono)){
			/*长按*/
			if(flag_timer_count >20){
				data |= (0x1 << 8);
				//发信号
				kill_fasync(&fapp,SIGIO,POLL_IN);
				flag_timer_count = 0;
				break;
			}
			/*再次启动定时器2*/
			mod_timer(&mytimer_2,jiffies+10);
		}else{
			/*短按*/
			if(flag_timer_count < 10){
				data &= ~(0x1 << 8);
				//发信号
				kill_fasync(&fapp,SIGIO,POLL_IN);
				flag_timer_count = 0;
			}
		}
		break;
	case 2:
		if(!gpio_get_value(gpiono_2)){
			/*长按*/
			if(flag_timer_count >20){
				data |= (0x1 << 8);
				//发信号
				kill_fasync(&fapp,SIGIO,POLL_IN);
				flag_timer_count = 0;
				break;
			}
			/*再次启动定时器2*/
			mod_timer(&mytimer_2,jiffies+10);
		}else{
			/*短按*/
			if(flag_timer_count < 10){
				data &= ~(0x1 << 8);
				//发信号
				kill_fasync(&fapp,SIGIO,POLL_IN);
				flag_timer_count = 0;
			}
		}
		break;
	case 3:
		if(!gpio_get_value(gpiono_3)){
			/*长按*/
			if(flag_timer_count >20){
				data |= (0x1 << 8);
				//发信号
				kill_fasync(&fapp,SIGIO,POLL_IN);
				flag_timer_count = 0;
				break;
			}
			/*再次启动定时器2*/
			mod_timer(&mytimer_2,jiffies+10);
		}else{
			/*短按*/
			if(flag_timer_count < 10){
				data &= ~(0x1 << 8);
				//发信号
				kill_fasync(&fapp,SIGIO,POLL_IN);
				flag_timer_count = 0;
			}
		}
		break;
	default:
		printk("%d\n",__LINE__);
		break;
	}
}

/*定时器1的回调函数：按键消抖 */
void irq_timer_function(struct timer_list *timer)
{
	/*按键1中断*/
	if(irqno_temp == irqno){
		if(!gpio_get_value(gpiono)){
			data = 1;
			if(flag_timer_2){
				/*再次启动定时器2*/
				mod_timer(&mytimer_2,jiffies+10);
			}else{
				flag_timer_2 = 1;
				/*注册启动定时器2*/
				add_timer(&mytimer_2);
			}
		}
	}
	/*按键2中断*/
	else if(irqno_temp == irqno_2){
		if(!gpio_get_value(gpiono_2)){
			data = 2;
			if(flag_timer_2){
				/*再次启动定时器2*/
				mod_timer(&mytimer_2,jiffies+10);
			}else{
				flag_timer_2 = 1;
				/*注册启动定时器2*/
				add_timer(&mytimer_2);
			}
		}
	}
	/*按键3中断*/
	else if(irqno_temp == irqno_3){
		if(!gpio_get_value(gpiono_3)){
			data = 3;
			if(flag_timer_2){
				/*再次启动定时器2*/
				mod_timer(&mytimer_2,jiffies+10);
			}else{
				flag_timer_2 = 1;
				/*注册启动定时器2*/
				add_timer(&mytimer_2);
			}
		}
	}else{
		printk("irqno=%d irqno_2=%d irqno_3=%d irqno_temp=%d\n",irqno,irqno_2,irqno_3,irqno_temp);
		printk("%d: unknow irqno\n",__LINE__);
	}
}
/*中断处理函数*/
irqreturn_t key_irq_handle(int irq, void *dev)
{
	if(irq == irqno)
		irqno_temp = irqno;
	else if(irq == irqno_2)
		irqno_temp = irqno_2;
	else if(irq == irqno_3)
		irqno_temp = irqno_3;
	else
		printk("%d: unknow irqno\n",__LINE__);
	if(flag_timer){
		/*再次启动定时器1*/
		mod_timer(&mytimer,jiffies+1);
	}else{
		flag_timer = 1;
		/*注册启动定时器1*/
		add_timer(&mytimer);
	}
	return IRQ_HANDLED;
}

int mykey_open(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

long mykey_ioctl(struct file *file,unsigned int cmd,unsigned long args)
{
	/*将准备好的数据传回应用层*/
	if(copy_to_user((void *)args,&data,GET_CMD_SIZE(cmd)) != 0){
		printk("%d:%s\n",__LINE__,__func__);
		return -1;
	}
	return 0;
}

int mykey_close(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

int mykey_fasync(int fd, struct file *file, int on)
{
	//初始化异步通知的队列，发信号的时候会用到这个队列
	return fasync_helper(fd, file, on, &fapp);
}

const struct file_operations fops = {
	.open = mykey_open,
	.unlocked_ioctl = mykey_ioctl,
	.fasync = mykey_fasync,
	.release = mykey_close,
};

static int __init key_irq_init(void)
{
	int ret;
	dev_t devno;
	// 分配对象
	cdev = cdev_alloc();
	if(cdev == NULL){
		printk("alloc cdev memory error\n");
		return -ENOMEM;
	}
	// 对象的初始化
	cdev_init(cdev,&fops);
	// 动态申请设备号
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
		return ret;
	}
	dev = device_create(cls,NULL,MKDEV(major,minor),NULL,CNAME);
	if(IS_ERR(dev)){
		printk("device create error\n");
		ret = PTR_ERR(dev);
		return ret;
	}

	//0.初始化定时器
	mytimer.expires = jiffies+1;
	timer_setup(&mytimer,irq_timer_function,0);
	//0.初始化定时器2
	mytimer_2.expires = jiffies+10;
	timer_setup(&mytimer_2,irq_timer_function_2,0);
	//1.获取节点
	node = of_find_compatible_node(NULL,NULL,"HQYJ,key");
	if(node == NULL){
		printk("of find node error\n");
		return -EAGAIN;
	}
	//2.解析软中断号
	irqno = irq_of_parse_and_map(node,0);
	if(irqno == 0){
		printk("irq parse and map error\n");
		return -EAGAIN;
	}
	irqno_2 = irq_of_parse_and_map(node,1);
	if(irqno_2 == 0){
		printk("irq parse and map error\n");
		return -EAGAIN;
	}
	irqno_3 = irq_of_parse_and_map(node,2);
	if(irqno_3 == 0){
		printk("irq parse and map error\n");
		return -EAGAIN;
	}
	/*获取gpio引脚号*/
	gpiono = of_get_named_gpio(node,"key1",0);
	if(gpiono < 0){
		printk("get gpiono error\n");
		return gpiono;
	}
	gpiono_2 = of_get_named_gpio(node,"key2",0);
	if(gpiono_2 < 0){
		printk("get gpiono error\n");
		return gpiono_2;
	}
	gpiono_3 = of_get_named_gpio(node,"key3",0);
	if(gpiono_3 < 0){
		printk("get gpiono error\n");
		return gpiono_3;
	}
	//3.注册中断
	ret = request_irq(irqno,key_irq_handle,IRQF_TRIGGER_FALLING,"key1_interrupts",NULL);
	if(ret){
		printk("%d:request irq error\n",__LINE__);
		return ret;
	}
	ret = request_irq(irqno_2,key_irq_handle,IRQF_TRIGGER_FALLING,"key2_interrupts",NULL);
	if(ret){
		printk("%d:request irq error\n",__LINE__);
		return ret;
	}
	ret = request_irq(irqno_3,key_irq_handle,IRQF_TRIGGER_FALLING,"key3_interrupts",NULL);
	if(ret){
		printk("%d:request irq error\n",__LINE__);
		return ret;
	}

	return 0;
}
static void __exit key_irq_exit(void)
{
	free_irq(irqno,NULL);
	free_irq(irqno_2,NULL);
	free_irq(irqno_3,NULL);
	del_timer(&mytimer);
	del_timer(&mytimer_2);
	device_destroy(cls,MKDEV(major,minor));
	class_destroy(cls);
	cdev_del(cdev);
	unregister_chrdev_region(MKDEV(major,minor),COUNT);
	kfree(cdev);
	printk("%s:%d\n",__func__,__LINE__);
}
module_init(key_irq_init);
module_exit(key_irq_exit);
MODULE_LICENSE("GPL");
