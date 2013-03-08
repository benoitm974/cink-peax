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

#ifndef __FM_STDLIB_H__
#define __FM_STDLIB_H__

#include "fm_typedef.h"
#include <linux/string.h>

#if 1
#define fm_memset(buf, a, len)  \
({                                    \
    void *__ret = (void*)0;              \
    __ret = memset((buf), (a), (len)); \
    __ret;                          \
})

#define fm_memcpy(dst, src, len)  \
({                                    \
    void *__ret = (void*)0;              \
    __ret = memcpy((dst), (src), (len)); \
    __ret;                          \
})

#define fm_malloc(len)  \
({                                    \
    void *__ret = (void*)0;              \
    __ret = kmalloc(len, GFP_KERNEL); \
    __ret;                          \
})

#define fm_zalloc(len)  \
({                                    \
    void *__ret = (void*)0;              \
    __ret = kzalloc(len, GFP_KERNEL); \
    __ret;                          \
})

#define fm_free(ptr)  kfree(ptr)

#define fm_vmalloc(len)  \
({                                    \
    void *__ret = (void*)0;              \
    __ret = vmalloc(len); \
    __ret;                          \
})

#define fm_vfree(ptr)  vfree(ptr)

#else
inline void* fm_memset(void *buf, fm_s8 val, fm_s32 len)
{
    return memset(buf, val, len);
}

inline void* fm_memcpy(void *dst, const void *src, fm_s32 len)
{
    return memcpy(dst, src, len);
}

#endif

#endif //__FM_STDLIB_H__

