#ifndef __LEDS_H__
#define __LEDS_H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>

#define COUNT 1
#define CNAME "myleds"
#define GET_CMD_SIZE(cmd)  ((cmd>>16)&0x3fff)
#define GET_CMD_NR(cmd)  (cmd & ~(0xffffff<<8))
#endif
