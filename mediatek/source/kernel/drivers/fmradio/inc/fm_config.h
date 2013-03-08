/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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


/* fm_config.h
 *
 * (C) Copyright 2011
 * MediaTek <www.MediaTek.com>
 * hongcheng <hongcheng.xia@MediaTek.com>
 *
 * FM Radio Driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __FM_CONFIG_H__
#define __FM_CONFIG_H__

#include "fm_typedef.h"
#include "fm_rds.h"
#include "fm_dbg.h"
#include "fm_err.h"
#include "fm_stdlib.h"

enum fm_cfg_parser_state {
    FM_CFG_STAT_NONE = 0,
    FM_CFG_STAT_GROUP,
    FM_CFG_STAT_KEY,
    FM_CFG_STAT_VALUE,
    FM_CFG_STAT_COMMENT
};

typedef enum fm_cfg_parser_state fm_cfg_parser_state_t;

#define COMMENT_CHAR '#'
#define DELIMIT_CHAR '='

#define isspace(a) ((a) == 0x20)

#define FAKE_CH_MAX 10
#define FM_CUST_CFG_PATH "/etc/fm_cust.cfg"

struct fm_rx_cust_cfg {
    fm_u16 short_ana_rssi_th;
    fm_u16 long_ana_rssi_th;
    fm_u16 cqi_th;
    fm_u16 mr_th;
    fm_u16 smg_th;
    fm_u16 scan_ch_size;
    fm_u16 seek_space;
    fm_u16 band;
    fm_u16 band_freq_l;
    fm_u16 band_freq_h;
    fm_u16 scan_sort;
    fm_u16 fake_ch[FAKE_CH_MAX];
    fm_u16 fake_ch_num;
    fm_u16 fake_ch_rssi_th;
    fm_u16 deemphasis;
    fm_u16 osc_freq;
};

struct fm_tx_cust_cfg {
    fm_u16 scan_hole_low;
    fm_u16 scan_hole_high;
    fm_u16 power_level;
};

struct fm_cust_cfg {
    struct fm_rx_cust_cfg rx_cfg;
    struct fm_tx_cust_cfg tx_cfg;
};

enum fm_cust_cfg_op {
    FM_CFG_RX_RSSI_TH_LONG = 0,
    FM_CFG_RX_RSSI_TH_SHORT,
    FM_CFG_RX_CQI_TH,
    FM_CFG_RX_MR_TH,
    FM_CFG_RX_SMG_TH,
    FM_CFG_RX_SCAN_CH_SIZE,
    FM_CFG_RX_SEEK_SPACE,
    FM_CFG_RX_BAND,
    FM_CFG_RX_BAND_FREQ_L,
    FM_CFG_RX_BAND_FREQ_H,
    FM_CFG_RX_SCAN_SORT,
    FM_CFG_RX_FAKE_CH_NUM,
    FM_CFG_RX_FAKE_CH_RSSI,
    FM_CFG_RX_FAKE_CH,
    FM_CFG_RX_DEEMPHASIS,
    FM_CFG_RX_OSC_FREQ,

    FM_CFG_TX_SCAN_HOLE_LOW,
    FM_CFG_TX_SCAN_HOLE_HIGH,
    FM_CFG_TX_PWR_LEVEL,
    FM_CFG_MAX
};

typedef fm_s32(*CFG_HANDLER)(fm_s8 *grp, fm_s8 *key, fm_s8 *val, struct fm_cust_cfg *cfg);

extern fm_s32 fm_cust_config_setup(const fm_s8 *filepath);
extern fm_u16 fm_cust_config_fetch(enum fm_cust_cfg_op op_code);

#endif //__FM_CONFIG_H__

