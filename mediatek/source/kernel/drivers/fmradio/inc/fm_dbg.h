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

#ifndef __FM_DBG_H__
#define __FM_DBG_H__

#include <linux/kernel.h> //for printk()

//DBG zone
#define BASE	4
#define MAIN	(1 << (BASE+0))
#define LINK	(1 << (BASE+1))
#define EINT	(1 << (BASE+2))
#define CHIP	(1 << (BASE+3))
#define RDSC	(1 << (BASE+4))
#define G0		(1 << (BASE+5))
#define G1		(1 << (BASE+6))
#define G2		(1 << (BASE+7))
#define G3		(1 << (BASE+8))
#define G4		(1 << (BASE+9))
#define G14		(1 << (BASE+10))
#define RAW		(1 << (BASE+11))
#define OPEN	(1 << (BASE+12))
#define IOCTL	(1 << (BASE+13))
#define READ_	(1 << (BASE+14))
#define CLOSE	(1 << (BASE+15))
#define CQI     (1 << (BASE+16))
#define ALL		0xfffffff0

//DBG level
#define L0 0x00000000 // EMERG, system will crush 
#define L1 0x00000001 // ALERT, need action in time
#define L2 0x00000002 // CRIT, important HW or SW operation failed 
#define L3 0x00000003 // ERR, normal HW or SW ERR 
#define L4 0x00000004 // WARNING, importan path or somewhere may occurs err
#define L5 0x00000005 // NOTICE, normal case 
#define L6 0x00000006 // INFO, print info if need 
#define L7 0x00000007 // DEBUG, for debug info

#define FM_EMG	L0
#define FM_ALT	L1
#define FM_CRT	L2
#define FM_ERR	L3
#define FM_WAR	L4
#define FM_NTC	L5
#define FM_INF	L6
#define FM_DBG	L7

extern fm_u32 g_dbg_level;
#define WCN_DBG(flag, fmt, args...) \
	do { \
		if ((((flag)&0x0000000f)<=(g_dbg_level&0x0000000f)) && ((flag)&0xfffffff0)& g_dbg_level) { \
			printk(KERN_ALERT "[" #flag "]" fmt, ## args); \
		} \
	} while(0)



#endif //__FM_DBG_H__

