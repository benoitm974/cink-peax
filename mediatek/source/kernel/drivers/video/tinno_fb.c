#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/vmalloc.h>
#include <linux/disp_assert_layer.h>
#include <linux/semaphore.h>
#include <linux/xlog.h>

#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/mach-types.h>
#include <asm/cacheflush.h>
#include <asm/io.h>

#include <mach/dma.h>
#include <mach/irqs.h>

#include "debug.h"

#include "tinno_fb.h"

#ifndef ASSERT
    #define ASSERT(expr)        BUG_ON(!(expr))
#endif

static u32 TINNO_FB_XRES  = 320;
static u32 TINNO_FB_YRES  = 480;
static u32 TINNO_FB_BPP   = 32;
static u32 TINNO_FB_PAGES = 2;
static u32 fb_xres_update = 320;
static u32 fb_yres_update = 480;
#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))

#define TINNO_FB_XRESV (ALIGN_TO(TINNO_FB_XRES, 32))
#define TINNO_FB_YRESV (ALIGN_TO(TINNO_FB_YRES, 32) * TINNO_FB_PAGES) /* For page flipping */
#define TINNO_FB_BYPP  ((TINNO_FB_BPP + 7) >> 3)
#define TINNO_FB_LINE  (ALIGN_TO(TINNO_FB_XRES, 32) * TINNO_FB_BYPP)
#define TINNO_FB_SIZE  (TINNO_FB_LINE * ALIGN_TO(TINNO_FB_YRES, 32))

#define TINNO_FB_SIZEV (TINNO_FB_LINE * ALIGN_TO(TINNO_FB_YRES, 32) * TINNO_FB_PAGES)

#define CHECK_RET(expr)    \
    do {                   \
        int ret = (expr);  \
        ASSERT(0 == ret);  \
    } while (0)

struct fb_info *tinno_fb_fbi = NULL;

static unsigned long tinno_GetFBRamSize(void)
{
	return ALIGN_TO(TINNO_FB_XRES, 32) * 
	       ALIGN_TO(TINNO_FB_YRES, 32) * 
	       ((TINNO_FB_BPP + 7) >> 3) * 
	       TINNO_FB_PAGES;
}

/* Set fb_info.fix fields and also updates fbdev.
 * When calling this fb_info.var must be set up already.
 */
static void tinno_set_fb_fix(struct tinno_fb_device *fbdev)
{
	struct fb_info           *fbi   = fbdev->fb_info;
	struct fb_fix_screeninfo *fix   = &fbi->fix;
	struct fb_var_screeninfo *var   = &fbi->var;
	struct fb_ops            *fbops = fbi->fbops;
	TFB_DBG();
	strncpy(fix->id, TINNO_FB_DRIVER, sizeof(fix->id));
	fix->type = FB_TYPE_PACKED_PIXELS;

	switch (var->bits_per_pixel)
	{
	case 16:
	case 24:
	case 32:
		fix->visual = FB_VISUAL_TRUECOLOR;
		break;
	case 1:
	case 2:
	case 4:
	case 8:
		fix->visual = FB_VISUAL_PSEUDOCOLOR;
		break;
	default:
		ASSERT(0);
	}

	fix->accel       = FB_ACCEL_NONE;
	fix->line_length = ALIGN_TO(var->xres_virtual, 32) * var->bits_per_pixel / 8;
	fix->smem_len    = fbdev->fb_size_in_byte;
	fix->smem_start  = fbdev->fb_pa_base;

	fix->xpanstep = 0;
	fix->ypanstep = 1;

	fbops->fb_fillrect  = cfb_fillrect;
	fbops->fb_copyarea  = cfb_copyarea;
	fbops->fb_imageblit = cfb_imageblit;
}


/* Check values in var, try to adjust them in case of out of bound values if
 * possible, or return error.
 */
static int tinno_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *fbi)
{
	unsigned int bpp;
	unsigned long max_frame_size;
	unsigned long line_size;

	struct tinno_fb_device *fbdev = (struct tinno_fb_device *)fbi->par;


	TFB_DBG("xres=%u, yres=%u, xres_virtual=%u, yres_virtual=%u, "
	          "xoffset=%u, yoffset=%u, bits_per_pixel=%u)\n",
	    var->xres, var->yres, var->xres_virtual, var->yres_virtual,
	    var->xoffset, var->yoffset, var->bits_per_pixel);

	bpp = var->bits_per_pixel;

	if (bpp != 16 && bpp != 24 && bpp != 32) {
		TFB_ERR("unsupported bpp: %d", bpp);
		return -1;
	}

	switch (var->rotate) {
	case 0:
	case 180:
		var->xres = TINNO_FB_XRES;
		var->yres = TINNO_FB_YRES;
		break;
	case 90:
	case 270:
		var->xres = TINNO_FB_YRES;
		var->yres = TINNO_FB_XRES;
		break;
	default:
		return -1;
	}

	if (var->xres_virtual < var->xres)
	    var->xres_virtual = var->xres;
	if (var->yres_virtual < var->yres)
	    var->yres_virtual = var->yres;
    
	max_frame_size = fbdev->fb_size_in_byte;
	line_size = var->xres_virtual * bpp / 8;

	if (line_size * var->yres_virtual > max_frame_size) {
		/* Try to keep yres_virtual first */
		line_size = max_frame_size / var->yres_virtual;
		var->xres_virtual = line_size * 8 / bpp;
		if (var->xres_virtual < var->xres) {
			/* Still doesn't fit. Shrink yres_virtual too */
			var->xres_virtual = var->xres;
			line_size = var->xres * bpp / 8;
			var->yres_virtual = max_frame_size / line_size;
		}
	}
	if (var->xres + var->xoffset > var->xres_virtual)
	    var->xoffset = var->xres_virtual - var->xres;
	if (var->yres + var->yoffset > var->yres_virtual)
	    var->yoffset = var->yres_virtual - var->yres;

	if (16 == bpp) {
		var->red.offset    = 11;  var->red.length    = 5;
		var->green.offset  =  5;  var->green.length  = 6;
		var->blue.offset   =  0;  var->blue.length   = 5;
		var->transp.offset =  0;  var->transp.length = 0;
	}
	else if (24 == bpp)
	{
		var->red.length = var->green.length = var->blue.length = 8;
		var->transp.length = 0;

		// Check if format is RGB565 or BGR565

		ASSERT(8 == var->green.offset);
		ASSERT(16 == var->red.offset + var->blue.offset);
		ASSERT(16 == var->red.offset || 0 == var->red.offset);
	}
	else if (32 == bpp)
	{
		var->red.length = var->green.length = 
		var->blue.length = var->transp.length = 8;

		// Check if format is ARGB8888 or ABGR8888

		ASSERT(8 == var->green.offset && 24 == var->transp.offset);
		ASSERT(16 == var->red.offset + var->blue.offset);
		ASSERT(16 == var->red.offset || 0 == var->red.offset);
	}

	TFB_DBG("offset:: transp=%u, red=%u, green=%u, blue=%u\n",
	    var->transp.offset, var->red.offset, var->green.offset, var->blue.offset);


	var->red.msb_right = var->green.msb_right = 
	var->blue.msb_right = var->transp.msb_right = 0;

	var->activate = FB_ACTIVATE_NOW;

	var->height    = UINT_MAX;
	var->width     = UINT_MAX;
	var->grayscale = 0;
	var->nonstd    = 0;

	var->pixclock     = UINT_MAX;
	var->left_margin  = UINT_MAX;
	var->right_margin = UINT_MAX;
	var->upper_margin = UINT_MAX;
	var->lower_margin = UINT_MAX;
	var->hsync_len    = UINT_MAX;
	var->vsync_len    = UINT_MAX;

	var->vmode = FB_VMODE_NONINTERLACED;
	var->sync  = 0;

	return 0;
}

/* Switch to a new mode. The parameters for it has been check already by
 * tinno_fb_check_var.
 */
static int tinno_fb_set_par(struct fb_info *fbi)
{
	struct fb_var_screeninfo *var = &fbi->var;
	struct tinno_fb_device *fbdev = (struct tinno_fb_device *)fbi->par;
	u32 bpp = var->bits_per_pixel;
	TFB_DBG();

	tinno_set_fb_fix(fbdev);
	memset(fbi->screen_base, 0, fbi->screen_size);  // clear the whole VRAM as zero

	return 0;
}

/* Called each time the mtkfb device is opened */
static int tino_fb_open(struct file *file, struct fb_info *info, int user)
{
	TFB_DBG();
	return 0;
}


/* Called when the mtkfb device is closed. We make sure that any pending
 * gfx DMA operations are ended, before we return. */
static int tino_fb_release(struct file *file, struct fb_info *info, int user)
{
	struct tinno_fb_device *fbdev = (struct tinno_fb_device *)info->par;
	TFB_DBG();
	return 0;
}


/* Store a single color palette entry into a pseudo palette or the hardware
 * palette if one is available. For now we support only 16bpp and thus store
 * the entry only to the pseudo palette.
 */
static int tino_fb_setcolreg(u_int regno, u_int red, u_int green,
                           u_int blue, u_int transp,
                           struct fb_info *info)
{
	int r = 0;
	unsigned bpp, m;
	TFB_DBG();

	bpp = info->var.bits_per_pixel;
	m = 1 << bpp;
	if (regno >= m){
		r = -EINVAL;
		goto exit;
	}

	switch (bpp)
	{
	case 16:
		/* RGB 565 */
		((u32 *)(info->pseudo_palette))[regno] = 
			((red & 0xF800) |
			((green & 0xFC00) >> 5) |
			((blue & 0xF800) >> 11));
		break;
	case 32:
		/* ARGB8888 */
		((u32 *)(info->pseudo_palette))[regno] = 
			(0xff000000)           |
			((red   & 0xFF00) << 8) |
			((green & 0xFF00)     ) |
			((blue  & 0xFF00) >> 8);
		break;

	default:
		ASSERT(0);
	}

exit:
    return r;
}

/* Callback table for the frame buffer framework. Some of these pointers
 * will be changed according to the current setting of fb_info->accel_flags.
 */
static struct fb_ops tinno_fb_ops = {
    .owner          = THIS_MODULE,
    .fb_open        = tino_fb_open,
    .fb_release     = tino_fb_release,
    .fb_setcolreg   = tino_fb_setcolreg,
    .fb_pan_display = fb_pan_display,
    .fb_fillrect    = cfb_fillrect,
    .fb_copyarea    = cfb_copyarea,
    .fb_imageblit   = cfb_imageblit,
    .fb_check_var   = tinno_fb_check_var,
    .fb_set_par     = tinno_fb_set_par,
};

/* Initialize system fb_info object and set the default video mode.
 * The frame buffer memory already allocated by lcddma_init
 */
static int tinno_fb_fbinfo_init(struct fb_info *info)
{
	struct tinno_fb_device *fbdev = (struct tinno_fb_device *)info->par;
	struct fb_var_screeninfo var;
	int r = 0;
	TFB_DBG();

	BUG_ON(!fbdev->fb_va_base);
	info->fbops = &tinno_fb_ops;
	info->flags = FBINFO_FLAG_DEFAULT;
	info->screen_base = (char *) fbdev->fb_va_base;
	info->screen_size = fbdev->fb_size_in_byte;
	info->pseudo_palette = fbdev->pseudo_palette;

	r = fb_alloc_cmap(&info->cmap, 16, 0);
	if (r != 0){
		TFB_ERR("unable to allocate color map memory\n");
		return r;
	}

	memset(&var, 0, sizeof(var));
	
	// setup the initial video mode (RGB565)
	var.xres         = TINNO_FB_XRES;
	var.yres         = TINNO_FB_YRES;
	var.xres_virtual = TINNO_FB_XRESV;
	var.yres_virtual = TINNO_FB_YRESV;

	var.bits_per_pixel = 16;

	var.red.offset   = 11; var.red.length   = 5;
	var.green.offset =  5; var.green.length = 6;
	var.blue.offset  =  0; var.blue.length  = 5;

	var.activate = FB_ACTIVATE_NOW;

	r = tinno_fb_check_var(&var, info);
	if (r != 0)
		TFB_ERR("failed to tinno_fb_check_var\n");

	info->var = var;

	r = tinno_fb_set_par(info);
	if (r != 0)
		TFB_ERR("failed to tinno_fb_set_par\n");

	return r;
}

/* Release the fb_info object */
static void tinno_fb_fbinfo_cleanup(struct tinno_fb_device *fbdev)
{
	TFB_DBG();
	fb_dealloc_cmap(&fbdev->fb_info->cmap);
}

static int tinno_fb_probe(struct device *dev)
{
	struct platform_device *pdev;
	struct tinno_fb_device    *fbdev = NULL;
	struct fb_info         *fbi;
	int                    r = 0;
	char *p = NULL;
	TFB_DBG();
	
	pdev = to_platform_device(dev);
	if (pdev->num_resources != 1) {
		TFB_ERR("probed for an unknown device\n");
		r = -ENODEV;
		goto err_to_platform;
	}

	fbi = framebuffer_alloc(sizeof(struct tinno_fb_device), dev);
	if (!fbi) {
		TFB_ERR("unable to allocate memory for device info\n");
		r = -ENOMEM;
		goto err_fb_alloc;
	}
	tinno_fb_fbi = fbi;

	fbdev = (struct tinno_fb_device *)fbi->par;
	fbdev->fb_info = fbi;
	fbdev->dev = dev;
	dev_set_drvdata(dev, fbdev);

	/* Allocate and initialize video frame buffer */

	fbdev->fb_size_in_byte = TINNO_FB_SIZEV;
	{
		struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		fbdev->fb_pa_base = res->start;
		fbdev->fb_va_base = ioremap_nocache(res->start, res->end - res->start + 1);
		ASSERT(tinno_GetFBRamSize() <= (res->end - res->start + 1));
	}

	TFB_ERR("fbdev->fb_pa_base = %x, fbdev->fb_va_base = %x\n", 
		fbdev->fb_pa_base, (unsigned int)(fbdev->fb_va_base));

	if (!fbdev->fb_va_base) {
		TFB_ERR("unable to allocate memory for frame buffer\n");
		r = -ENOMEM;
		goto err_ioremap;
	}


	/* Register to system */

	r = tinno_fb_fbinfo_init(fbi);
	if (r)
		goto err_fb_init;

	r = register_framebuffer(fbi);
	if (r != 0) {
		TFB_ERR("register_framebuffer failed\n");
		goto err_register_fb;
	}

	TFB_DBG("TINNO framebuffer initialized vram=%lu\n", fbdev->fb_size_in_byte);

	return 0;
err_register_fb:
	tinno_fb_fbinfo_cleanup(fbdev);
err_fb_init:
        dma_free_coherent(0, fbdev->fb_size_in_byte,
                      fbdev->fb_va_base, fbdev->fb_pa_base);
err_ioremap:
	tinno_fb_fbi = NULL;
	dev_set_drvdata(fbdev->dev, NULL);
	framebuffer_release(fbdev->fb_info);
err_fb_alloc:
err_to_platform:

    return r;
}










static struct platform_driver tinno_fb_driver = 
{
    .driver = {
        .name    = TINNO_FB_DRIVER,
        .bus     = &platform_bus_type,
        .probe   = tinno_fb_probe,
    },    
};


#ifdef CONFIG_HAS_EARLYSUSPEND

static void tinno_fb_early_suspend(struct early_suspend *h)
{
    TFB_DBG();
}

static void tinno_fb_late_resume(struct early_suspend *h)
{
    TFB_DBG();
}

static struct early_suspend tinno_fb_early_suspend_handler = 
{
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB+1,
	.suspend = tinno_fb_early_suspend,
	.resume = tinno_fb_late_resume,
};
#endif


/* Register both the driver and the device */
int __init tinno_fb_init(void)
{
    int r = 0;

    TFB_DBG();
	
	

    /* Register the driver with LDM */

    if (platform_driver_register(&tinno_fb_driver)) {
        TFB_ERR("failed to register mtkfb driver\n");
        r = -ENODEV;
        goto exit;
    }
   
#ifdef CONFIG_HAS_EARLYSUSPEND
   	register_early_suspend(&tinno_fb_early_suspend_handler);
#endif


exit:
    return r;
}


static void __exit tinno_fb_cleanup(void)
{
    TFB_DBG();

    platform_driver_unregister(&tinno_fb_driver);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&tinno_fb_early_suspend_handler);
#endif
}



module_init(tinno_fb_init);
module_exit(tinno_fb_cleanup);

MODULE_DESCRIPTION("TINNO framebuffer driver");
MODULE_AUTHOR("Jieve Liu <jieve.liu@tinno.com>");
MODULE_LICENSE("GPL");

