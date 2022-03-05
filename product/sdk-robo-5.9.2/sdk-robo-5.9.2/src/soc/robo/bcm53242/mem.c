/*
 * $Id: mem.c,v 1.48 Broadcom SDK $
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
 * SOC Memory (Table) Utilities
 */

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <sal/appl/io.h>

#include <soc/l2x.h>
#include <soc/cmic.h>
#include <soc/error.h>
#include <soc/register.h>
#include <soc/mcm/robo/driver.h>
#include <soc/spi.h>
#include <soc/arl.h>

#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))

#define BCM53242_TXDSC_MEMORY 1 
#define BCM53242_ARL_MEMORY  0

#define BCM53242_MEM_OP_WRITE    0
#define BCM53242_MEM_OP_READ    1

#define VLAN_MEMORY 2 
#define ARL_MEMORY  3

/* Define Table Type for Access */
#define BCM53242_ARL_TABLE_ACCESS       0x0
#define BCM53242_VLAN_TABLE_ACCESS      0x1
#define BCM53242_MARL_PBMP_TABLE_ACCESS 0x2
#define BCM53242_MSPT_TABLE_ACCESS      0x3
#define BCM53242_VLAN2VLAN_TABLE_ACCESS       0x4
#define BCM53242_MAC2VLAN_TABLE_ACCESS      0x5
#define BCM53242_PROTOCOL2VLAN_TABLE_ACCESS 0x6
#define BCM53242_FLOW2VLAN_TABLE_ACCESS      0x7

/* ---- internal MASK ---- */
#define BCM53242_MASK_PROTOCOL2VLAN_INDEX   0xF

/* MAC2VLAN hash bucket id */
#define BCM53242_MACVLAN_BIN0       0
#define BCM53242_MACVLAN_BIN1       1
#define BCM53242_MACVLAN_UNAVAIL    -1

/*
 *  Function : _drv_bcm53242_mem_macvlan_locate
 *
 *  Purpose :
 *      special write procedure to MAC2VLAN table
 *
 *  Parameters :
 *      unit    :   unit id
 *      mac_addr:   MAC address (key) for s
 *      data0   :   (OUTPUT)entry data0 pointer
 *      data1   :   (OUTPUT)entry data1 pointer
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
_drv_bcm53242_mem_macvlan_locate(int unit, sal_mac_addr_t mac_addr, 
                uint64 *data0, uint64 *data1)
{
    uint32  reg_addr, reg_len, reg_val32, count, temp;
    uint64  reg_val64 = 0, mac_field = 0;
    soc_control_t   *soc = SOC_CONTROL(unit);
    int rv = SOC_E_NONE;
    
    MEM_LOCK(unit, MAC2VLANm);
    /* 1. set arla_mac and arla_vid */
    if (mac_addr == NULL){
        return SOC_E_PARAM;
    }
    
    SAL_MAC_ADDR_TO_UINT64(mac_addr, mac_field);
    
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_MACr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_MACr, 0, 0);
    
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_MACr, (uint32 *)&reg_val64, 
            MAC_ADDR_INDXf, (uint32 *)&mac_field));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_val64, reg_len));
 
   /* always set arla_vid to zero to prevent the improper MAC2VLAN table
     * hash result.
     */
    temp = 0;
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_VIDr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_VIDr, 0, 0);

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VIDr, &reg_val32, ARLA_VIDTAB_INDXf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_val32, reg_len));

    /* 2. set arla_rwctl(read), check for command DONE. */
    MEM_RWCTRL_REG_LOCK(soc);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_RWCTLr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_val32, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_locate_exit;
    }
    temp = MEM_TABLE_READ;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, TAB_RWf, &temp);
    temp = BCM53242_MAC2VLAN_TABLE_ACCESS;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, TAB_INDEXf, &temp);
    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, ARL_STRTDNf, &temp);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, (uint32 *)&reg_val32, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_locate_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_val32, 1)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_locate_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_RWCTLr, &reg_val32, ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_locate_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);

    /* 3. get othere_table_data0 , othere_table_data1 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA0r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA0r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)data0, reg_len)) < 0) {
        goto mem_locate_exit;
    }
    
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)data1, reg_len)) < 0) {
        goto mem_locate_exit;
    }

 mem_locate_exit:
    MEM_UNLOCK(unit, MAC2VLANm);

    return SOC_E_NONE;
}

/*
 *  Function : _drv_bcm53242_mem_macvlan_search
 *
 *  Purpose :
 *      special search procedure to MAC2VLAN table
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry       :   entry data pointer
 *      mv_entry    :   (OUTPUT)search return entry
 *      index       :   (OUTPUTindex key if exist.
 *                          here we used to indicate the bin ID
 *                          (MAC2VLAN is a hashed table).
 *              1). if exist, index is that existed entry bin_id
 *              2). if not_found, then
 *                      >> index = -1, bin0 & bin1 both unavailable.
 *                      >> index = 0, bin0 is avaliable. 
 *                      >> index = 1, bin1 is avaiable.
 *                      P.S. both available got index = 0 still.
 *
 *  Return :
 *      SOC_E_EXISTS | SOC_E_NOT_FOUND
 *
 *  Note :
 *      The proper design should integrates this routine into 
 *      formal mem_search(). --- TBD 
 *
 */
int 
_drv_bcm53242_mem_macvlan_search(int unit, uint32 *entry, 
                mac2vlan_entry_t *mv_entry, uint32 *index)
{
    mac2vlan_entry_t    mv_entry0, mv_entry1;
    sal_mac_addr_t      ent_mac_addr, loc_mac_addr;
    uint32      loc_valid;
    uint64      mac_field;

    /* 1. get the hashed entrys */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                    (uint32 *)entry, (uint32 *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(ent_mac_addr, mac_field);       
    
    sal_memset(mv_entry, 0, sizeof(mac2vlan_entry_t));
    sal_memset(&mv_entry0, 0, sizeof(mac2vlan_entry_t));
    sal_memset(&mv_entry1, 0, sizeof(mac2vlan_entry_t));
    SOC_IF_ERROR_RETURN(_drv_bcm53242_mem_macvlan_locate(
                    unit, ent_mac_addr, (uint64 *)&mv_entry0, 
                    (uint64 *)&mv_entry1));
    
    /* get the data0 vlaid bit */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                    (uint32 *)&mv_entry0, (uint32 *)&loc_valid));
                    
    if (loc_valid){
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                        (uint32 *)&mv_entry0, (uint32 *)&mac_field));
        SAL_MAC_ADDR_FROM_UINT64(loc_mac_addr, mac_field);
        if (!SAL_MAC_ADDR_CMP(loc_mac_addr, ent_mac_addr)){
            mv_entry = &mv_entry0;
            *index = BCM53242_MACVLAN_BIN0;
            return SOC_E_EXISTS;
        } else {
            /* assign to bin1 first for not found case */
            *index = BCM53242_MACVLAN_BIN1;     
        }
    } else {
        /* assign to bin0 for not found case */
        *index = BCM53242_MACVLAN_BIN0;     
    }
    
    /* get the data1 vlaid bit */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                    (uint32 *)&mv_entry1, (uint32 *)&loc_valid));
                    
    if (loc_valid){
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                        (uint32 *)&mv_entry1, (uint32 *)&mac_field));
        SAL_MAC_ADDR_FROM_UINT64(loc_mac_addr, mac_field);
        if (!SAL_MAC_ADDR_CMP(loc_mac_addr, ent_mac_addr)){
            mv_entry = &mv_entry1;
            *index = BCM53242_MACVLAN_BIN1;
            return SOC_E_EXISTS;
        } else {
            if (*index == BCM53242_MACVLAN_BIN1){
                *index = BCM53242_MACVLAN_UNAVAIL; 
            }
        }
    }
   
    return SOC_E_NOT_FOUND;
}

/*
 *  Function : _drv_bcm53242_mem_macvlan_write
 *
 *  Purpose :
 *      special write procedure to MAC2VLAN table
 *
 *  Parameters :
 *      unit    :   unit id
 *      entry   :   entry data pointer
 *      flags       :   flag control.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
_drv_bcm53242_mem_macvlan_write(int unit, uint32 *entry, uint32 flags)
{
    mac2vlan_entry_t    mv_entry0, mv_entry1;
    sal_mac_addr_t      ent_mac_addr, loc_mac_addr0, loc_mac_addr1;
    uint32              ent_valid, loc_valid0, loc_valid1;
    uint32      reg_addr, reg_len, reg_val32, count, temp, reg_id;
    uint64      reg_val64, mac_field;
    soc_control_t   *soc = SOC_CONTROL(unit);
    int rv = SOC_E_NONE;

    if (entry == NULL){
        return SOC_E_PARAM;
    }
    
    /* get valid field */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)entry, &ent_valid));
                    
    /* 1. get the hashed entrys */                 
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                (uint32 *)entry, (uint32 *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(ent_mac_addr, mac_field);       
    
    sal_memset(&mv_entry0, 0, sizeof(mac2vlan_entry_t));
    sal_memset(&mv_entry1, 0, sizeof(mac2vlan_entry_t));
    _drv_bcm53242_mem_macvlan_locate(unit, ent_mac_addr, 
                (uint64 *)&mv_entry0, (uint64 *)&mv_entry1);
    
    /* get MAC 0/1 */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                (uint32 *)&mv_entry0, (uint32 *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(loc_mac_addr0, mac_field);
    
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                (uint32 *)&mv_entry1, (uint32 *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(loc_mac_addr1, mac_field);
    
    /* get valid 0/1 */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)&mv_entry0, &loc_valid0));
    
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)&mv_entry1, &loc_valid1));
    
    /* get the target entry to write :
     *  - default write to bin0 unless :
     *      1. valid1 + (MAC1 == writing_MAC).  #### bin1
     *      2. valid0 + (MAC0 != writing_MAC) and no valid1.    #### bin1
     */
    
    if ((SAL_MAC_ADDR_CMP(loc_mac_addr1, ent_mac_addr) == 0) && 
                    loc_valid1 == 1){
        reg_id = OTHER_TABLE_DATA1r;
    } else if ((SAL_MAC_ADDR_CMP(loc_mac_addr0, ent_mac_addr) != 0) && 
                    (loc_valid0 == 1) && loc_valid1 == 0){
        reg_id = OTHER_TABLE_DATA1r;
    } else {
        reg_id = OTHER_TABLE_DATA0r;
    }
    
    /* start to write ------------- */
    MEM_LOCK(unit, GEN_MEMORYm);
    
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, reg_id, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, reg_id);

    /* write to target entry (data0 or data1) */
    if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, (uint32 *)entry, reg_len)) < 0) {
            goto mv_mem_write_exit;
    }

    /*  1. clear arla_vid */
    temp = 0;
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_VIDr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_VIDr, 0, 0);
     
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
             (unit, ARLA_VIDr, &reg_val32, ARLA_VIDTAB_INDXf, &temp));
    
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
         (unit, reg_addr, &reg_val32, reg_len));

    /*  2. set arla_mac */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_MACr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_MACr, 0, 0);
    
    SAL_MAC_ADDR_TO_UINT64(ent_mac_addr, mac_field);       
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_MACr, (uint32 *)&reg_val64, 
            MAC_ADDR_INDXf, (uint32 *)&mac_field));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_val64, reg_len));
    
    /*  3. set arla_rwctl */
    MEM_RWCTRL_REG_LOCK(soc);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_RWCTLr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_val32, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mv_mem_write_exit;
    }
    temp = MEM_TABLE_WRITE;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, TAB_RWf, &temp);
    temp = BCM53242_MAC2VLAN_TABLE_ACCESS;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, TAB_INDEXf, &temp);
    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, ARL_STRTDNf, &temp);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, (uint32 *)&reg_val32, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mv_mem_write_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_val32, 1)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mv_mem_write_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_RWCTLr, &reg_val32, ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mv_mem_write_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);

 mv_mem_write_exit:
    MEM_UNLOCK(unit, GEN_MEMORYm);    
        
    return rv;
}


/*
 *  Function : _drv_bcm53242_mem_macvlan_delete
 *
 *  Purpose :
 *      special delete procedure to MAC2VLAN table
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry       :   entry data pointer
 *      flags       :   flag control.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
_drv_bcm53242_mem_macvlan_delete(int unit, uint32 *entry, uint32 flags)
{
    mac2vlan_entry_t    m2v_output;
    uint64  reg_val64, mac_field;
    uint32  reg_addr, reg_len, reg_val32, count, temp, reg_id;
    uint32  bin_id;
    soc_control_t   *soc = SOC_CONTROL(unit);
    int rv;

    if (entry == NULL){
        return SOC_E_PARAM;
    }
    sal_memset(&m2v_output, 0, sizeof(mac2vlan_entry_t));
    if ((rv = _drv_bcm53242_mem_macvlan_search(unit, 
                    entry, &m2v_output, &bin_id))< 0) {
        if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
            return rv;;
        }
    }
    
    if (rv == SOC_E_EXISTS){

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                    (uint32 *)entry, (uint32 *)&mac_field));
        
        /* start to write ------------- */
        MEM_LOCK(unit, GEN_MEMORYm);
        reg_id = (bin_id) ? OTHER_TABLE_DATA1r : OTHER_TABLE_DATA0r;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                            (unit, reg_id, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
                            (unit, reg_id);

        /* write to target entry (data0 or data1) */
        sal_memset(&m2v_output, 0, sizeof(mac2vlan_entry_t));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
                        (unit, reg_addr, (uint32 *)&m2v_output, reg_len)) < 0) {
                goto mv_mem_delete_exit;
        }

        /*  1. set arla_mac */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_MACr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_MACr, 0, 0);
        
       SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_MACr, (uint32 *)&reg_val64, 
                MAC_ADDR_INDXf, (uint32 *)&mac_field));

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_val64, reg_len));
        
        /*  2. set arla_rwctl */
        MEM_RWCTRL_REG_LOCK(soc);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_RWCTLr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_val32, 1)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mv_mem_delete_exit;
        }
        temp = MEM_TABLE_WRITE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &reg_val32, TAB_RWf, &temp);
        temp = BCM53242_MAC2VLAN_TABLE_ACCESS;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &reg_val32, TAB_INDEXf, &temp);
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &reg_val32, ARL_STRTDNf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, (uint32 *)&reg_val32, 1)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mv_mem_delete_exit;
        }

        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_val32, 1)) < 0) {
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mv_mem_delete_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_RWCTLr, &reg_val32, ARL_STRTDNf, &temp);
            if (!temp)
                break;
        }

        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mv_mem_delete_exit;
        }
        MEM_RWCTRL_REG_UNLOCK(soc);

 mv_mem_delete_exit:
        MEM_UNLOCK(unit, GEN_MEMORYm);   
    }
    
    return rv;
}

/*
 *  Function : _drv_bcm53242_mem_protocolvlan_search
 *
 *  Purpose :
 *      special search procedure to PROTOCOL2VLAN table
 *
 *  Parameters :
 *      unit    :   unit id
 *      entry   :   entry data pointer
 *      pv_entry    :   (OUTPUT)search return entry
 *      index       :   (OUTPUT)index for exist.
 *
 *  Return :
 *      SOC_E_EXISTS | SOC_E_NOT_FOUND
 *
 *  Note :
 *      The proper design should integrates this routine into 
 *      formal mem_search(). --- TBD 
 *
 */
int 
_drv_bcm53242_mem_protocolvlan_search(int unit, uint32 *entry, 
                protocol2vlan_entry_t *pv_entry, uint32 *index)
{
    uint32  field_temp, ether_type, ent_id;

    *index =  -1;
    /* get target ether_type */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_ETHER_TYPE,
            entry, &field_temp));
    
    ent_id = field_temp & BCM53242_MASK_PROTOCOL2VLAN_INDEX;

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_read)
                    (unit, DRV_MEM_PROTOCOLVLAN, 
                    ent_id, 1, (uint32 *)pv_entry));
    
    /* get current ether_type */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_ETHER_TYPE,
            (uint32 *)pv_entry, &ether_type));
            
    if (ether_type == field_temp){
        
        /* get current valid bit */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)pv_entry, &field_temp));
        if (!field_temp) {
            return SOC_E_NOT_FOUND;
        }
    } else {
        return SOC_E_NOT_FOUND;
    }

    *index = ent_id;
    return SOC_E_EXISTS;
}

/*
 *  Function : _drv_bcm53242_mem_macvlan_delete
 *
 *  Purpose :
 *      special delete procedure to MAC2VLAN table
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry       :   entry data pointer
 *      flags       :   flag control.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
_drv_bcm53242_mem_protococlvlan_delete(int unit, uint32 *entry, uint32 flags)
{
    protocol2vlan_entry_t    p2v_output;
    uint32  field_val32;
    uint32  ent_id;
    int rv;
    
    sal_memset(&p2v_output, 0, sizeof(protocol2vlan_entry_t));
    if ((rv = _drv_bcm53242_mem_protocolvlan_search(unit, 
                    entry, &p2v_output, &ent_id))< 0) {
        if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
            return rv;;
        }
    }
    
    if (rv == SOC_E_EXISTS){

        /* set valid bit */
        field_val32 = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)&p2v_output, &field_val32));
        /* set vid */
        field_val32 = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VLANID,
                (uint32 *)&p2v_output, &field_val32));
        /* set priority */
        field_val32 = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_PRIORITY,
                (uint32 *)&p2v_output, &field_val32));
                
        /* set priority */
        field_val32 = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_ETHER_TYPE,
                (uint32 *)&p2v_output, &field_val32));
        
        /* PROTOCOL2VLAN write */
        rv = ((DRV_SERVICES(unit)->mem_write)
                        (unit, DRV_MEM_PROTOCOLVLAN,
                        ent_id, 1, (uint32 *)&p2v_output));
                
    }
    
    return rv;
}

/*
 *  Function : _drv_bcm53242_mem_search
 *
 *  Purpose :
 *      Search selected memory for the key value
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      key   :   the pointer of the data to be search.
 *      entry     :   entry data pointer (if found).
 *      flags     :   search flags.
 *      index   :   entry index in this memory bank (0 or 1).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. In the MAC+VID hash search section, there are two search operations 
 *      will be preformed.
 *    a. Match basis search : report the valid entry with MAC+VID matched.
 *    b. Conflict basis search : report all valid entries in the MAC+VID 
 *      hashed bucket.
 */
int 
_drv_bcm53242_mem_search(int unit, uint32 mem, uint32 *key, 
                                    uint32 *entry, uint32 *entry_1, uint32 flags, int *index)
{
    int             rv = SOC_E_NONE;
    soc_control_t           *soc = SOC_CONTROL(unit);
    uint32          count, temp, value;
    uint32          control = 0;
    uint8           mac_addr_rw[6], temp_mac_addr[6];
    uint64          rw_mac_field, temp_mac_field;
    uint64          entry0, entry1, *input;
    uint32          vid_rw, vid1 = 0, vid2 = 0; 
    int             binNum = -1, existed = 0;
    uint32          reg_addr, reg_value;
    int             reg_len, i;
    uint32          process, search_valid;
    uint32          src_port;
    uint32 mcast_index;

    if (flags & DRV_MEM_OP_SEARCH_DONE) {
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_SRCH_CTLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_SRCH_CTLr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_search_done;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_SRCH_CTLr, &reg_value, 
            ARLA_SRCH_STDNf, &temp)) < 0) {
            goto mem_search_done;
        }
        if (temp) {
            rv = SOC_E_BUSY;    
        }else {
            rv = SOC_E_NONE;
        }
mem_search_done:
        return rv;
            
    } else if (flags & DRV_MEM_OP_SEARCH_VALID_START) {
        /* Set ARL Search Control register */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_SRCH_CTLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_SRCH_CTLr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &control, reg_len)) < 0) {
            goto mem_search_valid_start;
        }
        process = 1;
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, ARLA_SRCH_CTLr, 
                &control, ARLA_SRCH_STDNf, &process));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &control, reg_len)) < 0) {
            goto mem_search_valid_start;
        }
mem_search_valid_start:
        return rv;

    } else if (flags & DRV_MEM_OP_SEARCH_VALID_GET) {
        if (flags & DRV_MEM_OP_SEARCH_PORT) {
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    key, &src_port)) < 0) {
                goto mem_search_valid_get;
            }
        }
 
        process = 1;
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)(unit, 
                (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_SRCH_CTLr, 0, 0), &control, 1)) < 0) {
                goto mem_search_valid_get;
            }
            (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_SRCH_CTLr, 
                &control, ARLA_SRCH_STDNf, &process);
            (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_SRCH_CTLr, 
                &control, ARLA_SRCH_VLIDf, &search_valid);
            /* ARL search operation was done */
            if (!process){
                break;
            }
            if (!search_valid){
                continue;
            }
            count = 0;
            temp = 1;
            /* Set VALID field */
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp);

            if (!(flags & DRV_MEM_OP_SEARCH_PORT)) {
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                          (unit, ARLA_SRCH_ADRr);
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                           (unit, ARLA_SRCH_ADRr, 0, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    goto mem_search_valid_get;
                }
                if ((rv = (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_ADRr, &reg_value, 
                    SRCH_ADRf, key)) < 0) {
                    goto mem_search_valid_get;
                }
            }

            /* Read ARL Search Result VID Register */
            vid_rw = 0;
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)(unit, 
                ARLA_SRCH_RSLT_VIDr, 0, 0), &reg_value, 
                (DRV_SERVICES(unit)->reg_length_get)
                (unit,ARLA_SRCH_RSLT_VIDr))) < 0) {
                goto mem_search_valid_get;
            }
            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                ARLA_SRCH_RSLT_VIDr, &reg_value, 
                ARLA_SRCH_RSLT_VIDf, &vid_rw);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw);

            /* Read ARL Search Result MAC register */
            (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)(unit, 
                ARLA_SRCH_RSLTr, 0, 0), (uint32 *)&entry0, 8);
                    
            if (flags & DRV_MEM_OP_SEARCH_PORT) {
                (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                ARLA_SRCH_RSLT_PRIDf, &temp);
                if (temp != src_port) {
                    continue;
                }
            }

            /* MAC Address */
            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                ARLA_SRCH_RSLT_ADDRf, (uint32 *)&temp_mac_field);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                entry, (uint32 *)&temp_mac_field);
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            if (temp_mac_addr[0] & 0x01) { /* mcast address */
                /* The the multicast format didn't define, we need 
                   collect 3 fields to get the multicast index value.
                   multicast index : bit 55~48
                   age : bit 55
                   priority : bit 54~53
                   port id : bit 52~48
                */
                mcast_index = 0;
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_PRIDf, &temp);
                mcast_index = temp;
                 
                /* Port number should add 24 for BCM53242 */
                mcast_index += 24;

                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ENTRY_RSRV0f, &temp);
                mcast_index += (temp << 6);
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_AGEf, &temp);
                mcast_index += (temp << 11);
                ((DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
                    entry, (uint32 *)&mcast_index)); 

                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_STATICf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry, &temp); 
                /* arl_control */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARL_SRCH_RSLT_CONf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                    entry, &temp); 

            } else { /* unicast address */
                /* Source Port Number */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_PRIDf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        entry, &temp); 
            
                /* Static Bit */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_STATICf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry, &temp); 

                /* Hit bit */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_AGEf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                    entry, &temp); 

                /* arl_control */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARL_SRCH_RSLT_CONf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                    entry, &temp); 

            }
            rv = SOC_E_EXISTS;
            goto mem_search_valid_get;
        }
        rv = SOC_E_TIMEOUT;
mem_search_valid_get:
        return rv;

    } else if (flags & DRV_MEM_OP_BY_INDEX) {
        /*
         * Since no one calls this case, left it empty temporary.
         */
        soc_cm_print("_drv_bcm53242_mem_search: flag = DRV_MEM_OP_BY_INDEX\n");
        return SOC_E_UNAVAIL;
    /* delete by MAC */    
    } else if (flags & DRV_MEM_OP_BY_HASH_BY_MAC) {
        l2_arl_sw_entry_t   *rep_entry;
        int is_conflict[ROBO_53242_L2_BUCKET_SIZE];

        if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
            for (i = 0; i < ROBO_53242_L2_BUCKET_SIZE; i++){
                /* check the parameter for output entry buffer */
                rep_entry = (l2_arl_sw_entry_t *)entry + i;
                if (rep_entry == NULL){
                    soc_cm_debug(DK_WARN, 
                            "%s,entries buffer not allocated!\n", 
                            FUNCTION_NAME());
                    return SOC_E_PARAM;
                }

                sal_memset(rep_entry, 0, sizeof(l2_arl_sw_entry_t));
                is_conflict[i] = FALSE;
            }
        }
    
        ARL_MEM_SEARCH_LOCK(soc);
        /* enable 802.1Q and set VID+MAC to hash */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL0r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_search_exit;
        }
        temp = 1;
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, VLAN_CTRL0r, &reg_value, VLAN_ENf, &temp));
        
        /* check IVL or SVL */
        SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)
                (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp));
        if (temp == 3){     /* VLAN is at IVL mode */
            if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                temp = 3;
            } else {
                temp = 0;
            }
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp));
        }
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_search_exit;
        }
        /* Write MAC Address Index Register */
        if ((rv = (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                key, (uint32 *)&rw_mac_field)) < 0) {
            goto mem_search_exit;
        }
        SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_MACr, 0, 0), &rw_mac_field, 6)) < 0) {
            goto mem_search_exit;
        }

        if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
            /* Write VID Table Index Register */
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, key, &vid_rw)) < 0) {
                goto mem_search_exit;
            }
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_VIDr, 0, 0), &vid_rw, 2)) < 0) {
                goto mem_search_exit;
            }
        }

        /* Write ARL Read/Write Control Register */
        /* Read Operation */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            goto mem_search_exit;
        }
        temp = MEM_TABLE_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &control, TAB_RWf, &temp);

        temp = BCM53242_ARL_TABLE_ACCESS;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &control, TAB_INDEXf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            goto mem_search_exit;
        }

        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
                goto mem_search_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
            if (!temp)
                break;
        }

        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_search_exit;
        }

        /* Read Operation sucessfully */
        /* Get the ARL Entry 0/1 Register */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_ENTRY_0r, 0, 0), (uint32 *)&entry0, 8)) < 0) {
            goto mem_search_exit;
        }

        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_ENTRY_1r, 0, 0), (uint32 *)&entry1, 8)) < 0) {
            goto mem_search_exit;
        }

        /* check DRV_MEM_OP_REPLACE */
        if (flags & DRV_MEM_OP_REPLACE) {
            uint32 temp_valid1 = 0;
            uint32 temp_valid2 = 0;
            uint32 temp_static1 = 0;
            uint32 temp_static2 = 0;
        
            soc_cm_debug(DK_ARL, 
                "DRV_MEM_OP_REPLACE\n");

            /* Check the ARL Entry 0 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRY_0r, (uint32 *)&entry0, ARL_VALIDf, &temp_valid1);

            /* Check the ARL Entry 0 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRY_0r, (uint32 *)&entry0, ARL_STATICf, &temp_static1);

            /* Check the ARL Entry 1 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRY_1r, (uint32 *)&entry1, ARL_VALIDf, &temp_valid2);

            /* Check the ARL Entry 1 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRY_1r, (uint32 *)&entry1, ARL_STATICf, &temp_static2);
            
            if (temp_valid1) {
                if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                    is_conflict[0] = TRUE;
                }
                
                /* bin 0 valid, check mac or mac+vid */
                /* get mac_addr */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_ENTRY_0r, (uint32 *)&entry0, ARL_MACADDRf,
                    (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                /* get vid */
                (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_VID_ENTRY_0r, 0, 0), &vid1, 2);          

                /* check if we have to overwrite this valid bin0 */
                if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                    /* check mac + vid */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6) && (vid1 == vid_rw)) {
                        /* select bin 0 to overwrite it */
                            binNum = 0;
                    }
                } else {
                    /* check mac */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                        /* select bin 0 to overwrite it */                
                            binNum = 0;
                    }                
                } 
            } 
        
            if (temp_valid2) {
                /* bin 1 valid */          

                if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                    is_conflict[1] = TRUE;
                }

                /* get mac_addr */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_ENTRY_1r, (uint32 *)&entry1, ARL_MACADDRf,
                    (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                /* get vid */
                (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_VID_ENTRY_1r, 0, 0), &vid2, 2);
                
                /* check if we have to overwrite this valid bin0 */
                if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                    /* check mac + vid */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6) && (vid2 == vid_rw)) {
                        /* select bin 1 to overwrite it */                
                            binNum = 1;
                    }
                } else {
                    /* check mac */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                        /* select bin 1 to overwrite it */               
                            binNum = 1;
                    }                
                } 
            }
        
            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                /* can't find a entry to overwrite based on same mac + vid */
                if (binNum == -1) {
                    if (temp_valid1 == 0) {
                        binNum = 0;
                    } else if (temp_valid2 == 0) {
                        binNum = 1;
                    } else {
                        /* both valid, pick non-static one */
                        if (temp_static1 == 0) {
                            binNum = 0;
                        } else if (temp_static2 == 0) {
                            binNum = 1;
                        } else {
                            /* table full */
                            rv = SOC_E_FULL;
                            goto mem_search_exit;                    
                        }
                    }
                }
            }
        /* Not DRV_MEM_OP_REPLACE */
        } else {
            /* Check the ARL Entry 0 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRY_0r, (uint32 *)&entry0, ARL_VALIDf, &temp);
            if (temp) {
                if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                    is_conflict[0] = TRUE;
                }
                
                /* this entry if valid, check to see if this is the MAC */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_ENTRY_0r, (uint32 *)&entry0, ARL_MACADDRf,
                    (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_VID_ENTRY_0r, 0, 0), &vid1, 2);
                if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                    if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                        if (vid1 == vid_rw) {
                            binNum = 0;
                            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                                existed = 1;
                            }
                        }
                    } else {
                        binNum = 0;
                        if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                            existed = 1;
                        }
                    }
                }
            } else {
                binNum = 0;
            }

            /* Check the ARL Entry 1 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRY_1r, (uint32 *)&entry1, ARL_VALIDf, &temp);
            if (temp) {
                if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                    is_conflict[1] = TRUE;
                }

                /* this entry if valid, check to see if this is the MAC */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_ENTRY_1r, (uint32 *)&entry1, ARL_MACADDRf,
                    (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_VID_ENTRY_1r, 0, 0), &vid2, 2);
                if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                    if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                        if (vid2 == vid_rw) {
                            binNum = 1;
                            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                                existed = 1;
                            }
                        }
                    } else {
                        binNum = 1;
                        if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                            existed = 1;
                        }
                    }
                }
            } else {
                if (binNum == -1) binNum = 1;
            }
            
            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                /* if no entry found, fail */
                if (binNum == -1) {
                    rv = SOC_E_FULL;
                    goto mem_search_exit;
                }
            }
        }
        
        for (i = 0; i < ROBO_53242_L2_BUCKET_SIZE; i++){
            if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                if (is_conflict[i] == FALSE){
                    continue;
                }

                existed = 1;
                rep_entry = (l2_arl_sw_entry_t *)entry + i;
            } else {
                /* match basis search */
                if (i != binNum){
                    continue;
                }

                rep_entry = (l2_arl_sw_entry_t *)entry;
            }

            /* assign the processing hw arl entry */
            if (i == 0) {
                input = &entry0;
                vid_rw = vid1;
            } else {
                input = &entry1;
                vid_rw = vid2;
            }
            *index = i;

            /* Only need write the selected Entry Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRY_0r, (uint32 *)input, 
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            if (temp_mac_addr[0] & 0x01) { /* The input is the mcast address */
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                        (uint32 *)rep_entry, (uint32 *)&temp_mac_field);
                value = 0;
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY_0r, (uint32 *)input, 
                        ARL_PIDf, &temp);
                value = temp;
                
                /* Port number should add 24 for BCM53242 */
                value += 24;

                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY_0r, (uint32 *)input, 
                        ARL_ENTRY_RSRV0f, &temp);
                value |= (temp << 6);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY_0r, (uint32 *)input, 
                        ARL_AGEf, &temp);
                value |= (temp << 9);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
                        (uint32 *)rep_entry, &value);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)rep_entry, &vid_rw);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY_0r, (uint32 *)input, 
                        ARL_CONf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                        (uint32 *)rep_entry, &temp);
                temp = 1;
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                        (uint32 *)rep_entry, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                        (uint32 *)rep_entry, &temp);
            } else { /* The input is the unicast address */
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                        (uint32 *)rep_entry, (uint32 *)&temp_mac_field);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY_1r, (uint32 *)input, 
                        ARL_PIDf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        (uint32 *)rep_entry, &temp);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY_1r, (uint32 *)input, 
                        ARL_AGEf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                        (uint32 *)rep_entry, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)rep_entry, &vid_rw);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY_1r, (uint32 *)input, 
                        ARL_STATICf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                        (uint32 *)rep_entry, &temp);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY_1r, (uint32 *)input, 
                        ARL_CONf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                        (uint32 *)rep_entry, &temp);
                temp = 1;
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                        (uint32 *)rep_entry, &temp);
            }
            
        }

        if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
            if (existed){
                rv = SOC_E_EXISTS;
            } else {
                rv = SOC_E_NOT_FOUND;
            }
        } else {
            if (flags & DRV_MEM_OP_REPLACE) {
                rv = SOC_E_NONE;
            } else {
                 if(existed)
                    rv = SOC_E_EXISTS;    
                 else
                    rv = SOC_E_NOT_FOUND;
            }
        }

mem_search_exit:
        ARL_MEM_SEARCH_UNLOCK(soc);
        return rv;

    } else {
        return SOC_E_PARAM;
    }

}

int 
_drv_bcm53242_mem_arl_entry_delete(int unit, uint8 *mac_addr, uint32 vid, int index)
{
    uint32 reg_addr, reg_len, reg_val32, count, temp;
    uint64 reg_val64, mac_field;
    int rv = SOC_E_NONE;
    soc_control_t           *soc = SOC_CONTROL(unit);

    /* write MAC Addr */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_MACr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_MACr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_val64, reg_len)) < 0) {
        goto marl_entry_delete_exit;
    }

    SAL_MAC_ADDR_TO_UINT64(mac_addr, mac_field);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_MACr, (uint32 *)&reg_val64, MAC_ADDR_INDXf, (uint32 *)&mac_field));

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_val64, reg_len)) < 0) {
        goto marl_entry_delete_exit;
    }

    /* write VID */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_VIDr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_VIDr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_val32, reg_len)) < 0) {
        goto marl_entry_delete_exit;
    }
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_VIDr, &reg_val32, ARLA_VIDTAB_INDXf, &vid));

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_val32, reg_len)) < 0) {
        goto marl_entry_delete_exit;
    }

    if (index == 0) {
        /* entry 0 */
        /* Clear entire entry */
        COMPILER_64_ZERO(reg_val64);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_ENTRY_0r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_ENTRY_0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_val64, reg_len)) < 0) {
            goto marl_entry_delete_exit;
        }

        /* Clear VID entry */
        reg_val32 = 0;
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_VID_ENTRY_0r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_VID_ENTRY_0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_val32, reg_len)) < 0) {
            goto marl_entry_delete_exit;
        }
    } else {
        /* entry 1 */
        /* Clear entire entry */
        COMPILER_64_ZERO(reg_val64);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_ENTRY_1r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_ENTRY_1r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_val64, reg_len)) < 0) {
            goto marl_entry_delete_exit;
        }

        /* Clear VID entry */
        reg_val32 = 0;
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_VID_ENTRY_1r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_VID_ENTRY_1r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_val32, reg_len)) < 0) {
            goto marl_entry_delete_exit;
        }
    }

    MEM_RWCTRL_REG_LOCK(soc);
    /* Write ARL Read/Write Control Register */
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_RWCTLr, 0, 0), &reg_val32, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto marl_entry_delete_exit;
    }
    temp = MEM_TABLE_WRITE;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, TAB_RWf, &temp);
    temp = BCM53242_ARL_TABLE_ACCESS;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, TAB_INDEXf, (uint32 *) &temp);
    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, ARL_STRTDNf, &temp);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_RWCTLr, 0, 0), &reg_val32, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto marl_entry_delete_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &reg_val32, 1)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto marl_entry_delete_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_RWCTLr, &reg_val32, ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto marl_entry_delete_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);

marl_entry_delete_exit:
    return rv;
}   

/*
 *  Function : _drv_bcm53242_mem_table__reset
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53242_mem_table_reset(int unit, uint32 mem)
{
    int rv = SOC_E_NONE;
    uint32 retry;
    uint32  reg_len, reg_addr, temp, reg_value;
    uint32 fld_index, reg_index;
    soc_control_t           *soc = SOC_CONTROL(unit);
    
    switch(mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
            reg_index = RST_TABLE_MEMr;
            fld_index = RST_ARLf;
            break;
        case DRV_MEM_MCAST:
            reg_index = RST_TABLE_MEMr;
            fld_index = RST_IPMCf;
            break;
        case DRV_MEM_VLAN:
            reg_index = RST_TABLE_MEMr;
            fld_index = RST_VTf;
            break;
        case DRV_MEM_MSTP:
            reg_index = RST_TABLE_MEMr;
            fld_index = RST_MSPTf;
            break;
        case DRV_MEM_VLANVLAN:
            reg_index = RST_TABLE_MEM1r;
            fld_index = RST_VLAN2VLANf;
            break;
        case DRV_MEM_MACVLAN:
            reg_index = RST_TABLE_MEM1r;
            fld_index = RST_MAC2VLANf;
            break;
        case DRV_MEM_PROTOCOLVLAN:
            reg_index = RST_TABLE_MEM1r;
            fld_index = RST_PROTOCOL2VLANf;
            break;
        case DRV_MEM_FLOWVLAN:
            reg_index = RST_TABLE_MEM1r;
            fld_index = RST_FLOW2VLANf;
            break;
        case DRV_MEM_MARL:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }

    MEM_RWCTRL_REG_LOCK(soc);    
    /* read control setting */                
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, reg_index);

    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_table_reset_exit;
    }
    
    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, reg_index, &reg_value, fld_index, &temp);
    
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_table_reset_exit;
    }
    
    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_table_reset_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, &reg_value, fld_index, &temp);
        if (!temp) {
            break;
        }
    }

    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_table_reset_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);

mem_table_reset_exit:   
    return rv;
}

int 
_drv_bcm53242_mem_marl_delete_all(int unit)
{
    int rv = SOC_E_NONE;
    soc_control_t   *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t output;
    l2_arl_sw_entry_t entry0, entry1;
    uint64 temp_mac_field;
    uint8  temp_mac_addr[6];
    int index_min, index_max, index_count;
    int32 idx, temp_vid;
    uint32 valid;
    int index;

    index_min = SOC_MEM_BASE(unit, L2_ARLm);
    index_max = SOC_MEM_BASE(unit, L2_ARLm) + SOC_MEM_SIZE(unit, L2_ARLm);
    index_count = SOC_MEM_SIZE(unit, L2_ARLm);
    if(soc->arl_table != NULL){
        ARL_SW_TABLE_LOCK(soc);
        for (idx = index_min; idx < index_count; idx++) {
            sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
            if(!ARL_ENTRY_NULL(&soc->arl_table[idx])) {
                sal_memcpy(&output, &soc->arl_table[idx], 
                    sizeof(l2_arl_sw_entry_t));
            } else {
                continue;
            }
            (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_VALID, 
                    (uint32 *)&output, &valid);
            if (valid){
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&output, (uint32 *)&temp_mac_field);
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_VLANID, 
                    (uint32 *)&output, (uint32 *)&temp_vid);

                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                if (temp_mac_addr[0] & 0x01) { /* mcast address */
                    /* delete HW entry */
                    if ((rv =  _drv_bcm53242_mem_search
                        (unit, DRV_MEM_ARL, (uint32 *)&output, 
                        (uint32 *)&entry0, (uint32 *)&entry1, 
                        DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID, 
                        &index))< 0) {
                        if((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
                            if (rv == SOC_E_FULL) {
                            /* For mem_delete if mem_search return SOC_E_FULL it means
                              * the entry is not found. */
                                rv = SOC_E_NOT_FOUND;
                            }
                        }
                    }
                    if (rv == SOC_E_EXISTS) {
                        rv = _drv_bcm53242_mem_arl_entry_delete(unit, 
                            temp_mac_addr, temp_vid, index);
                        if (rv < 0) {
                            ARL_SW_TABLE_UNLOCK(soc);
                            return rv;
                        }

                        /* Remove the entry from sw database */
                        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
                        sal_memcpy(&soc->arl_table[idx], &output,
                            sizeof(l2_arl_sw_entry_t));
                    }
                }
           } 
        }
        ARL_SW_TABLE_UNLOCK(soc);
    } else {
        soc_cm_debug(DK_ERR,"soc arl table not allocated");
        rv = SOC_E_FAIL;
    }
    return rv;
}

int
_drv_bcm53242_cfp_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    drv_cfp_entry_t cfp_entry;
    int entry_len, rv = SOC_E_NONE;
    uint8 *data_ptr;
    uint32 mem_id, counter;
    uint32 i, index_min, index_max;
    soc_mem_info_t *meminfo;

    data_ptr = (uint8 *)entry;
    /* Get the length of entry */
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
            mem_id = CFP_TCAM_S0m;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_ACT:
            mem_id = CFP_ACT_POLm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_METER:
            mem_id = CFP_METERm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            mem_id = CFP_STAT_IBm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        default:
            return SOC_E_PARAM;
    }

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /* check index */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);

    for (i = 0; i < count; i++) {
        if (((entry_id + i) < index_min) || 
            ((entry_id + i) > index_max)) {
            return SOC_E_PARAM;
        }

        switch (mem) {
            case DRV_MEM_TCAM_DATA:
            case DRV_MEM_TCAM_MASK:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_read)(unit, (entry_id + i), 
                    DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                if (mem == DRV_MEM_TCAM_DATA) {
                    sal_memcpy(data_ptr, cfp_entry.tcam_data, entry_len);
                } else {
                    sal_memcpy(data_ptr, cfp_entry.tcam_mask, entry_len);
                }
                break;
            case DRV_MEM_CFP_ACT:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_read)(unit, (entry_id + i), 
                    DRV_CFP_RAM_ACT, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                sal_memcpy(data_ptr, cfp_entry.act_data, entry_len);
                break;
            case DRV_MEM_CFP_METER:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_read)(unit, (entry_id + i), 
                    DRV_CFP_RAM_METER, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                sal_memcpy(data_ptr, cfp_entry.meter_data, entry_len);
                break;
            case DRV_MEM_CFP_STAT_IB:
                sal_memset(&counter, 0, sizeof(uint32));
                if ((rv = (DRV_SERVICES(unit)->cfp_stat_get)
                    (unit, DRV_CFP_STAT_INBAND, 
                    (entry_id + i), &counter)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                sal_memcpy(data_ptr, &counter, entry_len);
                break;
            case DRV_MEM_CFP_STAT_OB:
                sal_memset(&counter, 0, sizeof(uint32));
                if ((rv = (DRV_SERVICES(unit)->cfp_stat_get)
                    (unit, DRV_CFP_STAT_OUTBAND, 
                    (entry_id + i), &counter)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                sal_memcpy(data_ptr, &counter, entry_len);
                break;
            default:
                return SOC_E_PARAM;
        }
        data_ptr = data_ptr + entry_len;
    }

    return rv;
}

int
_drv_bcm53242_cfp_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    drv_cfp_entry_t cfp_entry;
    int entry_len, rv = SOC_E_NONE;
    uint8 *data_ptr;
    uint32 mem_id, counter;
    uint32 i, index_min, index_max;
    soc_mem_info_t *meminfo;

    data_ptr = (uint8 *)entry;
    /* Get the length of entry */
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
            mem_id = CFP_TCAM_S0m;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_ACT:
            mem_id = CFP_ACT_POLm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_METER:
            mem_id = CFP_METERm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            mem_id = CFP_STAT_IBm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        default:
            return SOC_E_PARAM;
    }

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /* check index */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);

    for (i = 0; i < count; i++) {
        if (((entry_id + i) < index_min) || 
            ((entry_id + i) > index_max)) {
            return SOC_E_PARAM;
        }

        switch (mem) {
            case DRV_MEM_TCAM_DATA:
            case DRV_MEM_TCAM_MASK:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_read)(unit, (entry_id + i), 
                    DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                if (mem == DRV_MEM_TCAM_DATA) {
                    sal_memcpy(cfp_entry.tcam_data, data_ptr, entry_len);
                } else {
                    sal_memcpy(cfp_entry.tcam_mask, data_ptr, entry_len);
                }
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_write)(unit, (entry_id + i), 
                    DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_write(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                break;
            case DRV_MEM_CFP_ACT:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                sal_memcpy(cfp_entry.act_data, data_ptr, entry_len);
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_write)(unit, (entry_id + i), 
                    DRV_CFP_RAM_ACT, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_write(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                break;
            case DRV_MEM_CFP_METER:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                sal_memcpy(cfp_entry.meter_data, data_ptr, entry_len);
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_write)(unit, (entry_id + i), 
                    DRV_CFP_RAM_METER, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_write(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                break;
            case DRV_MEM_CFP_STAT_IB:
                sal_memset(&counter, 0, sizeof(uint32));
                sal_memcpy(&counter, data_ptr, entry_len);
                if ((rv = (DRV_SERVICES(unit)->cfp_stat_set)
                    (unit, DRV_CFP_STAT_INBAND, 
                    (entry_id + i), counter)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                break;
            case DRV_MEM_CFP_STAT_OB:
                sal_memset(&counter, 0, sizeof(uint32));
                sal_memcpy(&counter, data_ptr, entry_len);
                if ((rv = (DRV_SERVICES(unit)->cfp_stat_set)
                    (unit, DRV_CFP_STAT_OUTBAND, 
                    (entry_id + i), counter)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm53242_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                break;
            default:
                return SOC_E_PARAM;
        }
        data_ptr = data_ptr + entry_len;
    }

    return rv;
}

int
_drv_bcm53242_cfp_field_get(int unit, uint32 mem, uint32 field_index, 
        uint32 *entry, uint32 *fld_data)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t cfp_entry;
    int entry_len;
    soc_mem_info_t *meminfo;
    
    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
            meminfo = &SOC_MEM_INFO(unit, CFP_TCAM_S0m);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_TCAM, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm53242_cfp_field_get(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            break;
        case DRV_MEM_TCAM_MASK:
            meminfo = &SOC_MEM_INFO(unit, CFP_TCAM_MASKm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_mask, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_TCAM_MASK, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm53242_cfp_field_get(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            break;
        case DRV_MEM_CFP_ACT:
            meminfo = &SOC_MEM_INFO(unit, CFP_ACT_POLm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.act_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_ACT, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm53242_cfp_field_get(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            break;
        case DRV_MEM_CFP_METER:
            meminfo = &SOC_MEM_INFO(unit, CFP_METERm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.meter_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_METER, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm53242_cfp_field_get(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }

    return rv;
}


int
_drv_bcm53242_cfp_field_set(int unit, uint32 mem, uint32 field_index, 
        uint32 *entry, uint32 *fld_data)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t cfp_entry;
    int entry_len;
    soc_mem_info_t *meminfo;

    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
            meminfo = &SOC_MEM_INFO(unit, CFP_TCAM_S0m);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_set)
                (unit, DRV_CFP_RAM_TCAM, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm53242_cfp_field_set(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            sal_memcpy(entry, cfp_entry.tcam_data, entry_len);
            break;
        case DRV_MEM_TCAM_MASK:
            meminfo = &SOC_MEM_INFO(unit, CFP_TCAM_MASKm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_mask, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_set)
                (unit, DRV_CFP_RAM_TCAM_MASK, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm53242_cfp_field_set(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            sal_memcpy(entry, cfp_entry.tcam_mask, entry_len);
            break;
        case DRV_MEM_CFP_ACT:
            meminfo = &SOC_MEM_INFO(unit, CFP_ACT_POLm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.act_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_set)
                (unit, DRV_CFP_RAM_ACT, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm53242_cfp_field_set(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            sal_memcpy(entry, cfp_entry.act_data, entry_len);
            break;
        case DRV_MEM_CFP_METER:
            meminfo = &SOC_MEM_INFO(unit, CFP_METERm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.meter_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_set)
                (unit, DRV_CFP_RAM_METER, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm53242_cfp_field_set(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            sal_memcpy(entry, cfp_entry.meter_data, entry_len);
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }

    return rv;
}

/*
 *  Function : drv_bcm53242_mem_length_get
 *
 *  Purpose :
 *      Get the number of entries of the selected memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      data   :   total number entries of this memory type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm53242_mem_length_get(int unit, uint32 mem, uint32 *data)
{

    soc_mem_info_t *meminfo;
    
    switch (mem)
    {
        case DRV_MEM_ARL:
            meminfo = &SOC_MEM_INFO(unit, L2_ARLm);
            break;
        case DRV_MEM_MARL:
            meminfo = &SOC_MEM_INFO(unit, L2_MARL_SWm);
            break;            
        case DRV_MEM_VLAN:
            meminfo = &SOC_MEM_INFO(unit, VLAN_1Qm);
            break;
        case DRV_MEM_MSTP:
            meminfo = &SOC_MEM_INFO(unit, MSPT_TABm);
            break;
        case DRV_MEM_MCAST:
            meminfo = &SOC_MEM_INFO(unit, MARL_PBMPm);
            break;
        case DRV_MEM_GEN:
            meminfo = &SOC_MEM_INFO(unit, GEN_MEMORYm);
            break;
        case DRV_MEM_VLANVLAN:
            meminfo = &SOC_MEM_INFO(unit, VLAN2VLANm);
            break;
        case DRV_MEM_MACVLAN:
            meminfo = &SOC_MEM_INFO(unit, MAC2VLANm);
            break;
        case DRV_MEM_PROTOCOLVLAN:
            meminfo = &SOC_MEM_INFO(unit, PROTOCOL2VLANm);
            break;
        case DRV_MEM_FLOWVLAN:
            meminfo = &SOC_MEM_INFO(unit, FLOW2VLANm);
            break;
        default:
            return SOC_E_PARAM;
    }

  *data = meminfo->index_max - meminfo->index_min + 1;

  return SOC_E_NONE;
}


/*
 *  Function : drv_bcm53242_mem_width_get
 *
 *  Purpose :
 *      Get the width of selected memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      data   :   total number bits of entry.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm53242_mem_width_get(int unit, uint32 mem, uint32 *data)
{

    switch (mem)
    {
        case DRV_MEM_ARL:
            *data = sizeof(l2_arl_entry_t);
            break;
         case DRV_MEM_MARL:
            *data = sizeof(l2_marl_sw_entry_t);
            break;
        case DRV_MEM_VLAN:
            *data = sizeof(vlan_1q_entry_t);
            break;
        case DRV_MEM_MSTP:
            *data = sizeof(mspt_tab_entry_t);
            break;
        case DRV_MEM_MCAST:
            *data = sizeof(marl_pbmp_entry_t);
            break;
        case DRV_MEM_GEN:
            *data = sizeof(gen_memory_entry_t);
            break;
        case DRV_MEM_VLANVLAN:
            *data = sizeof(vlan2vlan_entry_t);
            break;
        case DRV_MEM_MACVLAN:
            *data = sizeof(mac2vlan_entry_t);
            break;
        case DRV_MEM_PROTOCOLVLAN:
            *data = sizeof(protocol2vlan_entry_t);
            break;
        case DRV_MEM_FLOWVLAN:
            *data = sizeof(flow2vlan_entry_t);
            break;
        default:
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

int
_drv_bcm53242_arl_read(int unit, uint32 entry_id, 
                               uint32 count, uint32 *entry)
{
    
    int rv = SOC_E_NONE;
    int i;
    uint32 retry, index_min, index_max, index;
    uint32 mem_id;
    uint32 acc_ctrl = 0, mem_addr = 0;
    uint32  temp;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint32 reg_addr;
    int reg_len;

    soc_cm_debug(DK_MEM, 
        "_drv_bcm53242_arl_read(entry_id=0x%x,count=%d)\n",
         entry_id, count);

    mem_id = L2_ARLm;

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    
    if (((entry_id) < index_min) || 
        ((entry_id + count - 1) > index_max)) {
            return SOC_E_PARAM;
    }

    /* process read action */
    MEM_LOCK(unit, GEN_MEMORYm);

    /* check count */
    if (count < 1) {
        MEM_UNLOCK(unit, GEN_MEMORYm);
        return SOC_E_PARAM;
    }

    for (i = 0;i < count; i++ ) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, GEN_MEMORYm);
            return SOC_E_PARAM;
        }

        /* Return data from cache if active */

        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL && CACHE_VMAP_TST(vmap, (entry_id + i))) {
            sal_memcpy(gmem_entry, 
                cache + (entry_id + i) * entry_size, entry_size);
            continue;
        }

        index = entry_id + i;
        temp = index / 2;
        /* Set memory entry address */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, GENMEM_ADDRr, 0, 0);
        reg_len = DRV_SERVICES(unit)->reg_length_get(unit,GENMEM_ADDRr);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_ADDRr, &mem_addr, GENMEM_ADDRf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &mem_addr, reg_len)) < 0) {
            goto arl_read_exit;
        }

        /* Read memory control register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, GENMEM_CTLr, 0, 0);
        reg_len = DRV_SERVICES(unit)->reg_length_get(unit,GENMEM_CTLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            goto arl_read_exit;
        }
        temp = BCM53242_MEM_OP_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_CTLr, &acc_ctrl, GENMEM_RWf, &temp);
        temp = BCM53242_ARL_MEMORY;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_CTLr, &acc_ctrl, TXDSC_ARLf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_CTLr, &acc_ctrl, GENMEM_STDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            goto arl_read_exit;}

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
                goto arl_read_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, GENMEM_CTLr, &acc_ctrl, GENMEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto arl_read_exit;
        }

        /* Read the current generic memory entry */
        if ((index % 2) == 0) {
            /* Read bin 0 entry */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                    (unit, GENMEM_DATA0r, 0, 0);
            reg_len = DRV_SERVICES(unit)->reg_length_get(unit,GENMEM_DATA0r);

            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, gmem_entry, reg_len)) < 0) {
                goto arl_read_exit;
            }
            gmem_entry++;
            /* Read bin 1 entry */
            if (++i < count) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                    (unit, GENMEM_DATA1r, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit,GENMEM_DATA1r);

                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, gmem_entry, reg_len)) < 0) {
                    goto arl_read_exit;
                }
                gmem_entry++;    
            }
        } else {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                    (unit, GENMEM_DATA1r, 0, 0);
            reg_len = DRV_SERVICES(unit)->reg_length_get(unit,GENMEM_DATA1r);

            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, gmem_entry, reg_len)) < 0) {
                goto arl_read_exit;
            }
            gmem_entry++;  
        }
        

    }

 arl_read_exit:
    MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;

}

int
_drv_bcm53242_arl_write(int unit, uint32 entry_id, 
                               uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max;
    uint32 i;
    uint32 mem_id;
    uint32 acc_ctrl = 0, mem_addr = 0;
    uint32  temp, index;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint32 reg_addr;
    int reg_len;

    soc_cm_debug(DK_MEM, "drv_mem_write(entry_id=0x%x,count=%d)\n",
         entry_id, count);
         
    mem_id = L2_ARLm;

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);
    
    if (count < 1) {
        return SOC_E_PARAM;
    }
    if (((entry_id) < index_min) || 
        ((entry_id + count - 1) > index_max)) {
            return SOC_E_PARAM;
    }

    /* process write action */
    MEM_LOCK(unit, GEN_MEMORYm);
    
    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, GEN_MEMORYm);
            return SOC_E_PARAM;
        }

        /* write data */
        index = entry_id + i;
        if ((index % 2) == 0) {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, GENMEM_DATA0r, 0, 0);
            reg_len = DRV_SERVICES(unit)->reg_length_get(unit,GENMEM_DATA0r);

            if ((rv =(DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, gmem_entry, reg_len)) < 0) {
                goto arl_write_exit;
            }
            if (++i < count) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, GENMEM_DATA1r, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit,GENMEM_DATA1r);

                if ((rv =(DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, ++gmem_entry, reg_len)) < 0) {
                    goto arl_write_exit;
                }
            }
        } else {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, GENMEM_DATA1r, 0, 0);
            reg_len = DRV_SERVICES(unit)->reg_length_get(unit,GENMEM_DATA1r);

            if ((rv =(DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, gmem_entry, reg_len)) < 0) {
                goto arl_write_exit;
            }
        }

        /* Set memory entry address */
        temp = index / 2;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, GENMEM_ADDRr, 0, 0);
        reg_len = DRV_SERVICES(unit)->reg_length_get(unit,GENMEM_ADDRr);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_ADDRr, &mem_addr, GENMEM_ADDRf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &mem_addr, reg_len)) < 0) {
            goto arl_write_exit;
        }

        /* Read memory control register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, GENMEM_CTLr, 0, 0);
        reg_len = DRV_SERVICES(unit)->reg_length_get(unit,GENMEM_CTLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            goto arl_write_exit;
        }
        temp = BCM53242_MEM_OP_WRITE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_CTLr, &acc_ctrl, GENMEM_RWf, &temp);
        temp = BCM53242_ARL_MEMORY;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_CTLr, &acc_ctrl, TXDSC_ARLf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_CTLr, &acc_ctrl, GENMEM_STDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            goto arl_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
                goto arl_write_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, GENMEM_CTLr, &acc_ctrl, GENMEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto arl_write_exit;
        }
        
        /* Write back to cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL) {
            if (((index % 2) == 0) && (i < count)) {
                sal_memcpy(cache + (index) * entry_size, 
                    --gmem_entry, entry_size * 2);
                CACHE_VMAP_SET(vmap, (index));
                CACHE_VMAP_SET(vmap, (index+1));
                gmem_entry++;
            } else {
                sal_memcpy(cache + (index) * entry_size, 
                    gmem_entry, entry_size);
                CACHE_VMAP_SET(vmap, (index));
            }
        }
        gmem_entry++;
    }

arl_write_exit:
    MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;
    
}
 /*
 *  Function : drv_bcm53242_mem_read
 *
 *  Purpose :
 *      Get the width of selected memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the read result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
drv_bcm53242_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i;
    uint32 retry, index_min, index_max;
    uint32 mem_id;
    uint32 acc_ctrl = 0;
    uint32 other_table_idx = 0;
    uint32  temp;

    /* bcm53242 uses 3 data register to read/write memories except 
     *  the ARL and CFP tables 
     *   - ARL table read are processed in other sub-routine.
     *   - CFP table read are not designed in this routine.
     *   - the gmem_entry access pointer is not referenced or used when the 
     *      target table is ARL or any one CFP related table.
     */
    uint64 temp_data[3];
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)temp_data;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint32 reg_addr;
    int reg_len;
    uint8 *mem_ptr;
    uint64 zero_value;
    soc_control_t           *soc = SOC_CONTROL(unit);
#ifdef BE_HOST
    uint32 val32;
    uint32  *temp_entry;
#endif

    soc_cm_debug(DK_MEM, 
        "drv_mem_read(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
             rv = _drv_bcm53242_arl_read(unit, entry_id, count, entry);
             return rv;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            break;
        case DRV_MEM_MSTP:
            mem_id = MSPT_TABm;
            break;
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            break;
        case DRV_MEM_VLANVLAN:
            mem_id = VLAN2VLANm;
            break;
        case DRV_MEM_MACVLAN:
            mem_id = MAC2VLANm;
            break;
        case DRV_MEM_PROTOCOLVLAN:
            mem_id = PROTOCOL2VLANm;
            break;
        case DRV_MEM_FLOWVLAN:
            mem_id = FLOW2VLANm;
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            rv = _drv_bcm53242_cfp_read(
                unit, mem, entry_id, count, entry);
            return rv;
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    /* process read action */
    MEM_LOCK(unit, GEN_MEMORYm);

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
        goto mem_read_exit;
    }

    for (i = 0;i < count; i++ ) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, GEN_MEMORYm);
            return SOC_E_PARAM;
        }

        /* Return data from cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL && CACHE_VMAP_TST(vmap, (entry_id + i))) {
            sal_memcpy(gmem_entry, cache + (entry_id + i) * entry_size, entry_size);
            continue;
        }

        /* Clear memory data access registers */
        COMPILER_64_ZERO(zero_value);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA0r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA0r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_read_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA1r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA1r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_read_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA2r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA2r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        /* Set memory index */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_INDEXr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_INDEXr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &other_table_idx, reg_len)) < 0) {
            goto mem_read_exit;
        }
        temp = entry_id + i;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, OTHER_TABLE_INDEXr, &other_table_idx, TABLE_INDEXf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &other_table_idx, reg_len)) < 0) {
            goto mem_read_exit;
        }

        MEM_RWCTRL_REG_LOCK(soc);
        /* Read memory control register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_RWCTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_RWCTLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_read_exit;
        }
        
        temp = MEM_TABLE_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, TAB_RWf, &temp);

        /* Decide which table to be read */
        switch(mem_id) {
            case VLAN_1Qm:
                temp = BCM53242_VLAN_TABLE_ACCESS;
                break;
            case MSPT_TABm:
                temp = BCM53242_MSPT_TABLE_ACCESS;
                break;
            case MARL_PBMPm:
                temp = BCM53242_MARL_PBMP_TABLE_ACCESS;
                break;
            case VLAN2VLANm:
                temp = BCM53242_VLAN2VLAN_TABLE_ACCESS;
                break;
            case MAC2VLANm:
                temp = BCM53242_MAC2VLAN_TABLE_ACCESS;
                break;
            case PROTOCOL2VLANm:
                temp = BCM53242_PROTOCOL2VLAN_TABLE_ACCESS;
                break;
            case FLOW2VLANm:
                temp = BCM53242_FLOW2VLAN_TABLE_ACCESS;
                break;
            default:
                rv = SOC_E_UNAVAIL;
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_read_exit;
        }
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, TAB_INDEXf, &temp);

        /* Start Read Process */
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, ARL_STRTDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_read_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_RWCTLr, &acc_ctrl, ARL_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_read_exit;
        }
        MEM_RWCTRL_REG_UNLOCK(soc);

        /* Read the current generic memory entry */
        mem_ptr = (uint8 *)gmem_entry;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA0r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA0r);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, mem_ptr, reg_len)) < 0) {
            goto mem_read_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA1r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA1r);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, mem_ptr+8, reg_len)) < 0) {
            goto mem_read_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA2r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA2r);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, mem_ptr+16, reg_len)) < 0) {
            goto mem_read_exit;
        }
#ifdef BE_HOST
    temp_entry = (uint32 *)temp_data;
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = temp_entry[0];
        temp_entry[0] = temp_entry [2];
        temp_entry [2] = val32;
    } else if (mem_id == VLAN_1Qm || mem_id == MARL_PBMPm){
        for (i = 0; i < 3; i++) {
            val32 = temp_entry[i];
            temp_entry[i] = temp_entry[5-i];
            temp_entry[5-i] = val32;
        }
        
        for (i = 0; i < 3; i++) {
            val32 = temp_entry[2*i];
            temp_entry[2*i] = temp_entry[2*i+1];
            temp_entry[2*i+1] = val32;
        }

    } else if (mem_id == VLAN2VLANm || mem_id == PROTOCOL2VLANm || 
                mem_id == FLOW2VLANm || mem_id == MAC2VLANm) {
        val32 = temp_entry[0];
        temp_entry[0] = temp_entry [1];
        temp_entry [1] = val32;
    }
    sal_memcpy(entry, temp_entry, entry_size);
#else
    sal_memcpy(entry, gmem_entry, entry_size);
#endif
        gmem_entry++;
    }

 mem_read_exit:
    MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;

}

 /*
 *  Function : drv_bcm53242_mem_write
 *
 *  Purpose :
 *      Writes an internal memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the entry's index of the memory to be written.
 *      count   :   one or more netries to be written.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the writting result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
drv_bcm53242_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max;
    uint32 i;
    uint32 mem_id;
    uint32 acc_ctrl = 0;
    uint32  temp;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint32 reg_addr;
    int reg_len;
    uint32 other_table_idx = 0;
    uint8 *mem_ptr;
    uint64 zero_value;
    soc_control_t           *soc = SOC_CONTROL(unit);
#ifdef BE_HOST
    uint32              val32;
#endif

    soc_cm_debug(DK_MEM, "drv_mem_write(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
         
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
             rv = _drv_bcm53242_arl_write(unit, entry_id, count, entry);
             return rv;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            break;
        case DRV_MEM_MSTP:
            mem_id = MSPT_TABm;
            break;
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            break;
        case DRV_MEM_VLANVLAN:
            mem_id = VLAN2VLANm;
            break;
        case DRV_MEM_MACVLAN:
            /* call MACVLAN specific routine */
            rv = _drv_bcm53242_mem_macvlan_write(unit, entry, 0);
            return rv;
            break;
        case DRV_MEM_PROTOCOLVLAN:
            mem_id = PROTOCOL2VLANm;
            break;
        case DRV_MEM_FLOWVLAN:
            mem_id = FLOW2VLANm;
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            rv = _drv_bcm53242_cfp_write(
                unit, mem, entry_id, count, entry);
            return rv;
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }


    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);
    
    /* process write action */
    MEM_LOCK(unit, GEN_MEMORYm);

#ifdef BE_HOST
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == VLAN_1Qm || mem_id == MARL_PBMPm){
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }

        for (i = 0; i < 3; i++) {
           val32 = entry[2*i];
           entry[2*i] = entry[2*i+1];
           entry[2*i+1] = val32;
        }
    } else if (mem_id == VLAN2VLANm || mem_id == PROTOCOL2VLANm || mem_id == FLOW2VLANm) {
        val32 = entry[0];
        entry[0] = entry [1];
        entry [1] = val32;
    }
#endif

    if (count < 1) {
        rv = SOC_E_PARAM;
        goto mem_write_exit;
    }

    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, GEN_MEMORYm);
            return SOC_E_PARAM;
        }

        /* Clear memory data access registers */
        COMPILER_64_ZERO(zero_value);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA0r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA0r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_write_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA1r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA1r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_write_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA2r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA2r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_write_exit;
        }

        /* write data */
        mem_ptr = (uint8 *)gmem_entry;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA0r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA0r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, mem_ptr, reg_len)) < 0) {
            goto mem_write_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA1r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA1r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, mem_ptr+8, reg_len)) < 0) {
            goto mem_write_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_DATA2r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_DATA2r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, mem_ptr+16, reg_len)) < 0) {
            goto mem_write_exit;
        }

        /* Set memory index */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, OTHER_TABLE_INDEXr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, OTHER_TABLE_INDEXr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &other_table_idx, reg_len)) < 0) {
            goto mem_write_exit;
        }
        temp = entry_id + i;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, OTHER_TABLE_INDEXr, &other_table_idx, TABLE_INDEXf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &other_table_idx, reg_len)) < 0) {
            goto mem_write_exit;
        }

        MEM_RWCTRL_REG_LOCK(soc);
        /* Read memory control register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_RWCTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_RWCTLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_write_exit;
        }
        
        temp = MEM_TABLE_WRITE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, TAB_RWf, &temp);

        /* Decide which table to be write */
        switch(mem_id) {
            case VLAN_1Qm:
                temp = BCM53242_VLAN_TABLE_ACCESS;
                break;
            case MSPT_TABm:
                temp = BCM53242_MSPT_TABLE_ACCESS;
                break;
            case MARL_PBMPm:
                temp = BCM53242_MARL_PBMP_TABLE_ACCESS;
                break;
            case VLAN2VLANm:
                temp = BCM53242_VLAN2VLAN_TABLE_ACCESS;
                break;
            case MAC2VLANm:
                temp = BCM53242_MAC2VLAN_TABLE_ACCESS;
                break;
            case PROTOCOL2VLANm:
                temp = BCM53242_PROTOCOL2VLAN_TABLE_ACCESS;
                break;
            case FLOW2VLANm:
                temp =BCM53242_FLOW2VLAN_TABLE_ACCESS;
                break;
            default:
                rv = SOC_E_UNAVAIL;
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_write_exit;
        }
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, TAB_INDEXf, &temp);

        /* Start Read Process */
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, ARL_STRTDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_write_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_RWCTLr, &acc_ctrl, ARL_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_write_exit;
        }
        MEM_RWCTRL_REG_UNLOCK(soc);
        
        /* Write back to cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL) {
            sal_memcpy(cache + (entry_id + i) * entry_size, gmem_entry, entry_size);
            CACHE_VMAP_SET(vmap, (entry_id + i));
        }
        gmem_entry++;
    }

#ifdef BE_HOST
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == VLAN_1Qm || mem_id == MARL_PBMPm){
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }

        for (i = 0; i < 3; i++) {
           val32 = entry[2*i];
           entry[2*i] = entry[2*i+1];
           entry[2*i+1] = val32;
        }
    } else if (mem_id == VLAN2VLANm || mem_id == PROTOCOL2VLANm || mem_id == FLOW2VLANm) {
        val32 = entry[0];
        entry[0] = entry [1];
        entry [1] = val32;
    }
#endif

 mem_write_exit:
    MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;
}

/*
 *  Function : drv_bcm53242_mem_field_get
 *
 *  Purpose :
 *      Extract the value of a field from a memory entry value.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *      field_index    :  field type.
 *      entry   :   entry value pointer.
 *      fld_data   :   field value pointer.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      1.For DRV_MEM_MSTP, because there is no port information 
 *          on the parameter list, so it will return all the ports' state.
 *      2. For DRV_MEM_ARL, the entry type will be l2_arl_sw_entry_t 
 *          and the mem_id is L2_ARL_SWm.
 */
int
drv_bcm53242_mem_field_get(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int         mem_id = 0, field_id;
    int         i, wp, bp, len;
#ifdef BE_HOST
    uint32              val32;
#endif

    soc_cm_debug(DK_MEM, "drv_mem_field_get(mem=0x%x,field_index=0x%x)\n",
         mem, field_index);
         
    switch (mem)
    {
        case DRV_MEM_ARL_HW:
            mem_id = L2_ARLm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = MACADDRf;
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = PORTID_Rf;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = RESERVED1_Rf;
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = AGEf;
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = CONTROLf;
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = STATICf;
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            mem_id = L2_ARL_SWm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = MACADDRf;
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = PORTID_Rf;
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = MARL_PBMP_IDXf;
                mem_id = L2_MARL_SWm;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = PRIORITY_Rf;
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = AGEf;
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = STATICf;
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = CONTROLf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
                field_id = MSPT_IDf;
            }else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = UNTAG_MAPf;
            }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = FORWARD_MAPf;
            }else if (field_index == DRV_MEM_FIELD_FWD_MODE) {
                field_id = FWD_MODEf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MSTP:
            mem_id = MSPT_TABm;
            if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
                sal_memcpy(fld_data, entry, sizeof(mspt_tab_entry_t));
                return SOC_E_NONE;
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = PBMP_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLANVLAN:
            mem_id = VLAN2VLANm;
           if (field_index == DRV_MEM_FIELD_MAPPING_MODE) {
                field_id = M_MODEf;
            }else if (field_index == DRV_MEM_FIELD_NEW_VLANID) {
                field_id = NEW_VID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MACVLAN:
            mem_id = MAC2VLANm;
            if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = PRI_Rf;
            }else if ((field_index == DRV_MEM_FIELD_NEW_VLANID)||
                        (field_index == DRV_MEM_FIELD_VLANID)) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = MACADDRf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_PROTOCOLVLAN:
            mem_id = PROTOCOL2VLANm;
            if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = PRI_Rf;
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_ETHER_TYPE) {
                field_id = ETHER_TYPEf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_FLOWVLAN:
            mem_id = FLOW2VLANm;
            if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
            return _drv_bcm53242_cfp_field_get
                (unit, mem, field_index, entry, fld_data);
            break;
        case DRV_MEM_SECMAC:
        case DRV_MEM_GEN:
        default:
            return SOC_E_PARAM;
    }    
    
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);

    assert(entry);
    assert(fld_data);

    SOC_FIND_FIELD(field_id,
         meminfo->fields,
         meminfo->nFields,
         fieldinfo);
    assert(fieldinfo);
    bp = fieldinfo->bp;
#ifdef BE_HOST
    if (mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == VLAN_1Qm || mem_id == MSPT_TABm || mem_id == MARL_PBMPm){
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }
    } else if (mem_id == MAC2VLANm || mem_id == L2_ARLm) {
        val32 = entry[0];
        entry[0] = entry [1];
        entry [1] = val32;
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif


    if (fieldinfo->len < 32) {
        mask = (1 << fieldinfo->len) - 1;
    } else {
        mask = -1;
    }
    
    wp = bp / 32;
    bp = bp & (32 - 1);
    len = fieldinfo->len;

    /* field is 1-bit wide */
    if (len == 1) {
        fld_data[0] = ((entry[FIX_MEM_ORDER_E(wp, meminfo)] >> bp) & 1);
    } else {

    if (fieldinfo->flags & SOCF_LE) {
        for (i = 0; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len > 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;
            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }
            /* get field value --- 32 bits each time */
            fld_data[i] = (entry[FIX_MEM_ORDER_E(wp++, meminfo)] 
                & mask_lo) >> bp;
            if (mask_hi) {
                fld_data[i] |= (entry[FIX_MEM_ORDER_E(wp, meminfo)] 
                    & mask_hi) << (32 - bp);
            }
            i++;
        }
    } else {
        i = (len - 1) / 32;

        while (len > 0) {
            assert(i >= 0);
            fld_data[i] = 0;
            do {
                fld_data[i] = (fld_data[i] << 1) |
                ((entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] >>
                (bp & (32 - 1))) & 1);
                len--;
                bp++;
            } while (len & (32 - 1));
            i--;
        }
    }
    }
#ifdef BE_HOST
    if (mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == VLAN_1Qm || mem_id == MSPT_TABm || mem_id == MARL_PBMPm){
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }
    } else if (mem_id == MAC2VLANm || mem_id == L2_ARLm) {
        val32 = entry[0];
        entry[0] = entry [1];
        entry [1] = val32;
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    if (field_id == PORTID_Rf) {
        *fld_data -= 24;
    }
    return SOC_E_NONE;
}

 /*
 *  Function : drv_bcm53242_mem_field_set
 *
 *  Purpose :
 *      Set the value of a field in a 8-, 16-, 32, and 64-bit memory��s value.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *      field_index    :  field type.
 *      entry   :   entry value pointer.
 *      fld_data   :   field value pointer.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      1.For DRV_MEM_MSTP, because there is no port information 
 *          on the parameter list, so it will set the value on the 
 *          fld_data to memory entry.
 *      2. For DRV_MEM_ARL, the entry type will be l2_arl_sw_entry_t 
 *          and the mem_id is L2_ARL_SWm.
 */
int
drv_bcm53242_mem_field_set(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int         mem_id, field_id;
    int         i, wp, bp, len;
#ifdef BE_HOST
    uint32               val32;
#endif

    soc_cm_debug(DK_MEM, "drv_mem_field_set(mem=0x%x,field_index=0x%x)0x%x\n",
         mem, field_index,*fld_data);
         
    switch (mem)
    {
        case DRV_MEM_ARL_HW:
            mem_id = L2_ARLm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = MACADDRf;
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = PORTID_Rf;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = RESERVED1_Rf;
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = AGEf;
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = CONTROLf;
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = STATICf;
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            mem_id = L2_ARL_SWm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = MACADDRf;
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = PORTID_Rf;
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = MARL_PBMP_IDXf;
                mem_id = L2_MARL_SWm;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = PRIORITY_Rf;
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = AGEf;
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = STATICf;
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = CONTROLf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
                field_id = MSPT_IDf;
            }else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = UNTAG_MAPf;
            }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = FORWARD_MAPf;
            }else if (field_index == DRV_MEM_FIELD_FWD_MODE) {
                field_id = FWD_MODEf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MSTP:
            mem_id = MSPT_TABm;
            if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
                sal_memcpy(entry, fld_data, sizeof(mspt_tab_entry_t));
                return SOC_E_NONE;
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = PBMP_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLANVLAN:
            mem_id = VLAN2VLANm;
            if (field_index == DRV_MEM_FIELD_MAPPING_MODE) {
                field_id = M_MODEf;
            }else if (field_index == DRV_MEM_FIELD_NEW_VLANID) {
                field_id = NEW_VID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MACVLAN:
            mem_id = MAC2VLANm;
            if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = PRI_Rf;
            }else if ((field_index == DRV_MEM_FIELD_NEW_VLANID)||
                        (field_index == DRV_MEM_FIELD_VLANID)) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = MACADDRf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_PROTOCOLVLAN:
            mem_id = PROTOCOL2VLANm;
            if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = PRI_Rf;
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_ETHER_TYPE) {
                field_id = ETHER_TYPEf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_FLOWVLAN:
            mem_id = FLOW2VLANm;
            if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
            return _drv_bcm53242_cfp_field_set
                (unit, mem, field_index, entry, fld_data);
            break;
        case DRV_MEM_SECMAC:
        case DRV_MEM_GEN:
        default:
            return SOC_E_PARAM;
    }    
    
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);

    assert(meminfo);
    assert(entry);
    assert(fld_data);
    SOC_FIND_FIELD(field_id,
         meminfo->fields,
         meminfo->nFields,
         fieldinfo);
    assert(fieldinfo);
#ifdef BE_HOST
    if (mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == VLAN_1Qm || mem_id == MSPT_TABm || mem_id == MARL_PBMPm){
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }
    } else if (mem_id == MAC2VLANm || mem_id == L2_ARLm) {
        val32 = entry[0];
        entry[0] = entry[1];
        entry[1] = val32;
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif

    /* Port number should add 24 for BCM53242 */
    if (field_id == PORTID_Rf) {
        *fld_data += 24;
    }
    bp = fieldinfo->bp;
    if (fieldinfo->flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = fieldinfo->len; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len > 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;

            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }

            /* set field value --- 32 bits each time */
            entry[FIX_MEM_ORDER_E(wp, meminfo)] &= ~mask_lo;
            entry[FIX_MEM_ORDER_E(wp++, meminfo)] |= 
                ((fld_data[i] << bp) & mask_lo);
            if (mask_hi) {
                entry[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask_hi);
                entry[FIX_MEM_ORDER_E(wp, meminfo)] |= 
                    ((fld_data[i] >> (32 - bp)) & mask_hi);
            }

            i++;
        }
    } else {                   
        /* Big endian: swap bits */
        len = fieldinfo->len;

        while (len > 0) {
            len--;
            entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] &= ~(1 << (bp & (32-1)));
            entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] |=
            (fld_data[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }
#ifdef BE_HOST
    if (mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == VLAN_1Qm || mem_id == MSPT_TABm || mem_id == MARL_PBMPm){
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }
    } else if (mem_id == MAC2VLANm || mem_id == L2_ARLm) {
        val32 = entry[0];
        entry[0] = entry[1];
        entry[1] = val32;
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    if (field_id == PORTID_Rf) {
        *fld_data -= 24;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm53242_mem_clear
 *
 *  Purpose :
 *      Clear whole memory entries.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 *
 */
int 
drv_bcm53242_mem_clear(int unit, uint32 mem)
{
    int rv = SOC_E_NONE;
    uint32 count;
    int mem_id;
    uint32 *entry;    
    uint32 del_id;

    soc_cm_debug(DK_MEM, "drv_mem_clear : mem=0x%x\n", mem);
    switch(mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MCAST:
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_VLANVLAN:
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_FLOWVLAN:
            rv = _drv_bcm53242_mem_table_reset(unit, mem);
            return rv;
            break;
        case DRV_MEM_MARL:
            rv = ((DRV_SERVICES(unit)->mem_delete)
                (unit, DRV_MEM_MARL, NULL, DRV_MEM_OP_DELETE_ALL_ARL));
            return SOC_E_NONE;
            break;
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }

    count = soc_robo_mem_index_count(unit, mem_id);

    entry = sal_alloc(sizeof(_soc_mem_entry_null_zeroes),"null_entry");
    if (entry == NULL) {
        soc_cm_print("Insufficient memory.\n");
        return SOC_E_MEMORY;
    }
    sal_memset(entry, 0, sizeof(_soc_mem_entry_null_zeroes));

    for (del_id = 0; del_id < count; del_id++) {
        rv = (DRV_SERVICES(unit)->mem_write)
            (unit, mem, del_id, 1, entry);
        if (rv != SOC_E_NONE){
            soc_cm_debug(DK_WARN, "%s : failed at mem_id=%d, entry=%d!\n", 
                    FUNCTION_NAME(), mem, del_id);
            break;
        }
    }
    sal_free(entry);

    return rv;
}

/*
 *  Function : drv_bcm53242_mem_search
 *
 *  Purpose :
 *      Search selected memory for the key value
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      key   :   the pointer of the data to be search.
 *      entry     :   entry data pointer (if found).
 *      flags     :   search flags.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for ARL memory now.
 *
 */
int 
drv_bcm53242_mem_search(int unit, uint32 mem, 
    uint32 *key, uint32 *entry, uint32 *entry_1, uint32 flags)
{
    int                 rv = SOC_E_NONE;
    int                 mem_id;
    int                 index;

    soc_cm_debug(DK_MEM, "drv_mem_search : mem=0x%x, flags = 0x%x)\n",
         mem, flags);
    switch(mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            mem_id = L2_ARLm;
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_VLANVLAN:
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_FLOWVLAN:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    rv = _drv_bcm53242_mem_search(unit, mem, key, 
        entry, entry_1, flags, &index);

    return rv;
}

#define _DRV_ST_OVERRIDE_NO_CHANGE  0
#define _DRV_ST_OVERRIDE_DYN2ST     1
#define _DRV_ST_OVERRIDE_ST2DYN     2
/*
 *  Function : drv_bcm53242_mem_insert
 *
 *  Purpose :
 *      Insert an entry to specific memory
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      entry     :   entry data pointer.
 *      flags     :   insert flags (no use now).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for ARL memory now.
 *
 */
int 
drv_bcm53242_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int                 rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t       output, output1;
    uint32          temp, count, mem_id;
    uint64          entry_reg;
    uint8           mac_addr[6];
    uint64          mac_field, mac_field_output, mac_field_entry;
    uint32          vid, control, vid_output, vid_entry, st_output, st_entry;
    int             index;
    uint32          value;
    int             is_override = 0, is_dynamic = 0, is_ucast = 0;
    int             ori_st = 0, ori_port = -1, ori_ucast = 0;
    uint32          st_override_status = 0, src_port = 0 ;
    soc_control_t   *soc = SOC_CONTROL(unit);

    soc_cm_debug(DK_MEM, "drv_mem_insert : mem=0x%x, flags = 0x%x)\n",
         mem, flags);
    switch(mem) {
        case DRV_MEM_ARL:
            mem_id = L2_ARL_SWm;
            break;
        case DRV_MEM_MACVLAN:
            mem_id = MAC2VLANm;
            break;
        case DRV_MEM_PROTOCOLVLAN:
            mem_id = PROTOCOL2VLANm;
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_VLANVLAN:
        case DRV_MEM_FLOWVLAN:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    /* section for inster the non-ARL table */
    if (mem != DRV_MEM_ARL){
        
        /* in bcm53242, the non-ARL tables for intersting routine are 
         *  - designed on replace mode when entry conflict.
         *  - Full table insterion is not checked currently
         * 1. MAC2VLAN table 
         * 2. Protocol2VLAN table
         */
        if (entry == NULL){
            return SOC_E_MEMORY;
        }
        
        /* 1. MAC2VLAN table */
        if (mem == DRV_MEM_MACVLAN){
            mac2vlan_entry_t    m2v_output;
            uint32  field_val32;
            uint32     bin_id;
            
            sal_memset(&m2v_output, 0, sizeof(mac2vlan_entry_t));
            if ((rv = _drv_bcm53242_mem_macvlan_search(unit, 
                            entry, &m2v_output, &bin_id))< 0) {
                if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
                    return rv;
                }
            }

            if (((rv == SOC_E_EXISTS) && (flags == DRV_MEM_OP_REPLACE)) || 
                        ((rv == SOC_E_NOT_FOUND) && (bin_id != BCM53242_MACVLAN_UNAVAIL))){
            
                /* set valid bit */
                field_val32 = 1;
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                        entry, &field_val32));
                        
                /* MAC2VLAN write */
                rv = _drv_bcm53242_mem_macvlan_write(unit, entry, 0);
            }
            
        /* 2. PROTOCOL2VLAN */
        } else if (mem == DRV_MEM_PROTOCOLVLAN){
            protocol2vlan_entry_t   prot2v_output;
            uint32  ether_type, ent_id;
            uint32  field_val32;
            
            sal_memset(&prot2v_output, 0, sizeof(protocol2vlan_entry_t));
            if ((rv = _drv_bcm53242_mem_protocolvlan_search(unit, 
                            entry, &prot2v_output, (uint32 *) &index))< 0) {
                if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
                    return rv;
                }
            }

            if (((rv == SOC_E_EXISTS) && (flags == DRV_MEM_OP_REPLACE)) || 
                        (rv == SOC_E_NOT_FOUND)){
                /* get target ether_type */
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                        (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_ETHER_TYPE,
                        entry, &ether_type));
                
                ent_id = ether_type & BCM53242_MASK_PROTOCOL2VLAN_INDEX;

                /* set valid bit */
                field_val32 = 1;
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VALID,
                        entry, &field_val32));

                /* Protocol2VLAN write */
                rv = drv_bcm53242_mem_write(unit, DRV_MEM_PROTOCOLVLAN,
                        ent_id, 1, entry);
            }
        }

        return rv;
    }

    MEM_LOCK(unit, L2_ARLm);
    /* search entry */
    sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
    is_override = FALSE;
    if ((rv =  _drv_bcm53242_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            (uint32 *)&output1, flags, &index))< 0) {
        if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
            goto mem_insert_exit;              
        }
    }
    /* Return SOC_E_NONE instead of SOC_E_EXISTS to fit DV test. */
    if (rv == SOC_E_EXISTS) {

        if (!sal_memcmp(&output, entry, 
                sizeof(l2_arl_sw_entry_t)) ){

            rv = SOC_E_NONE;
            goto mem_insert_exit;
        } else {
            /* MAC Address */
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&output, (uint32 *)&mac_field_output)) < 0) {
                goto mem_insert_exit;
            }
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    entry, (uint32 *)&mac_field_entry)) < 0) {
                goto mem_insert_exit;
            }
            if (!sal_memcmp(&mac_field_entry, &mac_field_output, 
                    sizeof(mac_field_output)) ){
                 /*  VLAN ID  */
                if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)&output, &vid_output)) < 0) {
                    goto mem_insert_exit;
                }
                if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        entry, &vid_entry)) < 0) {
                    goto mem_insert_exit;
                }
                if (vid_output != vid_entry){
                    rv = SOC_E_NONE;
                    goto mem_insert_exit;
                }                
            } else {
               rv = SOC_E_NONE;
                goto mem_insert_exit;
            }

            is_override = TRUE;
            /* retrieve the original Uncast status */
            SAL_MAC_ADDR_FROM_UINT64(mac_addr, mac_field_output);
            ori_ucast = (mac_addr[0] & 0x1) ? FALSE : TRUE;
            
            /* check static status change */
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    (uint32 *)&output, &st_output)) < 0) {
                goto mem_insert_exit;
            }
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry, &st_entry)) < 0) {
                goto mem_insert_exit;
            }
            ori_st = st_output;
            if (st_output == st_entry){
                st_override_status = _DRV_ST_OVERRIDE_NO_CHANGE;
            } else {
                if (st_output){
                    st_override_status = _DRV_ST_OVERRIDE_ST2DYN;
                } else {
                    st_override_status = _DRV_ST_OVERRIDE_DYN2ST;
                }
            }

            /* retrieve source port */
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&output, (uint32 *)&ori_port)) < 0) {
                goto mem_insert_exit;
            }
            
        }
    }
    /* write entry */
    
    /* form entry */
    /* VLAN ID */
    if ((rv = (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid)) < 0) {
        goto mem_insert_exit;
    }

    /* MAC Address */
    if ((rv = (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            entry, (uint32 *)&mac_field)) < 0) {
        goto mem_insert_exit;
    }
    SAL_MAC_ADDR_FROM_UINT64(mac_addr, mac_field);
    
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, 
        ARL_MACADDRf, (uint32 *)&mac_field);

    if (mac_addr[0] & 0x01) { /* The input is the mcast address */

        is_ucast = FALSE;
        
        /* multicast group index */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, entry, &temp);
        value = temp & 0x3f;
        
        /* Port number should subtract 24 for BCM53242 */
        value -= 24;

        (DRV_SERVICES(unit)->reg_field_set)
           (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_PIDf, &value);
        value = (temp >> 6) & 0x1f;
        (DRV_SERVICES(unit)->reg_field_set)
           (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_ENTRY_RSRV0f, &value);
        value = (temp >> 11) & 0x01;
        (DRV_SERVICES(unit)->reg_field_set)
           (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_AGEf, &value);
        /* arl_control */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_CONf, &temp);
        /* static :
         *  - ROBO chip arl_control_mode at none-zero value cab't work 
         *    without static setting.
         */
        if (temp == 0 ) {
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
        } else {
            temp = 1;
        }
        is_dynamic = (temp) ? FALSE : TRUE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_STATICf, &temp);

        /* valid bit */
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_VALIDf, &temp);
    } else { /* unicast address */

        is_ucast = TRUE;
        /* source port id */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_PIDf, &temp);
        src_port = temp;

        /* age */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_AGEf, &temp);

        /* arl_control */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_CONf, &temp);

        /* static :
         *  - ROBO chip arl_control_mode at none-zero value can't work 
         *    without static setting.
         */
        if (temp == 0 ) {
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
        } else {
            temp = 1;
        }
        is_dynamic = (temp) ? FALSE : TRUE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_STATICf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, ARL_VALIDf, &temp);
    }


    /* write ARL and VID entry register*/
    if (index == 0){ /* entry 0 */
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VID_ENTRY_0r, 0, 0), &vid, 2)) < 0) {
            goto mem_insert_exit;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_ENTRY_0r, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
            goto mem_insert_exit;
        }
    } else { /* entry 1 */
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VID_ENTRY_1r, 0, 0), &vid, 2)) < 0) {
            goto mem_insert_exit;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_ENTRY_1r, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
            goto mem_insert_exit;
        }
    }

    MEM_RWCTRL_REG_LOCK(soc);
    /* Write ARL Read/Write Control Register */
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_insert_exit;
    }
    temp = MEM_TABLE_WRITE;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &control, TAB_RWf, &temp);
    temp = BCM53242_ARL_TABLE_ACCESS;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &control, TAB_INDEXf, &temp);
    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_insert_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_insert_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_insert_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);
    
    /* Return SOC_E_NONE instead of SOC_E_EXISTS to fit DV test. */
    if (rv == SOC_E_EXISTS) {
        rv = SOC_E_NONE;
    }

    /* SA Learning Count handler :
     *  - increase one for the ARL insertion process
     *      (for Dynamic and Unicast entry only)
     *
     *  SW SA learn count handling process :
     *  ------------------------------------------
     *  If the L2 insert entry is new created on a empty entry space.
     *      1. Dynamic + Unicast 
     *          >> INCREASE one in SA_LRN_CNT
     *  else ( L2 entry insert to a existed entry)
     *      1. No port movement
     *          a. If both of original and this entries are uicast
     *              1). ST changed from Dynamic to Static
     *                  >> DECREASE one in SA_LRN_CNT
     *              2). ST changed from Static to Dynamic
     *                  >> INCREASE one in SA_LRN_CNT
     *          b. else if original entry is MCast and this entry is unicast
     *              1). This entry is Dynamic
     *                  >> INCREASE one in SA_LRN_CNT
     *          c. else if original entry is Unicast and this entry is MCast
     *              1). Original entry is Dynamic
     *                  >> DECREASE one in original port's SA_LRN_CNT
     *      2. Port movement
     *          a. Original entry is dynamic and unicast
     *              >> DECREASE one in original port's SA_LRN_CNT
     *          b. This entry is dynamic and unicast
     *              >> INCREASE entry in SA_LRN_CNT
     *
     */
    temp = 0;
    if (is_override == FALSE){
        if (is_dynamic && is_ucast){
            temp = DRV_PORT_SA_LRN_CNT_INCREASE;
        }
    } else {
        if (ori_port == src_port){
            if ((is_ucast == TRUE) && (ori_ucast == TRUE)){
                if (st_override_status == _DRV_ST_OVERRIDE_ST2DYN){
                    temp = DRV_PORT_SA_LRN_CNT_INCREASE;
                } else if (st_override_status == _DRV_ST_OVERRIDE_DYN2ST){
                    temp = DRV_PORT_SA_LRN_CNT_DECREASE;
                }
            } else if ((is_ucast == TRUE) && (ori_ucast == FALSE)){
                if (is_dynamic){
                    temp = DRV_PORT_SA_LRN_CNT_INCREASE;
                }
            } else if ((is_ucast == FALSE) && (ori_ucast == TRUE)){
                if (ori_st == FALSE){
                    temp = DRV_PORT_SA_LRN_CNT_DECREASE;
                }
            } else {
                /* both not ucast >> No SA learn count handling action!! */
            }
        } else {
            /* station removment(port changed)! */
            if (is_ucast && is_dynamic){
                temp = DRV_PORT_SA_LRN_CNT_INCREASE;
            }

            if (ori_st == FALSE && ori_ucast){
                /* decrease original port's SA learn count  */
                rv = DRV_ARL_LEARN_COUNT_SET(unit, 
                        ori_port, DRV_PORT_SA_LRN_CNT_DECREASE, 0);
                if (SOC_FAILURE(rv)){
                    goto mem_insert_exit;
                }   
                soc_cm_debug(DK_ARL,
                        "%s,port%d,SA_LRN_CNT decrease one!\n",
                        FUNCTION_NAME(), ori_port);
            }
        }
    }
    /* performing SW SA learn count handling process on THIS PORT */
    if (temp){
        rv = DRV_ARL_LEARN_COUNT_SET(unit, src_port, temp, 0);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }   
        soc_cm_debug(DK_ARL,"%s,port%d,SA_LRN_CNT %s one!\n",
                FUNCTION_NAME(), src_port, 
                (temp == DRV_PORT_SA_LRN_CNT_INCREASE) ? 
                "increase" : "decrease");
    }

    sw_arl_update = 1;
    
mem_insert_exit:
    MEM_UNLOCK(unit,L2_ARLm);

    if (sw_arl_update){
        /* Add the entry to sw database */
        _drv_arl_database_insert(unit, index, entry);
    }

    return rv;
}

/*
 *  Function : drv_bcm53242_mem_delete
 *
 *  Purpose :
 *      Remove an entry to specific memory or remove entries by flags 
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      entry     :   entry data pointer.
 *      flags     :   delete flags.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for ARL memory now.
 *
 */
int 
drv_bcm53242_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t output, output1;
    uint32 temp, count, mem_id;
    int index;
    uint32  reg_len, reg_addr, reg_value;
    uint32 ag_port_mode = 0, ag_vlan_mode = 0, ag_spt_mode = 0;
    uint32  ag_static_mode = 0;
    uint32  mst_con, age_out_ctl;
    uint64 temp_mac_field;
    uint8  temp_mac_addr[6];
    uint32 temp_vid;
    uint64 entry_reg;
    uint32      src_port= 0, vlanid = 0;
    int         is_ucast = 0, is_dynamic = 0;
    
    soc_cm_debug(DK_MEM | DK_ARL, "drv_mem_delete : mem=0x%x, flags = 0x%x)\n",
         mem, flags);
    switch(mem) {
        case DRV_MEM_ARL:
            mem_id = L2_ARL_SWm;
            break;
        case DRV_MEM_MARL:
            mem_id = L2_ARL_SWm;
            /* MARL entries should do normal deletion, not fast aging. */
            flags &= ~DRV_MEM_OP_DELETE_BY_PORT;
            flags &= ~DRV_MEM_OP_DELETE_BY_VLANID;
            flags &= ~DRV_MEM_OP_DELETE_BY_SPT;
            flags &= ~DRV_MEM_OP_DELETE_BY_STATIC;
            break;
        case DRV_MEM_MACVLAN:

            rv = _drv_bcm53242_mem_macvlan_delete(unit, entry, 0);
            return rv;
            break;
        case DRV_MEM_PROTOCOLVLAN:
            
            rv = _drv_bcm53242_mem_protococlvlan_delete(unit, entry, 0);
            return rv;
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        case DRV_MEM_VLANVLAN:
        case DRV_MEM_FLOWVLAN:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    if (flags & DRV_MEM_OP_DELETE_BY_PORT) {
        ag_port_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_VLANID) {
        ag_vlan_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_SPT) {
        ag_spt_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_STATIC) {
        ag_static_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
        if (mem == DRV_MEM_MARL) {
            rv = _drv_bcm53242_mem_marl_delete_all(unit);
            
            return rv;
        } else {
            rv = _drv_bcm53242_mem_table_reset(unit, mem);
            if (rv == SOC_E_NONE) {
                /* Remove entries from software table by port/vlan */
                _drv_arl_database_delete_by_portvlan(unit, src_port,
                        vlanid, flags);
            }
            return rv;
        }
    }
    
    if ((ag_port_mode) || (ag_vlan_mode)) {
        /* 
         * aging port and vlan mode 
         */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, AGEOUT_CTLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, AGEOUT_CTLr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_delete_exit;
        }

        /*
         * keep original age port id and vid
         */
        age_out_ctl = reg_value;

        /* aging port mode */
        if (ag_port_mode) {
            temp = 0;
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
            src_port = temp;

            /* Port number should add 24 for BCM53242 */
            temp = temp + 24;
            
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, AGEOUT_CTLr, &reg_value, AGE_EN_PORTf, &temp));
        }

        /* aging vlan mode */
        if (ag_vlan_mode) {
            temp = 0;
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &temp);
            vlanid = temp;

            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, AGEOUT_CTLr, &reg_value, AGE_EN_VIDf, &temp));
        } 

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_delete_exit;
        }

        /* start fast aging process */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MST_CONr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MST_CONr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_delete_exit;
        }
        mst_con = reg_value;
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
            AGE_MODE_PRTf, &ag_port_mode));
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
            AGE_MODE_VLANf, &ag_vlan_mode));
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
            EN_AGE_STATICf, &ag_static_mode));
        temp = 1;
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
            FAST_AGE_STDNf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_delete_exit;
        }
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, MST_CONr, &reg_value, 
                    FAST_AGE_STDNf, &temp));
            if (!temp) {
                break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_delete_exit;
        }
        /* Remove entries from software table by port/vlan */
        _drv_arl_database_delete_by_portvlan(unit, src_port,
            vlanid, flags);
    } else { /* normal deletion */
        /* get static status for normal deletion */
        if (flags & DRV_MEM_OP_DELETE_BY_STATIC) {
            ag_static_mode = 1;
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry, &ag_static_mode));
        }

        MEM_LOCK(unit,L2_ARLm);
        /* search entry */
        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
        if ((rv =  _drv_bcm53242_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            (uint32 *)&output1, flags, &index))< 0) {
            if((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
                if (rv == SOC_E_FULL) {
                /* For mem_delete if mem_search return SOC_E_FULL it means
                  * the entry is not found. */
                    rv = SOC_E_NOT_FOUND;
                }
                goto mem_delete_exit;              
            }
        }

        /* clear entry */
        if (rv == SOC_E_EXISTS) {
            if (index == 0) {
                /* entry 0 */
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_ENTRY_0r, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
                    goto mem_delete_exit;
                }
                /* check static bit if set */
                if (!ag_static_mode){
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                                    (unit, ARLA_ENTRY_0r, (uint32 *)&entry_reg, 
                                    ARL_STATICf, &temp));
                    if (temp) {
                        soc_cm_debug(DK_MEM, 
                                    "\t Entry exist with static=%d\n",
                                    temp);
                        rv = SOC_E_NOT_FOUND;
                        goto mem_delete_exit;
                    }
                }
            } else {
                /* entry 1 */
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_ENTRY_1r, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
                    goto mem_delete_exit;
                }
                /* check static bit if set */
                if (!ag_static_mode){
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                                    (unit, ARLA_ENTRY_1r, (uint32 *)&entry_reg, 
                                    ARL_STATICf, &temp));
                    if (temp) {
                        soc_cm_debug(DK_MEM, 
                                    "\t Entry exist with static=%d\n",
                                    temp);
                        rv = SOC_E_NOT_FOUND;
                        goto mem_delete_exit;
                    }
                }
            }

            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&output, (uint32 *)&temp_mac_field)) < 0) {
                goto mem_delete_exit;
            }

            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            is_ucast = (temp_mac_addr[0] & 0x1) ? FALSE : TRUE;

            /* retrieve the static staus */
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    (uint32 *)&output, &temp)) < 0) {
                goto mem_delete_exit;
            }
            is_dynamic = (temp) ? FALSE : TRUE;
            
            /* retrieve the src_port */
            if (is_ucast){
                if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        (uint32 *)&output, &temp)) < 0) {
                    goto mem_delete_exit;
                }
                src_port = temp;
            }
            
            /* Write VID Table Index Register */
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &temp_vid)) < 0) {
                goto mem_delete_exit;
            }

            rv = _drv_bcm53242_mem_arl_entry_delete(unit, temp_mac_addr, temp_vid, index);
            if (rv == SOC_E_NONE) {
                if (is_dynamic && is_ucast){
                    rv = DRV_ARL_LEARN_COUNT_SET(unit, src_port, 
                            DRV_PORT_SA_LRN_CNT_DECREASE, 0);
                    if (SOC_FAILURE(rv)){
                        goto mem_delete_exit;
                    }
                    soc_cm_debug(DK_ARL,
                            "%s,port%d, SA_LRN_CNT decreased one!\n",
                            FUNCTION_NAME(), temp);
                }

                sw_arl_update = 1;
            }
        }
    }

mem_delete_exit:
    /* 
     * Restore MST Settings
     */
    if ((ag_port_mode) || (ag_vlan_mode)) {
        /* MST configuration */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MST_CONr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MST_CONr, 0, 0);
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &mst_con, reg_len);

        /* Age Ports and VIDs */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, AGEOUT_CTLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, AGEOUT_CTLr, 0, 0);
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &age_out_ctl, reg_len);
    }else {
        MEM_UNLOCK(unit,L2_ARLm);
    }

    if (sw_arl_update){
        /* Remove the entry from sw database */
        _drv_arl_database_delete(unit, index, &output);
    }

    return rv;
}

/*
 *  Function : drv_bcm53242_mem_cache_get
 *
 *  Purpose :
 *    Get the status of caching is enabled for a specified memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory type.
 *      enable     :   status of cacheing enable or not.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for VLAN and SEC_MAC memory now.
 *
 */

int
drv_bcm53242_mem_cache_get(int unit,
                  uint32 mem, uint32 *enable)
{
    int mem_id;
    assert(SOC_UNIT_VALID(unit));

    switch (mem) {
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            break;
        default:
            *enable = FALSE;
            return SOC_E_NONE;
    }
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    /* 
    For robo-SDK, we defined one block for each memory types.
    */
    *enable = (SOC_MEM_STATE(unit, mem_id).cache[0] != NULL)? 1:0; 

    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm53242_mem_cache_set
 *
 *  Purpose :
 *    Set the status of caching is enabled for a specified memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory type.
 *      enable     :   status of cacheing enable or not.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for VLAN and SEC_MAC memory now.
 *
 */

int
drv_bcm53242_mem_cache_set(int unit,
                  uint32 mem, uint32 enable)
{
    soc_memstate_t *memState;
    int index_cnt;
    int cache_size, vmap_size;
    uint8 *vmap;
    uint32 *cache;
    int mem_id;
    int     count, i;
    int entry_size;

    assert(SOC_UNIT_VALID(unit));
    switch (mem) {
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            count = 1;
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    for (i = 0; i < count; i++) {
        assert(SOC_MEM_IS_VALID(unit, mem_id));
        entry_size = soc_mem_entry_bytes(unit, mem_id);
        memState = &SOC_MEM_STATE(unit, mem_id);
        index_cnt = soc_robo_mem_index_max(unit, mem_id) + 1;
        cache_size = index_cnt * entry_size;
        vmap_size = (index_cnt + 7) / 8;

        soc_cm_debug(DK_SOCMEM,
                     "drv_mem_cache_set: unit %d memory %s %sable\n",
                     unit, SOC_ROBO_MEM_UFNAME(unit, mem_id),
                     enable ? "en" : "dis");

        /* 
        For robo-SDK, we defined one block for each memory types.
        */
        /*
         * Turn off caching if currently enabled.
         */

        cache = memState->cache[0];
        vmap = memState->vmap[0];

        /* Zero before free to prevent potential race condition */

        if (cache != NULL) {
            memState->cache[0] = NULL;
            memState->vmap[0] = NULL;

            sal_free(cache);
            sal_free(vmap);
        }

        if (!enable) {
            return SOC_E_NONE;
        }

        MEM_LOCK(unit, mem_id);

        /* Allocate new cache */

        if ((cache = sal_alloc(cache_size, "table-cache")) == NULL) {
            MEM_UNLOCK(unit, mem_id);
            return SOC_E_MEMORY;
        }

        if ((vmap = sal_alloc(vmap_size, "table-vmap")) == NULL) {
            sal_free(cache);
            MEM_UNLOCK(unit, mem_id);
            return SOC_E_MEMORY;
        }

        sal_memset(vmap, 0, vmap_size);

        /* Set memState->cache last to avoid race condition */

        soc_cm_debug(DK_SOCMEM,
                     "drv_mem_cache_set: cache=%p size=%d vmap=%p\n",
                     (void *)cache, cache_size, (void *)vmap);

        memState->vmap[0] = vmap;
        memState->cache[0] = cache;

        MEM_UNLOCK(unit, mem_id);

        mem_id++;
    }

    return SOC_E_NONE;
}
