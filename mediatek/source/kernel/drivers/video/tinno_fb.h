#ifndef __TINNO_FB_H
#define __TINNO_FB_H







#ifdef __KERNEL__

#define TINNO_FB_DRIVER "tinno_fb"

struct tinno_fb_device {
    int             state;
    void           *fb_va_base;             /* MPU virtual address */
    dma_addr_t      fb_pa_base;             /* Bus physical address */
    unsigned long   fb_size_in_byte;


    u32             pseudo_palette[17];

    struct fb_info *fb_info;                /* Linux fbdev framework data */
    struct device  *dev;
};

#if 1
#define TFB_DBG(fmt, arg...) \
	printk("[TFBI] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)
#else
#define TFB_DBG(fmt, arg...) do {} while (0)
#endif

#define TFB_ERR(fmt, arg...) \
	printk("[TFBE] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)

#endif//__KERNEL__

#endif//__TINNO_FB_H
