/*
 * $Id: eav.c,v 1.20 Broadcom SDK $
 * $Copyright: Copyright 2011 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * Field Processor related CLI commands
 */

#include <soc/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>
#include <soc/drv.h>
#include <soc/debug.h>


#define EAV_BCM5395_MAX_TICK_INC    63
#define EAV_BCM5395_MAX_TICK_ADJUST_PERIOD    15
#define EAV_BCM5395_MAX_SLOT_ADJUST_PERIOD    15
#define EAV_BCM5395_MAX_TICK_ONE_SLOT    3126
#define EAV_BCM5395_MAX_SLOT_NUMBER    31

#define EAV_BCM53118_MAX_PCP_VALUE    0x7

/* Bytes count allowed for EAV Class4/Class5 bandwidth within a slot time */
#define EAV_5395_MAX_BANDWIDTH_VALUE 16383
#define EAV_5395_MIN_BANDWIDTH_VALUE 0

static int
_drv_bcm5395_eav_mmu_init(int unit)
{
    uint32 reg_val;

    /* 1. MMU settings provided by ASIC*/
    /* Hysteresis threshold */
    reg_val = 110; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa10, &reg_val, 2);
    reg_val = 111;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa12, &reg_val, 2);
    reg_val = 111;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa14, &reg_val, 2);
    reg_val = 112;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa16, &reg_val, 2);
    reg_val = 112;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab0, &reg_val, 2);
    reg_val = 112;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab2, &reg_val, 2);

    /* Pause threshold */
    reg_val = 232; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa18, &reg_val, 2);
    reg_val = 233;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1a, &reg_val, 2);
    reg_val = 234;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1c, &reg_val, 2);
    reg_val = 235;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1e, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab4, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab6, &reg_val, 2);

    /* Drop threshold */
    reg_val = 500; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa20, &reg_val, 2);
    reg_val = 500;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa22, &reg_val, 2);
    reg_val = 500;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa24, &reg_val, 2);
    reg_val = 500;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa26, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab8, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xaba, &reg_val, 2);

    /* Total reserved threshold */
    reg_val = 1; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa40, &reg_val, 2);
    reg_val = 1;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa42, &reg_val, 2);
    reg_val = 1;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa44, &reg_val, 2);
    reg_val = 1;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa46, &reg_val, 2);
    reg_val = 18;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa48, &reg_val, 2);
    reg_val = 24;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa4a, &reg_val, 2);

    /* IMP Hysteresis threshold */
    reg_val = 122; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd10, &reg_val, 2);
    reg_val = 123;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd12, &reg_val, 2);
    reg_val = 123;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd14, &reg_val, 2);
    reg_val = 124;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd16, &reg_val, 2);

    /* IMP Pause threshold */
    reg_val = 244; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd18, &reg_val, 2);
    reg_val = 245;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1a, &reg_val, 2);
    reg_val = 246;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1c, &reg_val, 2);
    reg_val = 247;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1e, &reg_val, 2);

    /* IMP Drop threshold */
    reg_val = 511; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd20, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd22, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd24, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd26, &reg_val, 2);

    /* Total Hysteresis threshold */
    reg_val = 108; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa28, &reg_val, 2);
    reg_val = 109;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2a, &reg_val, 2);
    reg_val = 110;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2c, &reg_val, 2);
    reg_val = 111;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2e, &reg_val, 2);
    reg_val = 111;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xabc, &reg_val, 2);
    reg_val = 111;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xabe, &reg_val, 2);

    /* Total Pause threshold */
    reg_val = 246; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa30, &reg_val, 2);
    reg_val = 248;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa32, &reg_val, 2);
    reg_val = 250;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa34, &reg_val, 2);
    reg_val = 252;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa36, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac0, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac2, &reg_val, 2);

    /* Total Drop threshold */
    reg_val = 378; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa38, &reg_val, 2);
    reg_val = 380;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3a, &reg_val, 2);
    reg_val = 382;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3c, &reg_val, 2);
    reg_val = 384;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3e, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac4, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac6, &reg_val, 2);

    /* Total IMP Hysteresis threshold */
    reg_val = 138; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd28, &reg_val, 2);
    reg_val = 139;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2a, &reg_val, 2);
    reg_val = 140;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2c, &reg_val, 2);
    reg_val = 141;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2e, &reg_val, 2);

    /* Total IMP Pause threshold */
    reg_val = 276; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd30, &reg_val, 2);
    reg_val = 278;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd32, &reg_val, 2);
    reg_val = 280;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd34, &reg_val, 2);
    reg_val = 282;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd36, &reg_val, 2);

    /* Total IMP Drop threshold */
    reg_val = 408; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd38, &reg_val, 2);
    reg_val = 410;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3a, &reg_val, 2);
    reg_val = 412;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3c, &reg_val, 2);
    reg_val = 414;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3e, &reg_val, 2);

    return SOC_E_NONE;
}

static int
_drv_bcm53101_eav_mmu_init(int unit)
{
    uint32 reg_val;

    /* 1. MMU settings provided by ASIC*/
    /* Hysteresis threshold */
    reg_val = 9; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa10, &reg_val, 2);
    reg_val = 9;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa12, &reg_val, 2);
    reg_val = 10;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa14, &reg_val, 2);
    reg_val = 10;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa16, &reg_val, 2);
    reg_val = 10;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab0, &reg_val, 2);
    reg_val = 10;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab2, &reg_val, 2);

    /* Pause threshold */
    reg_val = 18; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa18, &reg_val, 2);
    reg_val = 19;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1a, &reg_val, 2);
    reg_val = 20;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1c, &reg_val, 2);
    reg_val = 21;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1e, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab4, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab6, &reg_val, 2);

    /* Drop threshold */
    reg_val = 252; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa20, &reg_val, 2);
    reg_val = 252;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa22, &reg_val, 2);
    reg_val = 252;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa24, &reg_val, 2);
    reg_val = 252;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa26, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab8, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xaba, &reg_val, 2);

    /* Txq reserved threshold */
    reg_val = 6; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa40, &reg_val, 2);
    reg_val = 6;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa42, &reg_val, 2);
    reg_val = 6;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa44, &reg_val, 2);
    reg_val = 6;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa46, &reg_val, 2);
    reg_val = 6;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa48, &reg_val, 2);
    reg_val = 6;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa4a, &reg_val, 2);

    /* IMP Hysteresis threshold */
    reg_val = 10; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd10, &reg_val, 2);
    reg_val = 11;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd12, &reg_val, 2);
    reg_val = 11;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd14, &reg_val, 2);
    reg_val = 12;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd16, &reg_val, 2);

    /* IMP Pause threshold */
    reg_val = 21; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd18, &reg_val, 2);
    reg_val = 22;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1a, &reg_val, 2);
    reg_val = 23;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1c, &reg_val, 2);
    reg_val = 24;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1e, &reg_val, 2);

    /* IMP Drop threshold */
    reg_val = 255; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd20, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd22, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd24, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd26, &reg_val, 2);

    /* Total Hysteresis threshold */
    reg_val = 45; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa28, &reg_val, 2);
    reg_val = 45;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2a, &reg_val, 2);
    reg_val = 46;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2c, &reg_val, 2);
    reg_val = 46;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2e, &reg_val, 2);
    reg_val = 46;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xabc, &reg_val, 2);
    reg_val = 46;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xabe, &reg_val, 2);

    /* Total Pause threshold */
    reg_val = 90; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa30, &reg_val, 2);
    reg_val = 91;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa32, &reg_val, 2);
    reg_val = 92;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa34, &reg_val, 2);
    reg_val = 93;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa36, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac0, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac2, &reg_val, 2);

    /* Total Drop threshold */
    reg_val = 118; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa38, &reg_val, 2);
    reg_val = 119;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3a, &reg_val, 2);
    reg_val = 120;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3c, &reg_val, 2);
    reg_val = 121;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3e, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac4, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac6, &reg_val, 2);

    /* Total IMP Hysteresis threshold */
    reg_val = 48; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd28, &reg_val, 2);
    reg_val = 47;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2a, &reg_val, 2);
    reg_val = 47;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2c, &reg_val, 2);
    reg_val = 48;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2e, &reg_val, 2);

    /* Total IMP Pause threshold */
    reg_val = 93; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd30, &reg_val, 2);
    reg_val = 94;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd32, &reg_val, 2);
    reg_val = 95;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd34, &reg_val, 2);
    reg_val = 96;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd36, &reg_val, 2);

    /* Total IMP Drop threshold */
    reg_val = 121; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd38, &reg_val, 2);
    reg_val = 122;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3a, &reg_val, 2);
    reg_val = 123;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3c, &reg_val, 2);
    reg_val = 124;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3e, &reg_val, 2);

    /* WAN & IMP1 */
    /* IMP1 Hysteresis threshold */
    reg_val = 12; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe10, &reg_val, 2);
    reg_val = 12;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe12, &reg_val, 2);
    reg_val = 13;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe14, &reg_val, 2);
    reg_val = 13;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe16, &reg_val, 2);

    /* IMP1 Pause threshold */
    reg_val = 24; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe18, &reg_val, 2);
    reg_val = 25;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe1a, &reg_val, 2);
    reg_val = 26;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe1c, &reg_val, 2);
    reg_val = 27;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe1e, &reg_val, 2);

    /* IMP1 Drop threshold */
    reg_val = 255; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe20, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe22, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe24, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe26, &reg_val, 2);

    /* Total IMP1 Hysteresis threshold */
    reg_val = 48; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe28, &reg_val, 2);
    reg_val = 48;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe2a, &reg_val, 2);
    reg_val = 49;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe2c, &reg_val, 2);
    reg_val = 49;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe2e, &reg_val, 2);

    /* Total IMP1 Pause threshold */
    reg_val = 96; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe30, &reg_val, 2);
    reg_val = 97;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe32, &reg_val, 2);
    reg_val = 98;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe34, &reg_val, 2);
    reg_val = 99;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe36, &reg_val, 2);

    /* Total IMP1 Drop threshold */
    reg_val = 124; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe38, &reg_val, 2);
    reg_val = 125;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe3a, &reg_val, 2);
    reg_val = 126;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe3c, &reg_val, 2);
    reg_val = 127;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe3e, &reg_val, 2);

    return SOC_E_NONE;
}

int 
drv_bcm5395_eav_control_set(int unit, uint32 type, uint32 param)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    switch (type) {
        case DRV_EAV_CONTROL_TIME_STAMP_TO_IMP:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, TM_STAMP_RPT_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, TM_STAMP_RPT_CTRLr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, TM_STAMP_RPT_CTRLr, 
                &reg_value, TSRPT_PKT_ENf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        case DRV_EAV_CONTROL_MAX_AV_SIZE:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_MAX_AV_PKT_SZr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_MAX_AV_PKT_SZr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            temp = param;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_MAX_AV_PKT_SZr, 
                &reg_value, MAX_AV_PKT_SZf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSA_PCP:
            if (SOC_IS_ROBO53118(unit)) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, CLASS_PCPr, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, CLASS_PCPr);

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value,reg_len));

                if (param > EAV_BCM53118_MAX_PCP_VALUE) {
                    return SOC_E_PARAM;
                }

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, CLASS_PCPr, &reg_value, CLASSA_PCPf, &param));

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value,reg_len));
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSB_PCP:
            if (SOC_IS_ROBO53118(unit)) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, CLASS_PCPr, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, CLASS_PCPr);

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value,reg_len));

                if (param > EAV_BCM53118_MAX_PCP_VALUE) {
                    return SOC_E_PARAM;
                }

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, CLASS_PCPr, &reg_value, CLASSB_PCPf, &param));

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value,reg_len));
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_EAV_CONTROL_MMU_INIT:
            if (SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53115(unit) ||
                SOC_IS_ROBO53118(unit) || SOC_IS_ROBO53128(unit)) {
                rv = _drv_bcm5395_eav_mmu_init(unit);
            } else if (SOC_IS_ROBO53101(unit)) {
                rv = _drv_bcm53101_eav_mmu_init(unit);
            }
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}


int 
drv_bcm5395_eav_control_get(int unit, uint32 type, uint32 *param)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    switch (type) {
        case DRV_EAV_CONTROL_TIME_STAMP_TO_IMP:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, TM_STAMP_RPT_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, TM_STAMP_RPT_CTRLr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, TM_STAMP_RPT_CTRLr, 
                &reg_value, TSRPT_PKT_ENf, &temp));
            if (temp) {
                *param = TRUE;
            } else {
                *param = FALSE;
            }
            break;
        case DRV_EAV_CONTROL_MAX_AV_SIZE:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_MAX_AV_PKT_SZr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_MAX_AV_PKT_SZr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_MAX_AV_PKT_SZr, 
                &reg_value, MAX_AV_PKT_SZf, &temp));
            *param = temp;
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSA_PCP:
            if (SOC_IS_ROBO53118(unit)) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, CLASS_PCPr, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, CLASS_PCPr);

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value,reg_len));

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, CLASS_PCPr, &reg_value, CLASSA_PCPf, &temp));
                *param = temp;
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSB_PCP:
            if (SOC_IS_ROBO53118(unit)) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, CLASS_PCPr, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, CLASS_PCPr);

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value,reg_len));

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, CLASS_PCPr, &reg_value, CLASSB_PCPf, &temp));
                *param = temp;
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

int 
drv_bcm5395_eav_enable_set(int unit, uint32 port, uint32 enable)
{
    uint32 reg_addr, reg_value, temp;
    int reg_len;

    /* Set EAV enable register */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, RESE_AV_EN_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, RESE_AV_EN_CTRLr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, RESE_AV_EN_CTRLr, &reg_value, AV_ENf, &temp));
    if (enable){
        temp |= 0x1 << port;
    } else {
        temp &= ~(0x1 << port);
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, RESE_AV_EN_CTRLr, &reg_value, AV_ENf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value,reg_len));
    
    return SOC_E_NONE;
}

int 
drv_bcm5395_eav_enable_get(int unit, uint32 port, uint32 *enable)
{
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, RESE_AV_EN_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, RESE_AV_EN_CTRLr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, RESE_AV_EN_CTRLr, &reg_value, AV_ENf, &temp));
    if (temp & (0x1 << port)){
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }
    return SOC_E_NONE;
}

int 
drv_bcm5395_eav_link_status_set(int unit, uint32 port, uint32 link)
{
    uint32 reg_addr, reg_value, temp;
    int reg_len;

    /* Set EAV Link register */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, EAV_LNK_STATUSr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, EAV_LNK_STATUSr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, EAV_LNK_STATUSr, &reg_value, 
        PT_EAV_LNK_STATUSf, &temp));
    if (link){
        temp |= 0x1 << port;
    } else {
        temp &= ~(0x1 << port);
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAV_LNK_STATUSr, &reg_value, 
        PT_EAV_LNK_STATUSf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value,reg_len));
    
    return SOC_E_NONE;
}

int 
drv_bcm5395_eav_link_status_get(int unit, uint32 port, uint32 *link)
{
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, EAV_LNK_STATUSr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, EAV_LNK_STATUSr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, EAV_LNK_STATUSr, &reg_value, 
        PT_EAV_LNK_STATUSf, &temp));
    if (temp & (0x1 << port)){
        *link = TRUE;
    } else {
        *link = FALSE;
    }
    return SOC_E_NONE;
}

int
drv_bcm5395_eav_egress_timestamp_get(int unit, uint32 port,
    uint32 *timestamp)
{
    uint32 reg_addr, reg_value, temp;
    int reg_len;

    /* Check Valid Status */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, TM_STAMP_STATUSr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, TM_STAMP_STATUSr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, TM_STAMP_STATUSr, &reg_value, VALID_STATUSf, &temp));
    
    if ((temp & (0x1 << port)) == 0) {
        return SOC_E_EMPTY;
    }

    /* Get Egress Time STamp Value */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, RESE_EGRESS_TM_STAMPr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, RESE_EGRESS_TM_STAMPr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    *timestamp = reg_value;
    return SOC_E_NONE;
}


int 
drv_bcm5395_eav_time_sync_set(int unit, uint32 type, uint32 p0, uint32 p1)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    switch (type) {
        case DRV_EAV_TIME_SYNC_TIME_BASE:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_TM_BASEr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_TM_BASEr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            temp = p0;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_TM_BASEr, 
                &reg_value, TM_BASEf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        case DRV_EAV_TIME_SYNC_TIME_ADJUST:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_TM_ADJr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_TM_ADJr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            if ((p0 > EAV_BCM5395_MAX_TICK_INC) || 
                (p1 > EAV_BCM5395_MAX_TICK_ADJUST_PERIOD)) {
                return SOC_E_PARAM;
            }
            temp = p0;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_TM_ADJr, 
                &reg_value, TM_INCf, &temp));
            temp = p1;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_TM_ADJr, 
                &reg_value, TM_ADJ_PRDf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        case DRV_EAV_TIME_SYNC_TICK_COUNTER:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_SLOT_TICK_CNTRr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_SLOT_TICK_CNTRr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            if (p0 > EAV_BCM5395_MAX_TICK_ONE_SLOT) {
                return SOC_E_PARAM;
            }
            temp = p0;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_SLOT_TICK_CNTRr, 
                &reg_value, TICK_CNTRf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        case DRV_EAV_TIME_SYNC_SLOT_NUMBER:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_SLOT_TICK_CNTRr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_SLOT_TICK_CNTRr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            if (p0 > EAV_BCM5395_MAX_SLOT_NUMBER) {
                return SOC_E_PARAM;
            }
            temp = p0;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_SLOT_TICK_CNTRr, 
                &reg_value, SLOT_NUMf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        case DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_SLOT_ADJr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_SLOT_ADJr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            switch (p0) {
                case 1:
                    temp = 0;
                    break;
                case 2:
                    temp = 1;
                    break;
                case 4:
                    temp = 2;
                    break;
                default:
                    rv = SOC_E_PARAM;
                    return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_SLOT_ADJr, 
                &reg_value, MCRO_SLOT_PRDf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        case DRV_EAV_TIME_SYNC_SLOT_ADJUST:
            
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_SLOT_ADJr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_SLOT_ADJr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            switch (p0) {
                case 3125:
                    temp = 0;
                    break;
                case 3126:
                    temp = 1;
                    break;
                case 3124:
                    temp = 2;
                    break;
                default:
                    rv = SOC_E_PARAM;
                    return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_SLOT_ADJr, 
                &reg_value, SLOT_ADJf, &temp));
            if (p1 >= 16) {
                rv =  SOC_E_PARAM;
                return rv;
            }
            temp = p1;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_SLOT_ADJr, 
                &reg_value, SLOT_ADJ_PRDf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

int 
drv_bcm5395_eav_time_sync_get(int unit, uint32 type, uint32 *p0, uint32 *p1)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    switch (type) {
        case DRV_EAV_TIME_SYNC_TIME_BASE:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_TM_BASEr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_TM_BASEr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_TM_BASEr, 
                &reg_value, TM_BASEf, &temp));
            *p0 = temp;
            break;
        case DRV_EAV_TIME_SYNC_TIME_ADJUST:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_TM_ADJr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_TM_ADJr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_TM_ADJr, 
                &reg_value, TM_INCf, &temp));
            *p0 = temp;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_TM_ADJr, 
                &reg_value, TM_ADJ_PRDf, &temp));
            *p1 = temp;
            break;
        case DRV_EAV_TIME_SYNC_TICK_COUNTER:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_SLOT_TICK_CNTRr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_SLOT_TICK_CNTRr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_SLOT_TICK_CNTRr, 
                &reg_value, TICK_CNTRf, &temp));
            *p0 = temp;
            break;
        case DRV_EAV_TIME_SYNC_SLOT_NUMBER:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_SLOT_TICK_CNTRr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_SLOT_TICK_CNTRr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_SLOT_TICK_CNTRr, 
                &reg_value, SLOT_NUMf, &temp));
            *p0 = temp;
            break;
        case DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_SLOT_ADJr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_SLOT_ADJr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_SLOT_ADJr, 
                &reg_value, MCRO_SLOT_PRDf, &temp));
            switch(temp) {
                case 0:
                    *p0 = 1;
                    break;
                case 1:
                    *p0 = 2;
                    break;
                case 2:
                    *p0 = 4;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    return rv;
            }
            break;
        case DRV_EAV_TIME_SYNC_SLOT_ADJUST:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_SLOT_ADJr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_SLOT_ADJr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_SLOT_ADJr, 
                &reg_value, SLOT_ADJf, &temp));
            switch (temp) {
                case 0:
                    *p0 = 3125;
                    break;
                case 1:
                    *p0 = 3126;
                    break;
                case 2:
                    *p0 = 3124;
                    break;
                default:
                    return SOC_E_INTERNAL;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_SLOT_ADJr, 
                &reg_value, SLOT_ADJ_PRDf, &temp));
            *p1 = temp;
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

int 
drv_bcm5395_eav_queue_control_set(int unit, 
    uint32 port, uint32 type, uint32 param)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    switch (type) {
        case DRV_EAV_QUEUE_Q4_BANDWIDTH:
            /* Q4 BW maxmum value = 16383(0x3fff) */
            if (param > EAV_5395_MAX_BANDWIDTH_VALUE) {
                soc_cm_debug(DK_ERR, 
                    "drv_bcm5395_eav_queue_control_set : BW unsupported. \n");
                return  SOC_E_PARAM;
            }
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_C4_BW_CNTLr, port, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_C4_BW_CNTLr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            temp = param;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_C4_BW_CNTLr, 
                &reg_value, C4_BWf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH:
            /* Q5 BW maxmum value = 16383(0x3fff) */
    /*    coverity[unsigned_compare]    */
            if (param > EAV_5395_MAX_BANDWIDTH_VALUE) {
                soc_cm_debug(DK_ERR, 
                    "drv_bcm5395_eav_queue_control_set : BW unsupported. \n");
                return  SOC_E_PARAM;
            }
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_C5_BW_CNTLr, port, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_C5_BW_CNTLr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            temp = param;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_C5_BW_CNTLr, 
                &reg_value, C5_BWf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        case DRV_EAV_QUEUE_Q5_WINDOW:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_C5_BW_CNTLr, port, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_C5_BW_CNTLr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, RESE_C5_BW_CNTLr, 
                &reg_value, C5_WNDWf, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}
int 
drv_bcm5395_eav_queue_control_get(int unit, 
    uint32 port, uint32 type, uint32 *param)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_value, temp, max_value = 0;
    int reg_len;
    soc_field_info_t    *finfop = 0;
    
    switch (type) {
        case DRV_EAV_QUEUE_Q4_BANDWIDTH:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_C4_BW_CNTLr, port, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_C4_BW_CNTLr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_C4_BW_CNTLr, 
                &reg_value, C4_BWf, &temp));
            *param = temp;
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_C5_BW_CNTLr, port, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_C5_BW_CNTLr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_C5_BW_CNTLr, 
                &reg_value, C5_BWf, &temp));
            *param = temp;
            break;
        case DRV_EAV_QUEUE_Q5_WINDOW:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RESE_C5_BW_CNTLr, port, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RESE_C5_BW_CNTLr);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, RESE_C5_BW_CNTLr, 
                &reg_value, C5_WNDWf, &temp));
            if (temp){
                *param = TRUE;
            } else {
                *param = FALSE;
            }
            break;
        case DRV_EAV_QUEUE_Q4_BANDWIDTH_MAX_VALUE:
            /*
              * Get the maximum valid bandwidth value for EAV Class 4 (macro slot time = 1)
              *
              * C4_Bandwidth(bytes/slot) = 
              *     Max_value(kbits/sec) * 1024 / (8 * macro slot time * 1000)
              *
              * C4_Bandwidth (14 bits) = 0x3fff
              * Max_value = (((1<<14) * 8 * macro slot time* 1000)/(1024)) - 1
              */
            SOC_FIND_FIELD(C4_BWf,
                SOC_REG_INFO(unit, RESE_C4_BW_CNTLr).fields,
                SOC_REG_INFO(unit, RESE_C4_BW_CNTLr).nFields,
                finfop);
            assert(finfop);

            temp = (1 << finfop->len);
            max_value = ((temp * 8 * 1 * 1000) / (1024)) - 1;
            *param = max_value;
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH_MAX_VALUE:
            /*
              * Get the maximum valid bandwidth value for EAV Class 5
              *
              * Class 5 slot time is 125 us.
              * C5_Bandwidth(bytes/125us) = Max_value(kbits/sec) * 1024 / (8 * 8000)
              *
              * C5_Bandwidth (14 bits) = 0x3fff
              * Max_value = (((1<<14) * 8 * 8000)/(1024)) - 1
              */
            SOC_FIND_FIELD(C5_BWf,
                SOC_REG_INFO(unit, RESE_C5_BW_CNTLr).fields,
                SOC_REG_INFO(unit, RESE_C5_BW_CNTLr).nFields,
                finfop);
            assert(finfop);

            temp = (1 << finfop->len);
            max_value = ((temp * 8 * 8000) / 1024) - 1;
            *param = max_value;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}


int 
drv_bcm5395_eav_time_sync_mac_set(int unit, uint8* mac, uint16 ethertype)
{
    uint32 reg_addr, reg_val;
    uint64 reg_val64, mac_field;
    uint32 temp;
    int rv =  SOC_E_NONE, reg_len;

    /*
     * For time sync protocol, the mac should be set in Multi-address 0 register
     */

    /* 1. Set MAC and Ethertype value */
    SAL_MAC_ADDR_TO_UINT64(mac, mac_field);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MULTIPORT_ADDR0r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MULTIPORT_ADDR0r);

    COMPILER_64_ZERO(reg_val64);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, MULTIPORT_ADDR0r, (uint32 *)&reg_val64, 
                        MPORT_ADDRf, (uint32 *)&mac_field));
    if (ethertype) {
        temp = ethertype;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                        (unit, MULTIPORT_ADDR0r, (uint32 *)&reg_val64, 
                            MPORT_E_TYPEf, &temp));
    }
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_val64, reg_len)) < 0) {
        return rv;
    }

    /* 2. Set Forward map to CPU only */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MPORTVEC0r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MPORTVEC0r);
    temp  = SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0);
    reg_val = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, MPORTVEC0r, &reg_val, PORT_VCTRf, &temp));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_val, reg_len)) < 0) {
        return rv;
    }

    /* 3. Enable Multi-address o */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MULTI_PORT_CTLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MULTI_PORT_CTLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_val, reg_len)) < 0) {
        return rv;
    }
    /* Set the match condition are MAC/Ethertype */
    if (ethertype) {
        temp = DRV_MULTIPORT_CTRL_MATCH_ETYPE_ADDR;
    } else {
        temp = DRV_MULTIPORT_CTRL_MATCH_ADDR;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, MULTI_PORT_CTLr, &reg_val, MPORT_CTRL0f, &temp));
    /* Enable time stamped to CPU */
    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, MULTI_PORT_CTLr, &reg_val, MPORT0_TS_ENf, &temp));    

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_val, reg_len)) < 0) {
        return rv;
    }

    return rv;
    
}

int 
drv_bcm5395_eav_time_sync_mac_get(int unit, uint8* mac, uint16 *ethertype)
{
    uint32 reg_addr, reg_val;
    uint64 reg_val64, mac_field;
    uint32 temp;
    int rv =  SOC_E_NONE, reg_len;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MULTI_PORT_CTLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MULTI_PORT_CTLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_val, reg_len)) < 0) {
        return rv;
    }
    /* Get the value of time sync enable */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, MULTI_PORT_CTLr, &reg_val, MPORT0_TS_ENf, &temp));
    if ( temp == 0) {
        rv = SOC_E_DISABLED;
        return rv;
    }
    /* Get the Multi-address control value */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, MULTI_PORT_CTLr, &reg_val, MPORT_CTRL0f, &temp));
    if (temp == DRV_MULTIPORT_CTRL_DISABLE) {
        rv = SOC_E_DISABLED;
        return rv;
    }

    /* Get the MAC and Ethertype value */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MULTIPORT_ADDR0r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MULTIPORT_ADDR0r);

    COMPILER_64_ZERO(reg_val64);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_val64, reg_len)) < 0) {
        return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, MULTIPORT_ADDR0r, (uint32 *)&reg_val64, 
                        MPORT_ADDRf, (uint32 *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(mac, mac_field);
   
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, MULTIPORT_ADDR0r, (uint32 *)&reg_val64, 
                        MPORT_E_TYPEf, &temp));
    *ethertype = temp;

    return rv;
    
}
