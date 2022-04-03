#ifndef __BUZZER_H__
#define __BUZZER_H__

typedef struct {
	volatile unsigned int MODER;   // 0x00
	volatile unsigned int OTYPER;  // 0x04
	volatile unsigned int OSPEEDR; // 0x08
	volatile unsigned int PUPDR;   // 0x0C
	volatile unsigned int IDR;     // 0x10
	volatile unsigned int ODR;     // 0x14
	volatile unsigned int BSRR;    // 0x18
	volatile unsigned int LCKR;    // 0x1C
	volatile unsigned int AFRL;    // 0x20
	volatile unsigned int AFRH;    // 0x24
	volatile unsigned int BRR;     // 0x28
	volatile unsigned int res;
	volatile unsigned int SECCFGR; // 0x30

}gpio_t;


#define RCC_AHB4   (0x50000A28)
#define GPIOB_BASE_ADDR (0x50003000)
#define SET_BUZZER_ON (virt_gpiob_addr->ODR |= (0x1 << 6))
#define SET_BUZZER_OFF (virt_gpiob_addr->ODR &= ~(0x1 << 6))
#define BUZZER_ON _IOW('b',1,int)
#define BUZZER_OFF _IOW('b',0,int)
#define GET_CMD_SIZE(cmd) ((cmd>>16) & 0x3fff)
#define CNAME "buzzer"

#endif
