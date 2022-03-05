/*
 * $Id: mcast.c,v 1.2 Broadcom SDK $
 *
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
 */
#include <soc/error.h>
#include <soc/types.h>
#include <soc/mcm/robo/memregs.h>
#include <soc/drv_if.h>
#include <assert.h>
#include <soc/register.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/mcast.h>
#include <bcm/mcast.h>

/*
 *  Function : drv_bcm5396_mcast_bmp_get
 *
 *  Purpose :
 *      Get the multicast member ports from multicast entry
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry   :   entry data pointer 
 *      bmp     :   group port member
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int  
drv_bcm5396_mcast_bmp_get(int unit, uint32 *entry, soc_pbmp_t *bmp)
{
    int rv = SOC_E_NONE;
    uint32	pbmp;
    
    assert(entry);
    assert(bmp);

    /* get the multicast id */
    if ((rv = (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
            entry, &pbmp)) < 0) {
        return rv;
    }
    
    SOC_PBMP_WORD_SET(*bmp, 0, pbmp);

    soc_cm_debug(DK_L2TABLE, 
        "drv_mcast_bmp_get: unit %d, bmp = %x\n",
        unit, SOC_PBMP_WORD_GET(*bmp, 0));
    return rv;
}

 /*
 *  Function : drv_bcm5396_mcast_bmp_set
 *
 *  Purpose :
 *      Set the multicast member ports from multicast entry
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry   :   entry data pointer 
 *      bmp     :   group port member
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int  
drv_bcm5396_mcast_bmp_set(int unit, uint32 *entry, soc_pbmp_t bmp, uint32 flag)
{
    int rv;
    uint32  temp;
    uint32  reg_addr, reg_value, reg_len;
    uint32  fld_value = 0;
    
    assert(entry);
    soc_cm_debug(DK_L2TABLE, 
        "drv_mcast_bmp_set: unit %d, bmp = %x flag %x\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), flag);
    
    /* enable IP_Multicast scheme (specific for Robo5324)
     *   - here we enable IP_Multicast scheme each time when set mcast_pbm.
     */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, NEW_CONTROLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, NEW_CONTROLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, NEW_CONTROLr, &reg_value, IP_MULTICASTf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    fld_value= SOC_PBMP_WORD_GET(bmp, 0);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
            entry, &fld_value));

        /* Insert this address into arl table. */
     SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_insert)
            (unit, DRV_MEM_ARL, entry,
             (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID 
               | DRV_MEM_OP_REPLACE)));

    return rv;
}
