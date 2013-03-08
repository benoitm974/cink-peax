/*
In order to support all versions of Android, no matter what your Touch Screen is support 
multi-touch or not, you should remain the single touch events, such as ABS_TOUCH, 
ABS_PRESSURE, ABS_X, ABS_Y, and so on. In addition, some elder versions of Android 
that before 2.0 support multi-touch by event HAT0X, HAT0Y, therefore you should report 
these events too.

Here is what a minimal event sequence for a two-finger touch would look like:

   ABS_MT_TOUCH_MAJOR
   ABS_MT_POSITION_X
   ABS_MT_POSITION_Y
   SYN_MT_REPORT
   ABS_MT_TOUCH_MAJOR
   ABS_MT_POSITION_X
   ABS_MT_POSITION_Y
   SYN_MT_REPORT
   SYN_REPORT

Note that if you want to report a single point, you should report a SYN_MT_REPORT too.

*/

#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>


#ifdef MT6575
#include <mach/mt6575_pm_ldo.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_boot.h>
#endif

#include "cust_gpio_usage.h"
#include <linux/debug_control.h>

#define TPD_TYPE_CAPACITIVE
#define TPD_HAVE_BUTTON
#ifdef TPD_HAVE_BUTTON
#define TPD_KEY_COUNT		4
#define TPD_KEYS			{KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{60, 830, 60, 120}, {180, 830, 60, 120},	\
							  {300, 830, 60, 120}, {420, 830, 60, 120}}
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
extern void tpd_button(unsigned int x, unsigned int y, unsigned int down) ;
#endif

#define TPD_I2C_GROUP_ID 0
#define TPD_I2C_SLAVE_ADDR (0x7E >> 1)
#define GPIO182_CTP_EN GPIO182
#define GPIO184_CTP_SD GPIO184
#define GPIO186_CTP_RST GPIO186
#define DRIVER_NAME "ft5x05"
extern struct tpd_device *tpd;
 
struct i2c_client *i2c_client = NULL;
struct task_struct *thread = NULL;
 
static DECLARE_WAIT_QUEUE_HEAD(waiter);
static void tpd_eint_interrupt_handler(void);
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
									  kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
									  kal_bool auto_umask);
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect(struct i2c_client *client, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);

static int tpd_flag = 0;
static int point_num = 0;
static int p_point_num = 0;
#define TPD_OK 0

struct touch_info {
    int y[3];
    int x[3];
    int p[3];
    int count;
};

static const struct i2c_device_id ft5x05_tpd_id[] = {{DRIVER_NAME,0},{}};
static struct i2c_board_info __initdata ft5x05_i2c_tpd={ I2C_BOARD_INFO(DRIVER_NAME, 
 	TPD_I2C_SLAVE_ADDR)};
 
static struct i2c_driver tpd_i2c_driver = {
	.probe = tpd_probe,
	.remove = __devexit_p(tpd_remove),
	.id_table = ft5x05_tpd_id,
	.detect = tpd_detect,
	.driver = {
		.name = DRIVER_NAME,
		//.owner = THIS_MODULE,
	},
};
 

static  void tpd_down(int x, int y, int p) 
{
#ifdef TPD_HAVE_BUTTON
	if((x > -100) && (x < 0)) {
		x = 800 - x;
		tpd_button(x, y, 1);
	}			
#endif
	 input_report_key(tpd->dev, BTN_TOUCH, 1);
	 input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	 input_mt_sync(tpd->dev);
	if (FACTORY_BOOT == get_boot_mode()) {   
		tpd_button(x, y, 1);  
	}	 
 }
 
static  int tpd_up(int x, int y, int *count) 
{
	 if(*count>0) {
#ifdef TPD_HAVE_BUTTON
		if((x > -100) && (x < 0)) {
			x = 800 - x;
			tpd_button(x,y,0);
		}
#endif
		 input_report_abs(tpd->dev, ABS_PRESSURE, 0);
		 input_report_key(tpd->dev, BTN_TOUCH, 0);
		 input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
		 input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
		 input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
		 input_mt_sync(tpd->dev);
		 (*count)--;
		 return 1;
	 } 
	 return 0;
 }

 static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)
 {
	int i = 0;	
	char buf[24] = {0};
	u16 high_byte,low_byte;
	p_point_num = point_num;
	/* 8bit max one time */
	i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(buf[0]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x08, 8, &(buf[8]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x10, 8, &(buf[16]));
	mt6575_touch_debug("+%s: --Liu\n", __func__);
#if 0
	mt6575_touch_debug("received raw data from touch panel as following:\n");
	mt6575_touch_debug("[buf[0]=%x, buf[1]= %x, buf[2]=%x ,buf[3]=%x, buf[4]=%x, buf[5]=%x]\n",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	mt6575_touch_debug("[buf[9]=%x, buf[10]=%x, buf[11]=%x, buf[12]=%x]\n",
		buf[9], buf[10], buf[11], buf[12]);
	mt6575_touch_debug("[buf[15]=%x, buf[16]= %x, buf[17]=%x, buf[18]=%x]\n",
		buf[15], buf[16], buf[17], buf[18]);
#endif
	/* Device Mode[2:0] == 0 :Normal operating Mode */
	if (buf[0] & 0x70) 
		return false; 
	/*get the number of the touch points*/
	point_num= buf[2] & 0x0f;
	mt6575_touch_debug("point_num =%d\n",point_num);
	mt6575_touch_debug("Procss raw data...\n");
	for(i = 0; i < point_num; i++) {	
		cinfo->p[i] = buf[3 + 6 * i] >> 6; //event flag 
		/*get the X coordinate, 2 bytes*/
		high_byte = buf[ 3 + 6 * i];
		high_byte <<= 8;
		high_byte &= 0x0f00;
		low_byte = buf[3+6*i + 1];
		cinfo->x[i] = high_byte|low_byte;
		/*get the Y coordinate, 2 bytes*/
		high_byte = buf[3 + 6 * i + 2];
		high_byte <<= 8;
		high_byte &= 0x0f00;
		low_byte = buf[3 + 6 * i + 3];
		cinfo->y[i] = high_byte |low_byte;
	}
	mt6575_touch_debug(" cinfo->x[0]=%d, cinfo->y[0]=%d, cinfo->p[0]=%d\n", 
		cinfo->x[0], cinfo->y[0], cinfo->p[0]);
	if(point_num > 1) {
		mt6575_touch_debug(" cinfo->x[1]=%d, cinfo->y[1]=%d, cinfo->p[1]=%d\n", 
			cinfo->x[1], cinfo->y[1], cinfo->p[1]);
		if(point_num > 2)
			mt6575_touch_debug(" cinfo->x[2]= %d, cinfo->y[2]=%d, cinfo->p[2]=%d\n", 
				cinfo->x[2], cinfo->y[2], cinfo->p[2]);	
	}
	mt6575_touch_debug("-%s: --Liu\n", __func__);		  
	return true;

 };

 static int touch_event_handler(void *unused)
 {
	struct touch_info cinfo, pinfo;
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	sched_setscheduler(current, SCHED_RR, &param);
	do {
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
		set_current_state(TASK_INTERRUPTIBLE); 
		wait_event_interruptible(waiter,tpd_flag!=0);
		tpd_flag = 0;
		set_current_state(TASK_RUNNING);
		if (tpd_touchinfo(&cinfo, &pinfo)) {
			mt6575_touch_debug("point_num = %d\n", point_num);
			if (point_num > 0) {
				tpd_down(cinfo.x[0], cinfo.y[0], 1);
				if (point_num > 1) {
						tpd_down(cinfo.x[1], cinfo.y[1], 1);
						if(point_num > 2) 
							tpd_down(cinfo.x[2], cinfo.y[2], 1);
		             }
				input_sync(tpd->dev);
				mt6575_touch_debug("press --->\n");
			} 
			else  {
				//tpd_up(cinfo.x[0], cinfo.y[0], 0);
				mt6575_touch_debug("release --->\n"); 
				input_mt_sync(tpd->dev);
				input_sync(tpd->dev);
			}
		}
	}while(!kthread_should_stop());
	return 0;
 }
 
 static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info) 
 {
	 strcpy(info->type, TPD_DEVICE);	
	  return 0;
 }
 
 static void tpd_eint_interrupt_handler(void)
 {
	mt6575_touch_debug("TPD interrupt has been triggered\n");
	tpd_flag = 1;
	wake_up_interruptible(&waiter);	 
 }

int ft5x05_get_fw_version(void)
{
	int ret;
	uint8_t fw_version;
	ret = i2c_smbus_read_i2c_block_data(i2c_client, 0xA6, 1, &fw_version);
	if (fw_version < 0 || ret < 0){
		mt6575_touch_info("%s: i2c error, ret=%d--Liu\n", __func__, ret);
		return -1;
	}
	mt6575_touch_info("%s: fw_version=0x%X--Liu\n", __func__, fw_version);
	return (int)fw_version;
}
EXPORT_SYMBOL(ft5x05_get_fw_version);

int get_fw_version_ext(void)
{
	return ft5x05_get_fw_version();
}
EXPORT_SYMBOL(get_fw_version_ext);

 static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
 {	 
	int retval = TPD_OK;
	char data;
	int panel_version = 0;
	int iRetry = 3;
	i2c_client = client;
	mt6575_touch_info("%s: FT5x05--Liu\n", __func__);
	hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_1800, "touch");    	

	mt_set_gpio_mode(GPIO182_CTP_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO182_CTP_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO182_CTP_EN, GPIO_OUT_ONE);

	mt_set_gpio_mode(GPIO184_CTP_SD, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO184_CTP_SD, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO184_CTP_SD, GPIO_OUT_ONE);
	msleep(5);

	mt_set_gpio_mode(GPIO186_CTP_RST, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO186_CTP_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO186_CTP_RST, GPIO_OUT_ONE);
	msleep(100);

	while (iRetry) {
		panel_version = ft5x05_get_fw_version();
		mt6575_touch_info("Product version is %d--Liu\n", panel_version);
		if ( panel_version < 0 ){
			mt_set_gpio_out(GPIO186_CTP_RST, GPIO_OUT_ONE);
			mdelay(1);
			mt_set_gpio_out(GPIO186_CTP_RST, GPIO_OUT_ZERO);
			mdelay(1);
			mt_set_gpio_out(GPIO186_CTP_RST, GPIO_OUT_ONE);
		}else{
			break;
		}
		iRetry--;
	} 

	if((i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &data))< 0) {
		   mt6575_touch_info("%s: I2C error--Liu\n", __func__);
		   return -1; 
	}
	
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, 
		CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, 
		CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, 
		CUST_EINT_TOUCH_PANEL_POLARITY, 
		tpd_eint_interrupt_handler, 1); 
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	msleep(100);
	
	tpd_load_status = 1;
	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	 if (IS_ERR(thread)) { 
		retval = PTR_ERR(thread);
		mt6575_touch_info(" failed to create kernel thread: %d\n", retval);
	}
	return 0; 
 }

 static int __devexit tpd_remove(struct i2c_client *client) 
 {
	mt6575_touch_info("%s: FT5x05--Liu\n", __func__);
	return 0;
 }
 
 static int tpd_local_init(void)
 {
	mt6575_touch_info("%s: FT5x05--Liu\n", __func__);
	if(i2c_add_driver(&tpd_i2c_driver) != 0)
   	{
		mt6575_touch_info("%s: unable to add FT5x05 i2c driver--Liu\n", __func__);
      		return -1;
	}
#ifdef TPD_HAVE_BUTTON     
	tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif   	  
	tpd_type_cap = 1;
	return 0; 
 }

 static int tpd_resume(struct early_suspend *h)
 {
	int retval = TPD_OK;
	mt6575_touch_info("%s: FT5x05--Liu\n", __func__);
	
	mt_set_gpio_mode(GPIO186_CTP_RST, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO186_CTP_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO186_CTP_RST, GPIO_OUT_ZERO);  
	msleep(1);  
	mt_set_gpio_mode(GPIO186_CTP_RST, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO186_CTP_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO186_CTP_RST, GPIO_OUT_ONE);
	
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
	
	 return retval;
 }
 
 static int tpd_suspend(struct early_suspend *h)
 {
	int retval = TPD_OK;
	mt6575_touch_info("%s: FT5x05--Liu\n", __func__);
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	//retval = i2c_smbus_write_byte_data(client, 0xA5, 0x03);
	return retval;
 } 


 static struct tpd_driver_t tpd_device_driver = {
		 .tpd_device_name = "FT5x05",
		 .tpd_local_init = tpd_local_init,
		 .suspend = tpd_suspend,
		 .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		 .tpd_have_button = 1,
#else
		 .tpd_have_button = 0,
#endif		
 };

 static int __init tpd_driver_init(void) {
	mt6575_touch_info("%s: FT5x05--Liu\n", __func__);
	i2c_register_board_info(0, &ft5x05_i2c_tpd, 1);
	if(tpd_driver_add(&tpd_device_driver) < 0)
			 mt6575_touch_info("add FT5x05 driver failed\n");
	return 0;
 }
 
 /* should never be called */
 static void __exit tpd_driver_exit(void) {
	mt6575_touch_info("%s: FT5x05--Liu\n", __func__);
	//input_unregister_device(tpd->dev);
	tpd_driver_remove(&tpd_device_driver);
 }
 
 module_init(tpd_driver_init);
 module_exit(tpd_driver_exit);
