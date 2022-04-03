#ifndef __SI7006_H__
#define __SI7006_H__

#define GET_TEMP_DATA _IOR('s',0,int) /*0读温度*/
#define GET_HUM_DATA _IOR('s',1,int)  /*1读湿度*/	


#define TEMP_ADDR 0xe3
#define HUM_ADDR 0xe5

#define I2C_NAME "si7006"

#endif

