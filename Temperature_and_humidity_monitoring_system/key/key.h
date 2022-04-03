#ifndef __KEY_H__
#define __KEY_H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/poll.h>
#define CNAME "mykey"
#define COUNT 1
#define GET_CMD_SIZE(cmd) ((cmd>>16)&0x3fff)
#endif
