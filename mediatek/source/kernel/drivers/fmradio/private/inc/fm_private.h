/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __FM_PRIVATE_H__
#define __FM_PRIVATE_H__

#include "fm_typedef.h"

typedef enum fm_priv_state {
    UNINITED,
    INITED
} fm_priv_state_t;

typedef enum fm_adpll_state {
    FM_ADPLL_ON,
    FM_ADPLL_OFF
} fm_adpll_state_t;

typedef enum fm_hl_dese {
    FM_HL_DESE_LOW,
    FM_HL_DESE_HIGH
} fm_hl_dese_t;

typedef enum fm_adpll_clk {
    FM_ADPLL_16M,
    FM_ADPLL_15M
} fm_adpll_clk_t;

typedef enum fm_mcu_desense {
    FM_MCU_DESE_ENABLE,
    FM_MCU_DESE_DISABLE
} fm_mcu_desense_t;

typedef enum fm_gps_desense {
    FM_GPS_DESE_ENABLE,
    FM_GPS_DESE_DISABLE
} fm_gps_desense_t;

typedef struct fm_priv_cb {
    //De-sense functions.
    fm_s32(*is_dese_chan)(fm_u16 freq);             // check if this is a de-sense channel
    fm_s32(*hl_dese)(fm_u16 freq, void *arg);       // return value: 0, low side; 1, high side; else error no
    fm_s32(*fa_dese)(fm_u16 freq, void *arg);       // return value: 0, fa off; 1, fa on; else error no
    fm_s32(*mcu_dese)(fm_u16 freq, void *arg);      // return value: 0, mcu dese disable; 1, enable; else error no
    fm_s32(*gps_dese)(fm_u16 freq, void *arg);      // return value: 0,mcu dese disable; 1, enable; else error no
    fm_u16(*chan_para_get)(fm_u16 freq);            //get channel parameter, HL side/ FA / ATJ
} fm_priv_cb_t;

typedef struct fm_priv {
    fm_s32 state;
    struct fm_priv_cb priv_tbl;
    void *data;
} fm_priv_t;


typedef struct fm_pub_cb {
    //Basic functions.
    fm_s32(*read)(fm_u8 addr, fm_u16 *val);
    fm_s32(*write)(fm_u8 addr, fm_u16 val);
    fm_s32(*setbits)(fm_u8 addr, fm_u16 bits, fm_u16 mask);
    fm_s32(*rampdown)(void);
    fm_s32(*msdelay)(fm_u32 val);
    fm_s32(*usdelay)(fm_u32 val);
    fm_s32(*log)(const fm_s8 *arg1, ...);
} fm_pub_cb_t;

typedef struct fm_pub {
    fm_s32 state;
    void *data;
    struct fm_pub_cb pub_tbl;
} fm_pub_t;


extern fm_s32 fm_priv_register(struct fm_priv *pri, struct fm_pub *pub);
extern fm_s32 fm_priv_unregister(struct fm_priv *pri, struct fm_pub *pub);

#endif //__FM_PRIVATE_H__