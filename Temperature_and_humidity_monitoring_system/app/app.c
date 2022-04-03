#include "./app.h"
/*条件变量*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
//互斥锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int fd,fd_m74hc595,fd_buzzer,fd_key,fd_temp_txt,fd_leds;
float rtemp,rhum;
float threshold = 21; /*温度阈值*/
char str[8] = {0};
char str_2[8] = {0};
char buf[2] = {0};
char mycmd;
int flag_tmp_hum = 0;		/*温湿度状态*/
int flag_warning = 1;

void signal_handle(int signo)
{
	int data;
	switch(signo){
	case SIGIO:
		ioctl(fd_key,GET_KEY_VALUE,&data);
		switch(data & ~(0xffffff<<8)){
		case KEY1:
			switch(GET_9){
			case SHORT_PRESS: 
				/*增加阈值*/
				threshold += data;
				printf("阈值 = %.2f\n",threshold);
				lseek(fd_temp_txt,0,SEEK_SET);
				write(fd_temp_txt,&threshold,sizeof(threshold));
				break;
			case LONG_PRESS:
				break;
			default:
				break;
			}
			break;
		case KEY2:
			switch(GET_9){
			case SHORT_PRESS: 
				/*降低阈值*/
				threshold -= data;
				printf("阈值 = %.2f\n",threshold);
				lseek(fd_temp_txt,0,SEEK_SET);
				write(fd_temp_txt,&threshold,sizeof(threshold));
				break;
			case LONG_PRESS:
				break;
			default:
				break;
			}
			break;
		case KEY3:
			switch(GET_9){
			case SHORT_PRESS: 
				flag_warning = 0;;
				ioctl(fd_buzzer,BUZZER_OFF,&data);
				printf("清除警报\n");
				break;
			case LONG_PRESS:
				/*切换温湿度显示*/
				flag_tmp_hum ^= 0x1;
				break;
			default:
				printf("%d:未知错误\n",__LINE__);
				break;
			}
			break;
		default:
			printf("%d:未知错误\n",__LINE__);
			break;
		}
		break;
	case SIGINT:
		/*关闭蜂鸣器和灯*/
		ioctl(fd_buzzer,BUZZER_OFF,&data);
		ioctl(fd_leds,LED_OFF,NULL);
		exit(0);
		break;
	default:
		printf("%d:未知错误\n",__LINE__);
		break;
	}
}
/*显示温度或湿度*/
void *call_back(void *arg)
{
	pthread_detach(pthread_self());
	int rbuf[4] = {0}; /*四个对应数码管四个数的下标*/
	int i,j,t;
	while(1){
		switch(flag_tmp_hum){
		case 0:		/*显示温度*/
			t = 0;
			/*比较找到对应的地址下标*/
			for(i=0;i<sizeof(str);i++){
				if(str[i] == '.') continue;
				for(j=0;j<sizeof(data);j++){
					if(str[i] == data[j] && str[i+1] == '.'){
						j = j + 18;
						rbuf[t++] = j;
						break;
					}else if(str[i] == data[j]){
						rbuf[t++] = j;
						break;
					}
				}
			}
			/*发送四位数*/
			for(i=0;i<4;i++){
				buf[0] = which[i];
				buf[1] = math[rbuf[i]];
				ioctl(fd_m74hc595,SPI_DATA_2,buf);
			}
			break;
		case 1:		/*显示湿度*/
			t = 0;
			/*比较找到对应的地址下标*/
			for(i=0;i<sizeof(str_2);i++){
				if(str_2[i] == '.') continue;
				for(j=0;j<sizeof(data);j++){
					if(str_2[i] == data[j] && str_2[i+1] == '.'){
						j = j + 18;
						rbuf[t++] = j;
						break;
					}else if(str_2[i] == data[j]){
						rbuf[t++] = j;
						break;
					}
				}
			}
			/*发送四位数*/
			for(i=0;i<4;i++){
				buf[0] = which[i];
				buf[1] = math[rbuf[i]];
				ioctl(fd_m74hc595,SPI_DATA_2,buf);
			}
			break;
		default:
			break;
		}
	}

	return 0;
}
/*报警*/
void *call_back_2(void *arg)
{
	pthread_detach(pthread_self());
	int data;
	while(1){
		if(rtemp >= threshold){
			if(flag_warning == 0){
				sleep(10);
				flag_warning = 1;
			}
			ioctl(fd_buzzer,BUZZER_ON,&data);
			ioctl(fd_leds,LED_ON,NULL);
			sleep(1);
			ioctl(fd_buzzer,BUZZER_OFF,&data);
			ioctl(fd_leds,LED_OFF,NULL);
			sleep(1);
		}else{
			ioctl(fd_buzzer,BUZZER_OFF,&data);
			ioctl(fd_leds,LED_OFF,NULL);
			pthread_cond_wait(&cond, &mutex);
		}
	}
}
/*功能选择*/
void *call_back_3(void *arg)
{
	pthread_detach(pthread_self());
	read(fd_temp_txt,&threshold,sizeof(threshold));
	while(1){
		mycmd = fgetc(stdin);
		while(getchar() != '\n');
		switch(mycmd){
		case '1':
			flag_tmp_hum ^= (0x1);
			break;
		case '2':
			printf("目前报警阈值 = %.2f\n",threshold);
			printf("请输入修改后的温度阈值\n");
			scanf("%f",&threshold);
			printf("修改完成阈值 = %.2f\n",threshold);
			lseek(fd_temp_txt,0,SEEK_SET);
			write(fd_temp_txt,&threshold,sizeof(threshold));
			break;
		}
	}
}
int main(int argc, const char *argv[])
{
	int i,j;
	int temp,hum,ret;
	char temp_t = 'C';
	char hum_t = 'H';
	pthread_t tid;
	pthread_t tid_2;
	pthread_t tid_3;
	fd_temp_txt = open("./temp.txt",O_RDWR | O_CREAT);
	if(fd_temp_txt < 0){
		ERR("open");
		return -1;
	}

	fd = open("/dev/si7006_0",O_RDWR);
	if(fd < 0){
		ERR("open");
		return -1;
	}
	fd_m74hc595 = open("/dev/m74hc595",O_RDWR);
	if(fd_m74hc595 < 0){
		ERR("open");
		return -1;
	}

	fd_buzzer = open("/dev/buzzer",O_RDWR);
	if(fd_buzzer < 0){
		ERR("open");
		return -1;
	}
	fd_key = open("/dev/mykey",O_RDWR);
	if(fd_key < 0){
		ERR("open");
		return -1;
	}
	fd_leds = open("/dev/myleds",O_RDWR);
	if(fd_leds < 0){
		ERR("open");
		return -1;
	}

	//1.注册信号处理函数
	signal(SIGIO,signal_handle);
	signal(SIGINT,signal_handle);
	//2.调用fasync初始化异步通知队列
	fcntl(fd_key,F_SETFL,fcntl(fd_key,F_GETFL)|FASYNC);
	//告诉内核接收信号的进程
	fcntl(fd_key,F_SETOWN,getpid());

	if(pthread_create(&tid,NULL,call_back,NULL) != 0){
		ERR("pthread_create");
		return -1;
	}
	if(pthread_create(&tid_2,NULL,call_back_2,NULL) != 0){
		ERR("pthread_create");
		return -1;
	}
	if(pthread_create(&tid_3,NULL,call_back_3,NULL) != 0){
		ERR("pthread_create");
		return -1;
	}

	while(1){
		ioctl(fd,GET_TEMP_DATA,&temp);
		ioctl(fd,GET_HUM_DATA,&hum);
		rtemp = 175.72*temp/65536 - 46.85;
		rhum = 125*hum/65536.0 - 6;
		bzero(str,sizeof(str));
		bzero(str_2,sizeof(str_2));
		sprintf(str,"%.1f%c",rtemp,temp_t);
		sprintf(str_2,"%.1f%c",rhum,hum_t);
		/*唤醒警报*/
		if(rtemp > threshold){
			pthread_cond_signal(&cond);
		}
		if(flag_tmp_hum){
			printf("hum = %.2f%%\n",rhum);
		}else{
			printf("temp = %.2f\n",rtemp);
		}
		if(mycmd != '2'){
			printf("请输入要选择的功能\n");
			printf("--------1.切换温湿度显示--------\n");
			printf("--------2.修改报警温度阈值--------\n");
		}else{
			printf("目前报警阈值 = %.2f\n",threshold);
			printf("请输入修改后的温度阈值\n");	
		}
		sleep(1);
	}
	close(fd);
	close(fd_m74hc595);
	close(fd_buzzer);
	close(fd_key);
	close(fd_temp_txt);
	close(fd_leds);
	//销毁互斥锁
	pthread_mutex_destroy(&mutex);
	//销毁条件变量
	pthread_cond_destroy(&cond);
	return 0;
}
