/*
 * debug_control.h -- for Audio, USB, mmc debug
 *
 * Copyright (C) 2010 Liu Yang
 * All rights reserved.
 *
 */
 #ifndef __DEBUG_CONTROL__
 #define __DEBUG_CONTROL__
void mt6575_audio_debug(const char *fmt, ...);
void mt6575_audio_info(const char *fmt, ...);
void mt6575_usb_debug(const char *fmt, ...);
void mt6575_usb_info(const char *fmt, ...);
void mt6575_mmc_debug(const char *fmt, ...);
void mt6575_mmc_info(const char *fmt, ...);
void mt6575_gps_debug(const char *fmt, ...);
void mt6575_gps_info(const char *fmt, ...);
void mt6575_touch_debug(const char *fmt, ...);
void mt6575_touch_info(const char *fmt, ...);
#else
#endif