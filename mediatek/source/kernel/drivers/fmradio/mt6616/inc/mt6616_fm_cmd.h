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

#ifndef __MT6626_FM_CMD_H__
#define __MT6626_FM_CMD_H__

#include <linux/types.h>
#include "fm_typedef.h"

/* FM basic-operation's opcode */
#define FM_BOP_BASE (0x80)
enum {
    FM_WRITE_BASIC_OP       = (FM_BOP_BASE + 0x00),
    FM_UDELAY_BASIC_OP      = (FM_BOP_BASE + 0x01),
    FM_RD_UNTIL_BASIC_OP    = (FM_BOP_BASE + 0x02),
    FM_MODIFY_BASIC_OP      = (FM_BOP_BASE + 0x03),
    FM_MSLEEP_BASIC_OP      = (FM_BOP_BASE + 0x04),
    FM_MAX_BASIC_OP         = (FM_BOP_BASE + 0x05)
};

/* FM BOP's size */
#define FM_WRITE_BASIC_OP_SIZE      (3)
#define FM_UDELAY_BASIC_OP_SIZE     (4)
#define FM_RD_UNTIL_BASIC_OP_SIZE   (5)
#define FM_MODIFY_BASIC_OP_SIZE     (5)
#define FM_MSLEEP_BASIC_OP_SIZE     (4)

fm_s32 mt6626_pwrup_clock_on(fm_u8 *tx_buf, fm_s32 tx_buf_size);
fm_s32 mt6626_pwrup_digital_init_1(fm_u8 *tx_buf, fm_s32 tx_buf_size);
fm_s32 mt6626_pwrup_digital_init_2(fm_u8 *tx_buf, fm_s32 tx_buf_size);
fm_s32 mt6626_pwrdown(fm_u8 *tx_buf, fm_s32 tx_buf_size);
fm_s32 mt6626_rampdown(fm_u8 *tx_buf, fm_s32 tx_buf_size);
fm_s32 mt6626_tune(fm_u8 *tx_buf, fm_s32 tx_buf_size, fm_u16 freq);
fm_s32 mt6626_seek(fm_u8 *tx_buf, fm_s32 tx_buf_size, fm_u16 seekdir, fm_u16 space, fm_u16 max_freq, fm_u16 min_freq);
fm_s32 mt6626_scan(fm_u8 *tx_buf, fm_s32 tx_buf_size, fm_u16 scandir, fm_u16 space, fm_u16 max_freq, fm_u16 min_freq);
fm_s32 mt6626_get_reg(fm_u8 *tx_buf, fm_s32 tx_buf_size, fm_u8 addr);
fm_s32 mt6626_set_reg(fm_u8 *tx_buf, fm_s32 tx_buf_size, fm_u8 addr, fm_u16 value);
fm_s32 mt6626_patch_download(fm_u8 *tx_buf, fm_s32 tx_buf_size, fm_u8 seg_num, fm_u8 seg_id, const fm_u8 *src, fm_s32 seg_len);
fm_s32 mt6626_coeff_download(fm_u8 *tx_buf, fm_s32 tx_buf_size, fm_u8 seg_num, fm_u8 seg_id, const fm_u8 *src, fm_s32 seg_len);
fm_s32 mt6626_hwcoeff_download(fm_u8 *tx_buf, fm_s32 tx_buf_size, fm_u8 seg_num, fm_u8 seg_id, const fm_u8 *src, fm_s32 seg_len);
fm_s32 mt6626_rom_download(fm_u8 *tx_buf, fm_s32 tx_buf_size, fm_u8 seg_num, fm_u8 seg_id, const fm_u8 *src, fm_s32 seg_len);

#endif
