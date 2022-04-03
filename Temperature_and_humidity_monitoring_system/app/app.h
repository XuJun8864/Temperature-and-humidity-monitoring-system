#ifndef __APP_H__
#define __APP_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "../si7006/si7006.h"
#include "../m74hc595/m74hc595.h"
#include "../buzzer/buzzer.h"
#define GET_KEY_VALUE _IOR('k',1,int)
#define KEY1 1
#define KEY2 2
#define KEY3 3
#define LONG_PRESS 1
#define SHORT_PRESS 0
#define GET_9 (data>>8 & 1)
#define LED_ON _IO('l',1)
#define LED_OFF _IO('l',0)
#define ERR(msg) do{\
	fprintf(stderr, "__%d__ ", __LINE__);\
	perror(msg);\
}while(0)

#endif
