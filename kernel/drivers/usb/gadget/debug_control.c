/*
 * debug_control.c -- for Audio, USB, mmc debug
 *
 * Copyright (C) 2010 Liu Yang
 * All rights reserved.
 *
 */
#include <linux/types.h>
#include <linux/ioctl.h>
#include <asm/atomic.h>
#include <asm/sizes.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/debug_control.h>
#include <linux/workqueue.h>

#define DRIVER_NAME "debug_control"

uint16_t  audio_debug_mask = 0;
uint16_t  audio_info_mask = 0;
uint16_t  usb_debug_mask = 0;
uint16_t  usb_info_mask = 0;
uint16_t  mmc_debug_mask = 0;
uint16_t  mmc_info_mask = 0;
uint16_t  gps_debug_mask = 0;
uint16_t  gps_info_mask = 0;
uint16_t  touch_debug_mask = 0;
uint16_t  touch_info_mask = 0;

static int set_audio_debug(const char *val, struct kernel_param *kp);
static int set_audio_info(const char *val, struct kernel_param *kp);
static int set_usb_debug(const char *val, struct kernel_param *kp);
static int set_usb_info(const char *val, struct kernel_param *kp);
static int set_mmc_debug(const char *val, struct kernel_param *kp);
static int set_mmc_info(const char *val, struct kernel_param *kp);
static int set_gps_debug(const char *val, struct kernel_param *kp);
static int set_gps_info(const char *val, struct kernel_param *kp);
static int set_touch_debug(const char *val, struct kernel_param *kp);
static int set_touch_info(const char *val, struct kernel_param *kp);

module_param_call(audio_debug, set_audio_debug, NULL, NULL, S_IWUSR);
MODULE_PARM_DESC(audio_debug, "0 - Audio Debug Disable, 1 - Audio Debug Enable");
module_param_call(audio_info, set_audio_info, NULL, NULL, S_IWUSR);
MODULE_PARM_DESC(audio_info, "0 - Audio Info Disable, 1 - Audio Info Enable");

module_param_call(usb_debug, set_usb_debug, NULL, NULL, S_IWUSR);
MODULE_PARM_DESC(usb_debug, "0 - USB Debug Disable, 1 - USB Debug Enable");
module_param_call(usb_info, set_usb_info, NULL, NULL, S_IWUSR);
MODULE_PARM_DESC(usb_info, "0 - USB Info Disable, 1 - USB Info Enable");

module_param_call(mmc_debug, set_mmc_debug, NULL, NULL, S_IWUSR);
MODULE_PARM_DESC(mmc_debug, "0 - MMC Debug Disable, 1 - MMC Debug Enable");
module_param_call(mmc_info, set_mmc_info, NULL, NULL, S_IWUSR);
MODULE_PARM_DESC(mmc_info, "0 - MMC Info Disable, 1 - MMC Info Enable");

module_param_call(gps_debug, set_gps_debug, NULL, NULL, S_IWUSR);
MODULE_PARM_DESC(gps_debug, "0 - GPS Debug Disable, 1 - GPS Debug Enable");
module_param_call(gps_info, set_gps_info, NULL, NULL, S_IWUSR);
MODULE_PARM_DESC(gps_info, "0 - GPS Info Disable, 1 - GPS Info Enable");

module_param_call(touch_debug, set_touch_debug, NULL, NULL, S_IWUSR);
MODULE_PARM_DESC(touch_debug, "0 - TOUCH Debug Disable, 1 - TOUCH Debug Enable");
module_param_call(touch_info, set_touch_info, NULL, NULL, S_IWUSR);
MODULE_PARM_DESC(touch_info, "0 - TOUCH Info Disable, 1 - TOUCH Info Enable");

static int set_audio_debug(const char *val, struct kernel_param *kp)
{
	int ret = 0;
	unsigned long tmp = 0;
	ret = strict_strtoul(val, 10, &tmp);
	printk("%s: audio_debug_mask=%d, tmp = %d, ret=%d\n", 
		__func__, (int)audio_debug_mask, (int)tmp, ret);
	if (ret)
		goto out;
	audio_debug_mask = tmp;
	
out:
	return ret;
}
void mt6575_audio_debug(const char *fmt, ...)
{
	va_list args;
	int r;

	if (audio_debug_mask) {
        	va_start(args, fmt);
        	r = vprintk(fmt, args);
        	va_end(args);
	}
}
EXPORT_SYMBOL(mt6575_audio_debug);

static int set_audio_info(const char *val, struct kernel_param *kp)
{
	int ret = 0;
	unsigned long tmp = 0;
	ret = strict_strtoul(val, 10, &tmp);
	printk("%s: audio_info_mask=%d, tmp = %d, ret=%d\n", 
		__func__, (int)audio_info_mask, (int)tmp, ret);
	if (ret)
		goto out;
	audio_info_mask = tmp;
	
out:
	return ret;
}
void mt6575_audio_info(const char *fmt, ...)
{
	va_list args;
	int r;
	if (audio_info_mask) 
	{
        	va_start(args, fmt);
        	r = vprintk(fmt, args);
        	va_end(args);
	}
}
EXPORT_SYMBOL(mt6575_audio_info);

//extern int lcm_power_test(int on);
static int set_usb_debug(const char *val, struct kernel_param *kp)
{
	int ret = 0;
	unsigned long tmp = 0;
	ret = strict_strtoul(val, 10, &tmp);
	printk("%s: usb_debug_mask=%d, tmp = %d, ret=%d\n", 
		__func__, (int)usb_debug_mask, (int)tmp, ret);
	//lcm_power_test((int)tmp);
	if (ret)
		goto out;
	usb_debug_mask = tmp;
	
out:
	return ret;
}
void mt6575_usb_debug(const char *fmt, ...)
{
	va_list args;
	int r;

	if (usb_debug_mask) {
        	va_start(args, fmt);
        	r = vprintk(fmt, args);
        	va_end(args);
	}
}
EXPORT_SYMBOL(mt6575_usb_debug);

static int set_usb_info(const char *val, struct kernel_param *kp)
{
	int ret = 0;
	unsigned long tmp = 0;
	ret = strict_strtoul(val, 10, &tmp);
	printk("%s: usb_info_mask=%d, tmp = %d, ret=%d\n",
		__func__, (int)usb_info_mask, (int)tmp, ret);
	if (ret)
		goto out;
	usb_info_mask = tmp;
	
out:
	return ret;
}
void mt6575_usb_info(const char *fmt, ...)
{
	va_list args;
	int r;
	if (usb_info_mask) 
	{
        	va_start(args, fmt);
        	r = vprintk(fmt, args);
        	va_end(args);
	}
}
EXPORT_SYMBOL(mt6575_usb_info);

extern void force_rescan_mmc(int slot);
static int set_mmc_debug(const char *val, struct kernel_param *kp)
{
	int ret = 0;
	unsigned long tmp = 0;
	ret = strict_strtoul(val, 10, &tmp);
	printk("%s: mmc_debug_mask=%d, tmp = %d, ret=%d\n", 
		__func__, (int)mmc_debug_mask, (int)tmp, ret);
	force_rescan_mmc(tmp);
	if (ret)
		goto out;
	mmc_debug_mask = tmp;
	
out:
	return ret;
}

void mt6575_mmc_debug(const char *fmt, ...)
{
	va_list args;
	int r;

	if (mmc_debug_mask) {
        	va_start(args, fmt);
        	r = vprintk(fmt, args);
        	va_end(args);
	}
}
EXPORT_SYMBOL(mt6575_mmc_debug);

static int set_mmc_info(const char *val, struct kernel_param *kp)
{
	int ret = 0;
	unsigned long tmp = 0;
	ret = strict_strtoul(val, 10, &tmp);
	printk("%s: mmc_info_mask=%d, tmp = %d, ret=%d\n", 
		__func__, (int)mmc_info_mask, (int)tmp, ret);
	if (ret)
		goto out;
	mmc_info_mask = tmp;
	
out:
	return ret;
}
void mt6575_mmc_info(const char *fmt, ...)
{
	va_list args;
	int r;
	if (mmc_info_mask) 
	{
        	va_start(args, fmt);
        	r = vprintk(fmt, args);
        	va_end(args);
	}
}
EXPORT_SYMBOL(mt6575_mmc_info);

static int set_gps_debug(const char *val, struct kernel_param *kp)
{
	int ret = 0;
	unsigned long tmp = 0;
	ret = strict_strtoul(val, 10, &tmp);
	printk("%s: gps_debug_mask=%d, tmp = %d, ret=%d\n", 
		__func__, (int)gps_debug_mask, (int)tmp, ret);
	if (ret)
		goto out;
	gps_debug_mask = tmp;
	
out:
	return ret;
}
void mt6575_gps_debug(const char *fmt, ...)
{
	va_list args;
	int r;

	if (gps_debug_mask) {
        	va_start(args, fmt);
        	r = vprintk(fmt, args);
        	va_end(args);
	}
}
EXPORT_SYMBOL(mt6575_gps_debug);

static int set_gps_info(const char *val, struct kernel_param *kp)
{
	int ret = 0;
	unsigned long tmp = 0;
	ret = strict_strtoul(val, 10, &tmp);
	printk("%s: gps_info_mask=%d, tmp = %d, ret=%d\n", 
		__func__, (int)gps_info_mask, (int)tmp, ret);
	if (ret)
		goto out;
	gps_info_mask = tmp;
	
out:
	return ret;
}
void mt6575_gps_info(const char *fmt, ...)
{
	va_list args;
	int r;
	if (gps_info_mask) 
	{
        	va_start(args, fmt);
        	r = vprintk(fmt, args);
        	va_end(args);
	}
}
EXPORT_SYMBOL(mt6575_gps_info);

static int set_touch_debug(const char *val, struct kernel_param *kp)
{
	int ret = 0;
	unsigned long tmp = 0;
	ret = strict_strtoul(val, 10, &tmp);
	printk("%s: touch_debug_mask=%d, tmp = %d, ret=%d\n", 
		__func__, (int)touch_debug_mask, (int)tmp, ret);
	if (ret)
		goto out;
	touch_debug_mask = tmp;
	
out:
	return ret;
}
void mt6575_touch_debug(const char *fmt, ...)
{
	va_list args;
	int r;

	if (touch_debug_mask) {
        	va_start(args, fmt);
        	r = vprintk(fmt, args);
        	va_end(args);
	}
}
EXPORT_SYMBOL(mt6575_touch_debug);

static int set_touch_info(const char *val, struct kernel_param *kp)
{
	int ret = 0;
	unsigned long tmp = 0;
	ret = strict_strtoul(val, 10, &tmp);
	printk("%s: touch_info_mask=%d, tmp = %d, ret=%d\n", 
		__func__, (int)touch_info_mask, (int)tmp, ret);
	if (ret)
		goto out;
	touch_info_mask = tmp;
	
out:
	return ret;
}
void mt6575_touch_info(const char *fmt, ...)
{
	va_list args;
	int r;
	if (touch_info_mask) 
	{
        	va_start(args, fmt);
        	r = vprintk(fmt, args);
        	va_end(args);
	}
}
EXPORT_SYMBOL(mt6575_touch_info);

static int debug_control_probe(struct platform_device *pdev)
{
	return 0;
}

static int debug_control_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_device debug_control_device = {
	.name	= DRIVER_NAME,
	.id		= 0,
};

static struct platform_driver debug_control_driver = {
	.driver = {
		.name = DRIVER_NAME,
	},
	.probe 	= debug_control_probe,
	.remove 	= debug_control_remove,
};

static int __init debug_control_init(void)
{
	int ret;
	printk("%s--Liu\n",__func__);
	ret = platform_driver_register(&debug_control_driver);
	if (ret) {
		printk(KERN_ERR "%s failed to register debug_control_driver\n", __func__);
		return ret;
	}
	ret = platform_device_register(&debug_control_device);
	if (ret) {
		printk(KERN_ERR "%s failed to register debug_control_device\n", __func__);
		return ret;
	}

	return ret;
}

static void __exit debug_control_cleanup(void)
{
	printk("%s--Liu\n",__func__);
	platform_device_unregister(&debug_control_device);
	platform_driver_unregister(&debug_control_driver);
}

module_init(debug_control_init);
module_exit(debug_control_cleanup);

MODULE_AUTHOR("Liu Yang");
MODULE_DESCRIPTION("mt6575 Audio USB  MMC Debug Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
