#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "./test.h"

int main(int argc, const char *argv[])
{
	int which=0;
	int data=0;
	int fd;
	fd = open("/dev/m74hc595",O_RDWR);
	if(fd < 0){
		perror("open error");
		return -1;
	}

	while(1){
		ioctl(fd,SEG_WHICH,which++);
		ioctl(fd,SEG_DAT,data++);
		if(which >= 4)which=0;
		if(data >= 16)data = 0;
		sleep(1);
	}

	close(fd);
	return 0;
}

