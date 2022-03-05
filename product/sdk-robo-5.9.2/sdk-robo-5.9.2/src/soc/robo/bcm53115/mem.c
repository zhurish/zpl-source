/*
 * $Id: mem.c,v 1.34 Broadcom SDK $
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
#include "robo_53115.h"



#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))

#if defined(BCM_53125) || defined(BCM_53128)
#define BCM53125_8051_ROM_MEMORY 0 
#define BCM53125_8051_RAM_MEMORY 1
#endif /* BCM_53125 */
#define BCM53115_VLAN_MEMORY 2 
#define BCM53115_ARL_MEMORY  3

#define BCM53115_MEM_OP_WRITE    0
#define BCM53115_MEM_OP_READ     1
#define BCM53115_MEM_OP_CLEAR    2

int
_drv_bcm53115_search_valid_op(int unit, uint32 *key, uint32 *entry, 
        uint32 *entry_1, uint32 flags)
{
    uint32  reg_len, reg_addr, control, reg_value;
    uint32  temp, process;
    int     rv = SOC_E_NONE, multicast = 0;
    uint32  count, search_valid, entry_valid;
    uint32  result, result1;
    uint64  entry0, entry1, temp_mac_field;
    uint32  vid_rw = 0;
    uint8   temp_mac_addr[6];


    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_SRCH_CTLr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_SRCH_CTLr, 0, 0);
    /* Check if Search Valid Procedure is running or not */  
    if (flags & DRV_MEM_OP_SEARCH_DONE) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &control, reg_len)) < 0) {
            return rv;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_SRCH_CTLr, &control, 
            ARLA_SRCH_STDNf, &temp)) < 0) {
            return rv;
        }
        if (temp) {
            rv = SOC_E_BUSY;    
        } else {
            rv = SOC_E_NONE;
        }
        
    } else if (flags & DRV_MEM_OP_SEARCH_VALID_START) {
        /* Start Search Valid procedure */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &control, reg_len)) < 0) {
            return rv;
        }
        process = 1;
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, ARLA_SRCH_CTLr, 
                &control, ARLA_SRCH_STDNf, &process));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &control, reg_len)) < 0) {
            return rv;
        }
        return rv;
    } else if (flags & DRV_MEM_OP_SEARCH_VALID_GET) {
        process = 1;
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)(unit, 
                reg_addr, &control, 1)) < 0) {
                goto mem_search_valid_get;
            }
            (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_SRCH_CTLr, 
                &control, ARLA_SRCH_STDNf, &process);
            (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_SRCH_CTLr, 
                &control, ARLA_SRCH_VLIDf, &search_valid);
            /* ARL search operation was done */
            if (!process){
                rv = SOC_E_NOT_FOUND;
                break;
            }
            if (!search_valid){
                continue;
            } else {
                break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_search_valid_get;
        }
        
        if (search_valid) {
            /* Get index value */
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
                    ARLA_SRCH_ADRf, key)) < 0) {
                goto mem_search_valid_get;
            }

            /* Read searched entry 0 MAC/VID register */
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)(unit, 
                    ARLA_SRCH_RSLT_0_MACVIDr, 0, 0), (uint32 *)&entry0, 
                    (DRV_SERVICES(unit)->reg_length_get)
                    (unit,ARLA_SRCH_RSLT_0_MACVIDr))) < 0) {
                goto mem_search_valid_get;
            }
                
            /* Read searched entry 1 MAC/VID register */
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)(unit, 
                    ARLA_SRCH_RSLT_1_MACVIDr, 0, 0), (uint32 *)&entry1, 
                    (DRV_SERVICES(unit)->reg_length_get)
                    (unit,ARLA_SRCH_RSLT_1_MACVIDr))) < 0) {
                goto mem_search_valid_get;
            }
            /* Read searched entry 0 result register */
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)(unit, 
                    ARLA_SRCH_RSLTr, 0, 0), &result, 
                    (DRV_SERVICES(unit)->reg_length_get)
                    (unit,ARLA_SRCH_RSLTr))) < 0) {
                goto mem_search_valid_get;
            }
            /* Read searched entry 1 result register */
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)(unit, 
                    ARLA_SRCH_RSLT_1r, 0, 0), &result1, 
                    (DRV_SERVICES(unit)->reg_length_get)
                    (unit,ARLA_SRCH_RSLT_1r))) < 0) {
                    goto mem_search_valid_get;
                }
            
            /* Check the valid bit of bin 0 entry */
            entry_valid = 0;
            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                ARLA_SRCH_RSLTr, &result, \
                ARLA_SRCH_RSLT_VLIDf, &entry_valid);
            if (entry_valid) {
                (DRV_SERVICES(unit)->reg_field_get)(unit, 
                        ARLA_SRCH_RSLT_0_MACVIDr, (uint32 *)&entry0, 
                        ARLA_SRCH_MACADDRf, (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    entry, (uint32 *)&temp_mac_field);
                if (temp_mac_addr[0] & 0x01) {
                    multicast = 1;
                }
                temp = 1;
                 /* Set VALID field */
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp);
                /* Read ARL Search Result VID Register */
                (DRV_SERVICES(unit)->reg_field_get)(unit, 
                    ARLA_SRCH_RSLT_0_MACVIDr, (uint32 *)&entry0, 
                        ARLA_SRCH_RSLT_VIDf, &vid_rw);
                /* Set VLANID field */
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw);
                /* Set MACADDR field */
                (DRV_SERVICES(unit)->reg_field_get)(unit, 
                        ARLA_SRCH_RSLT_0_MACVIDr, (uint32 *)&entry0, 
                        ARLA_SRCH_MACADDRf, (uint32 *)&temp_mac_field);        

                if (multicast) { /* mcast address */
                    /* Multicast Port Bitmap */
                    if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                        SOC_IS_ROBO53128(unit)) {
                        (DRV_SERVICES(unit)->reg_field_get)
                            (unit, ARLA_SRCH_RSLTr, &result, 
                            INDEX(PORTIDf), &temp); 
                    } else {
                        (DRV_SERVICES(unit)->reg_field_get)
                            (unit, ARLA_SRCH_RSLTr, &result, 
                            PORTID_Rf, &temp); 
                    }
                    ((DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                        entry, &temp)); 
                    /* STATIC */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result, 
                        ARLA_SRCH_RSLT_STATICf, &temp);
                    (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_MARL, DRV_MEM_FIELD_STATIC, 
                        entry, &temp); 
                    /* Priority */
                    (DRV_SERVICES(unit)->reg_field_get)
                         (unit, ARLA_SRCH_RSLTr, &result, 
                         ARLA_SRCH_RSLT_PRIf, &temp);
                    (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_MARL, 
                        DRV_MEM_FIELD_PRIORITY, entry, &temp); 
                    /* age */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result, 
                        ARLA_SRCH_RSLT_AGEf, &temp);
                    (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_MARL, DRV_MEM_FIELD_AGE, 
                        entry, &temp); 
                    /* ARL control */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result, 
                        ARL_CONf, &temp);
                    (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_MARL, DRV_MEM_FIELD_ARL_CONTROL, 
                        entry, &temp); 

                } else { /* unicast address */       
                    /* Source Port Number */
                    if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                        SOC_IS_ROBO53128(unit)) {
                        (DRV_SERVICES(unit)->reg_field_get)
                            (unit, ARLA_SRCH_RSLTr, &result, 
                            INDEX(PORTIDf), &temp);
                    } else {
                        (DRV_SERVICES(unit)->reg_field_get)
                            (unit, ARLA_SRCH_RSLTr, &result, 
                            PORTID_Rf, &temp);
                    }
                    ((DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        entry, &temp)); 
            
                    /* Priority */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result, 
                        ARLA_SRCH_RSLT_PRIf, &temp);
                    (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                        entry, &temp); 

                    /* Static Bit */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result, 
                        ARLA_SRCH_RSLT_STATICf, &temp);
                    (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                        entry, &temp); 

                    /* Hit bit */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result, 
                        ARLA_SRCH_RSLT_AGEf, &temp);
                    (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                        entry, &temp); 

                    /* ARL control */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result, 
                        ARL_CONf, &temp);
                    (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                        entry, &temp); 
                        
                }
        }

         /* Check the valid bit of bin 1 entry */
        entry_valid = 0;
        (DRV_SERVICES(unit)->reg_field_get)(unit, 
            ARLA_SRCH_RSLTr, &result1, \
            ARLA_SRCH_RSLT_VLIDf, &entry_valid);
        if (entry_valid) {
            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                    ARLA_SRCH_RSLT_0_MACVIDr, (uint32 *)&entry1, 
                    ARLA_SRCH_MACADDRf, (uint32 *)&temp_mac_field);
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                entry_1, (uint32 *)&temp_mac_field);
            if (temp_mac_addr[0] & 0x01) {
                multicast = 1;
            }
            temp = 1;
             /* Set VALID field */
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry_1, &temp);
            /* Read ARL Search Result VID Register */
            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                ARLA_SRCH_RSLT_0_MACVIDr, (uint32 *)&entry1, 
                    ARLA_SRCH_RSLT_VIDf, &vid_rw);
            /* Set VLANID field */
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry_1, &vid_rw);
            /* Set MACADDR field */
            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                    ARLA_SRCH_RSLT_0_MACVIDr, (uint32 *)&entry1, 
                    ARLA_SRCH_MACADDRf, (uint32 *)&temp_mac_field);
            

            if (multicast) { /* mcast address */
                /* Multicast Port Bitmap */
                if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                    SOC_IS_ROBO53128(unit)) {
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result1, 
                        INDEX(PORTIDf), &temp); 
                } else {
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result1, 
                        PORTID_Rf, &temp); 
                }
                ((DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                    entry_1, &temp)); 
                /* STATIC */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, &result1, 
                    ARLA_SRCH_RSLT_STATICf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_STATIC, 
                    entry_1, &temp); 
                /* Priority */
                (DRV_SERVICES(unit)->reg_field_get)
                     (unit, ARLA_SRCH_RSLTr, &result1, 
                     ARLA_SRCH_RSLT_PRIf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_MARL, 
                    DRV_MEM_FIELD_PRIORITY, entry_1, &temp); 
                /* age */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, &result1, 
                    ARLA_SRCH_RSLT_AGEf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_AGE, 
                    entry_1, &temp); 
                /* ARL control */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, &result1, 
                    ARL_CONf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_ARL_CONTROL, 
                    entry_1, &temp); 

            } else { /* unicast address */       
                /* Source Port Number */
                if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                    SOC_IS_ROBO53128(unit)) {
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result1, 
                        INDEX(PORTIDf), &temp);
                } else {
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result1, 
                        PORTID_Rf, &temp);
                }
                ((DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    entry_1, &temp)); 

                /* Priority */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, &result1, 
                    ARLA_SRCH_RSLT_PRIf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                    entry_1, &temp); 

                /* ARL control */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, &result1, 
                    ARL_CONf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                    entry_1, &temp); 

                /* Static Bit */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, &result1, 
                    ARLA_SRCH_RSLT_STATICf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry_1, &temp); 

                /* Hit bit */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, &result1, 
                    ARLA_SRCH_RSLT_AGEf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                    entry_1, &temp); 
                    
            }
        }
            rv = SOC_E_EXISTS;
            goto mem_search_valid_get;
                }
mem_search_valid_get:
        return rv;
    }

    return rv;
}


int
_drv_bcm53115_arl_mac_vid_match(int unit, uint32 *mac, uint32 vid, 
        uint32 *entry, uint32 flags)
{
    uint64  entry_mac_field;
    uint32  entry_vid;
    int match = 0;
    
    (DRV_SERVICES(unit)->reg_field_get)
       (unit, ARLA_MACVID_ENTRY0r, entry, ARL_MACADDRf,
       (uint32 *)&entry_mac_field);
    if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
        SOC_IS_ROBO53128(unit)) {
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_MACVID_ENTRY0r, entry, INDEX(VIDf), &entry_vid);
    } else {
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_MACVID_ENTRY0r, entry, VID_Rf, &entry_vid);
    }
    /* check if we have to overwrite this valid bin0 */
    if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
        /* check mac + vid */
        if (!sal_memcmp((uint32 *)&entry_mac_field, mac, 
                    sizeof(entry_mac_field)) && (vid == entry_vid)) {
            match = 1;
       }
    } else {
        /* check mac */
        if (!sal_memcmp((uint32 *)&entry_mac_field, mac, 
                sizeof(entry_mac_field))) {
           match = 1;
        }
    }
    return match;
}

/*
 *  Function : _drv_bcm53115_mem_search
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
 *      index   :   bin_id in this entry bucket (-1 ~ 3).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. DRV_MEM_OP_SEARCH_CONFLICT
 *      a. will be performed in the MAC+VID hash search section.
 *      b. '*entry_1' not be used but 'entry' will be used as l2_entry array
 *          and this l2_entry array size should not smaller than ARL table  
 *          hash bucket size(bin number).
 *      c. all valid entries in the MAC+VID hash bucket will be reported.
 */
int 
_drv_bcm53115_mem_search(int unit, uint32 mem, uint32 *key, 
                uint32 *entry, uint32 *entry_1, uint32 flags, int *index)
{
    int         rv = SOC_E_NONE;
    soc_control_t   *soc = SOC_CONTROL(unit);
    uint32      count, temp, i;
    uint32      control = 0;
    uint8       mac_addr_rw[6], temp_mac_addr[6];
    uint64      rw_mac_field, temp_mac_field;
    uint64      entry0;
    uint32      vid_rw;
    int         binNum = -1, matched_id = -1, dynamic_index = -1;
    int         valid[ROBO_53115_L2_BUCKET_SIZE], is_conflict = 0;
    uint32      reg_addr, reg_value;
    int         reg_len;
    gen_memory_entry_t      gmem_entry;
    uint8       hash_value[6];
    uint16      hash_result;
    uint32      result, mcast_pbmp;
    uint32      macvid_reg, result_reg;
    
    if ((flags & DRV_MEM_OP_SEARCH_DONE) ||
        (flags & DRV_MEM_OP_SEARCH_VALID_START) ||
        (flags & DRV_MEM_OP_SEARCH_VALID_GET)){
        rv = _drv_bcm53115_search_valid_op(unit, key, entry, entry_1, flags);
        return rv;
    } else if (flags & DRV_MEM_OP_BY_INDEX) {
        /* The entry index is inside the input parameter "key". */ 
        /* Read the memory raw data */
        (DRV_SERVICES(unit)->mem_read)(
            unit, mem, *key, 1, (uint32 *)&gmem_entry);
        /* Read VALID bit */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VALID, 
                (uint32 *)&gmem_entry, &temp);
        if (!temp) {
            return rv;
        }
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp);

        /* Read VLAN ID value */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VLANID, 
                (uint32 *)&gmem_entry, &vid_rw);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw);
        
        /* Read MAC address bit 10~47 */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_MAC, 
                (uint32 *)&gmem_entry, (uint32 *)&rw_mac_field);
        SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);
        /* check HASH enabled ? */
        /* check 802.1q enable */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL0r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)
            (unit, VLAN_CTRL0r, &reg_value, VLAN_ENf, &temp));
        sal_memset(hash_value, 0, sizeof(hash_value));
        if (temp) {
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)
                (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp));
            if (temp == 3) {
                /* hash value = VID + MAC */
                hash_value[0] = (vid_rw >> 4) & 0xff;
                hash_value[1] = ((vid_rw & 0xf) << 4) + (mac_addr_rw[1] & 0xf);
                hash_value[2] = mac_addr_rw[2];
                hash_value[3] = mac_addr_rw[3];
                hash_value[4] = mac_addr_rw[4];
                hash_value[5] = mac_addr_rw[5];
            } else if (temp == 0){
                /* hash value = MAC */
                hash_value[0] = ((mac_addr_rw[1] & 0xf) << 4) 
                    + ((mac_addr_rw[2] & 0xf0) >> 4);
                hash_value[1] = ((mac_addr_rw[2] & 0xf) << 4) 
                    + ((mac_addr_rw[3] & 0xf0) >> 4);
                hash_value[2] = ((mac_addr_rw[3] & 0xf) << 4) 
                    + ((mac_addr_rw[4] & 0xf0) >> 4);
                hash_value[3] = ((mac_addr_rw[4] & 0xf) << 4) 
                    + ((mac_addr_rw[5] & 0xf0) >> 4);
                hash_value[4] = ((mac_addr_rw[5] & 0xf) << 4);
            } else {
                return SOC_E_CONFIG;
            }
        } else {
            /* hash value = MAC value */
            hash_value[0] = ((mac_addr_rw[1] & 0xf) << 4) 
                    + ((mac_addr_rw[2] & 0xf0) >> 4);
            hash_value[1] = ((mac_addr_rw[2] & 0xf) << 4) 
                + ((mac_addr_rw[3] & 0xf0) >> 4);
            hash_value[2] = ((mac_addr_rw[3] & 0xf) << 4) 
                + ((mac_addr_rw[4] & 0xf0) >> 4);
            hash_value[3] = ((mac_addr_rw[4] & 0xf) << 4) 
                + ((mac_addr_rw[5] & 0xf0) >> 4);
            hash_value[4] = ((mac_addr_rw[5] & 0xf) << 4);
        }
        /* Get the hash revalue */
        _drv_arl_hash(hash_value, 48, &hash_result);
        /* Recover the MAC bit 0~11 */
        temp = *key;
        temp = temp >> 1; 
        hash_result = (hash_result ^ temp) & 0xfff;
        
        temp_mac_addr[0] = ((mac_addr_rw[1] & 0xf) << 4) 
            + ((mac_addr_rw[2] & 0xf0) >> 4);
        temp_mac_addr[1] = ((mac_addr_rw[2] & 0xf) << 4) 
            + ((mac_addr_rw[3] & 0xf0) >> 4);
        temp_mac_addr[2] = ((mac_addr_rw[3] & 0xf) << 4) 
            + ((mac_addr_rw[4] & 0xf0) >> 4);
        temp_mac_addr[3] = ((mac_addr_rw[4] & 0xf) << 4) 
            + ((mac_addr_rw[5] & 0xf0) >> 4);
        temp_mac_addr[4] = ((mac_addr_rw[5] & 0xf) << 4) 
            + (hash_result >> 8);
        temp_mac_addr[5] = hash_result & 0xff;
        SAL_MAC_ADDR_TO_UINT64(temp_mac_addr, temp_mac_field);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                entry, (uint32 *)&temp_mac_field);
        if (MAC_IS_MCAST(temp_mac_addr)) { /* multicast entry */
            /* Multicast index */
            mcast_pbmp = 0;
            (DRV_SERVICES(unit)->mem_field_get)(
                    unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&gmem_entry, &temp);
            mcast_pbmp = temp;
            (DRV_SERVICES(unit)->mem_field_get)(
                    unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_DEST_BITMAP, 
                    (uint32 *)&gmem_entry, &temp);
            mcast_pbmp += (temp << 4);
            (DRV_SERVICES(unit)->mem_field_set)(
                    unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                    entry, &mcast_pbmp);
        } else { /* unicast entry */
            /* Source port number */
            (DRV_SERVICES(unit)->mem_field_get)(
                    unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&gmem_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                    unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
            /* Priority value */
            (DRV_SERVICES(unit)->mem_field_get)(
                    unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_PRIORITY, 
                    (uint32 *)&gmem_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                    unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, entry, &temp);
            
        }
        /* Age bit */
        (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                (uint32 *)&gmem_entry, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp);
        /* ARL_control */
        (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_ARL_CONTROL, 
                (uint32 *)&gmem_entry, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);

        /* Static bit */
        (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_STATIC, 
                (uint32 *)&gmem_entry, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
        
        return rv;
     
    /* delete by MAC/MAC+VID */
    } else if (flags & DRV_MEM_OP_BY_HASH_BY_MAC) {
        l2_arl_sw_entry_t   *rep_entry;
        int     entry_len = sizeof(l2_arl_sw_entry_t);

        sal_memset(valid, 0, sizeof(valid));
        if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
            for (i = 0; i < ROBO_53115_L2_BUCKET_SIZE; i++){
                /* check the parameter for output entry buffer */
                rep_entry = (l2_arl_sw_entry_t *)entry + i;
                if (rep_entry == NULL){
                    soc_cm_debug(DK_WARN, 
                            "%s,entries buffer not allocated!\n", 
                            FUNCTION_NAME());
                    return SOC_E_PARAM;
                }

            }
        }
    
        ARL_MEM_SEARCH_LOCK(soc);
        /* enable 802.1Q and set VID+MAC to hash */
        reg_len = DRV_REG_LENGTH_GET(unit, VLAN_CTRL0r);
        reg_addr = DRV_REG_ADDR(unit, VLAN_CTRL0r, 0, 0);
        if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_search_exit;
        }
        temp = 1;
        if ((rv = DRV_REG_FIELD_SET(unit, VLAN_CTRL0r, &reg_value, 
                VLAN_ENf, &temp)) < 0){
            goto mem_search_exit;
        }
        /* check IVL or SVL */
        if ((rv = DRV_REG_FIELD_GET(unit, VLAN_CTRL0r, &reg_value, 
                VLAN_LEARN_MODEf, &temp)) < 0){
            goto mem_search_exit;
        }
        if (temp == 3){     /* VLAN is at IVL mode */
            if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                temp = 3;
            } else {
                temp = 0;
            }
            if ((rv = DRV_REG_FIELD_SET(unit, VLAN_CTRL0r, &reg_value, 
                    VLAN_LEARN_MODEf, &temp)) < 0){
                goto mem_search_exit;
            }
        }
        if ((rv = DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_search_exit;
        }

        /* Write MAC Address Index Register */
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                key, (uint32 *)&rw_mac_field)) < 0) {
            goto mem_search_exit;
        }
        SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);
        if ((rv = DRV_REG_WRITE(unit, DRV_REG_ADDR(unit, ARLA_MACr, 0, 0), 
                &rw_mac_field, 6)) < 0) {
            goto mem_search_exit;
        }

        if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
            /* Write VID Table Index Register */
            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VLANID, key, &vid_rw))< 0) {
                goto mem_search_exit;
            }
            if ((rv = DRV_REG_WRITE(unit, DRV_REG_ADDR(unit, ARLA_VIDr, 0, 0),
                    &vid_rw, 2)) < 0) {
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
        if ((rv = DRV_REG_FIELD_SET(unit, ARLA_RWCTLr, &control, 
                ARL_RWf, &temp)) < 0){
            goto mem_search_exit;
        }
        temp = 1;
        if ((rv = DRV_REG_FIELD_SET(unit, ARLA_RWCTLr, &control, 
                ARL_STRTDNf, &temp)) < 0){
            goto mem_search_exit;
        }
        if ((rv = DRV_REG_WRITE(unit, DRV_REG_ADDR(unit, ARLA_RWCTLr, 0, 0),
                &control, 1)) < 0) {
            goto mem_search_exit;
        }

        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            rv = DRV_REG_READ(unit, DRV_REG_ADDR(unit, ARLA_RWCTLr, 0, 0),
                    &control, 1);
            if (rv < 0) {
                goto mem_search_exit;
            }

            if ((rv = DRV_REG_FIELD_GET(unit, ARLA_RWCTLr, &control, 
                    ARL_STRTDNf, &temp)) < 0) {
                goto mem_search_exit;
            }
            if (!temp)
                break;
        }

        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_search_exit;
        }

        /* bcm53115 read operation will index to a 4 bins buckets */
        *index = -1;    /* assigning the initial value */
        for (i = 0 ;i < ROBO_53115_L2_BUCKET_SIZE; i++) {
            switch(i) {
            case 0:
                macvid_reg = ARLA_MACVID_ENTRY0r;
                result_reg = ARLA_FWD_ENTRY0r;
                break;
            case 1:
                macvid_reg = ARLA_MACVID_ENTRY1r;
                result_reg = ARLA_FWD_ENTRY1r;
                break;
            case 2:
                macvid_reg = ARLA_MACVID_ENTRY2r;
                result_reg = ARLA_FWD_ENTRY2r;
                break;
            case 3:
                macvid_reg = ARLA_MACVID_ENTRY3r;
                result_reg = ARLA_FWD_ENTRY3r;
                break;
            default:
                ARL_MEM_SEARCH_UNLOCK(soc);
                return SOC_E_INTERNAL;
            }
            if ((rv = DRV_REG_READ(unit, DRV_REG_ADDR(unit, result_reg, 0, 0),
                    &result, 4)) < 0) {
                goto mem_search_exit;
            }
            if ((rv = DRV_REG_FIELD_GET(unit, result_reg, &result, 
                    ARL_VALIDf, &temp)) < 0) {
                goto mem_search_exit;
            }
            /* Check valid bit */
            if (!temp) {
                if (binNum == -1) {
                    binNum = i;     /* assigning the free bin_id */
                }
                continue;
            }

            /* valid[] updating process :
             *  a. CONFLICT search is asserted :
             *      - valid[] will be updated for each bin
             *  b. CONFLICT search is not asserted :
             *      - valid[] may not be all updated once the match occurred.
             */
            valid[i] = TRUE;
            if (flags & DRV_MEM_OP_SEARCH_CONFLICT){

                /* for conflict search, to know the valid status is enough */
                continue;
            }

            /* Read MAC/VID entry register */
            if ((rv = DRV_REG_READ(unit, DRV_REG_ADDR(unit, macvid_reg, 0, 0),
                    &entry0, 8)) < 0) {
                goto mem_search_exit;
            }
            
            /* Compare the MAC and VID value */
            if (_drv_bcm53115_arl_mac_vid_match
                    (unit, (uint32*)&rw_mac_field, 
                    vid_rw, (uint32 *)&entry0, flags)) {
                    
                binNum = i; /* update binNum to report the existed bin_id */
                matched_id = binNum;

                break;
            } else {

                /* DRV_MEM_OP_REPLACE processing :
                 *  a. REPLACE is set and non-static entry found
                 *      - dynamic_index = 0~3
                 *  b. REPLACE is set but no non-static entry
                 *      - dynamic_index = -1
                 *  c. REPLACE is not set 
                 *      - dynamic_index = -1
                 */
                if (flags & DRV_MEM_OP_REPLACE){
                    /* Check static valid */
                    if ((rv = DRV_REG_FIELD_GET(unit, result_reg, &result, 
                            ARL_STATICf, &temp)) < 0) {
                        goto mem_search_exit;
                    }
                    if (!temp) {
                        dynamic_index = i;
                    }
                }
            }
        }

        /* After the process above, the possible condition will be :
         *  1. FOUND : the MAC+VID matched and valid entry is found.
         *      - binNum = 0~3
         *      - matched_id = binNum
         *  2. FULL : bucket FULL(all bins are valid)
         *      - binNum = -1
         *      - matched_id = -1
         *  3. NOT_FOUND : no valid bin with MAC+VID matched.
         *      - binNum = 0~3
         *      - matched_id = -1
         */
        if (binNum == -1){
            /* Condition here :
             *  - every entry in those 4 bins are valid and no matached
             *      MAC+VID
             */
            
            if ((flags & DRV_MEM_OP_REPLACE) && (dynamic_index != -1)){
                binNum = dynamic_index;
            }
        }

        /* check the searching operation :
         *  1. Normal search : search the entry with MAC+VID been matched
         *    a. SOC_E_EXISTS : found the matched entry
         *      - '*entry' : report this matched entry(binNum)
         *      - '*index' : report the matched entry index(binNum)
         *    b. SOC_E_FULL : all 4 bins are valid but no MAC+VID matched.
         *      - '*entry' : 
         *          1). if REPLACE flag is not set, report all zero entry
         *          2). if REPLACE flag is set, report a found entry which 
         *              is non-static entry in this hash bucket.(binNum)
         *      - '*index' : 
         *          1). if REPLACE flag is not set, report -1
         *          2). if REPLACE flag is set, report a found bin_id(binNum)
         *              of an entry which is non-static entry.
         *              (allow to be replaced)
         *    c. SOC_E_NOT_FOUND : no MAC+VID matched entry and the hash 
         *          bucket is not full(some bin is invalid)
         *      - '*entry' : report all zero entry
         *      - '*index' : report a found bin_id which is invalid.(binNum)
         *  2. Conflict search : search all valid entries in this MAC_VID hash
         *      bucket.
         *      - '*index' : report -1 (for ignor)
         *    a. SOC_E_EXISTS : found the valid entry/entries in this bucket
         *      - '*entry' : report those conflicted entries
         *    b. SOC_E_NOT_FOUND : hash bucket has no valid entry (EMPTY)
         *      - '*entry' : report all zero entry
         */

        /* reporting entry/entries and the index process :
         *  1. for conflict search,
         *      >> report all valid but not matched entries.
         *  2. for normal search,
         *      >> report the valid entry with bin_id=binNum.
         */
        for (i = 0; i < ROBO_53115_L2_BUCKET_SIZE; i++){
            rep_entry = (l2_arl_sw_entry_t *)entry + i;
            /* init the reporting entry and index buffer */
            memset(rep_entry, 0, entry_len);
            
            if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                if (valid[i] == 0){
                    /* no report on this bin */
                    continue;
                } else {
                    /* report this entry */
                    is_conflict = TRUE;
                }
            } else {
                if (binNum == -1){
                    /* FULL condition, no reported entry and index */
                    break;
                } else {
                    /* report this entry */
                    *index = binNum;
                    /* force one time report process only */
                    i = binNum;
                }
            }

            /* ------ entries report process ------ */
            if (valid[i] == TRUE){
                switch(i) {
                case 0:
                    macvid_reg = ARLA_MACVID_ENTRY0r;
                    result_reg = ARLA_FWD_ENTRY0r;
                    break;
                case 1:
                    macvid_reg = ARLA_MACVID_ENTRY1r;
                    result_reg = ARLA_FWD_ENTRY1r;
                    break;
                case 2:
                    macvid_reg = ARLA_MACVID_ENTRY2r;
                    result_reg = ARLA_FWD_ENTRY2r;
                    break;
                case 3:
                    macvid_reg = ARLA_MACVID_ENTRY3r;
                    result_reg = ARLA_FWD_ENTRY3r;
                    break;
                default:
                    ARL_MEM_SEARCH_UNLOCK(soc);
                    return SOC_E_INTERNAL;
                }

                /* Read MAC/VID entry register */
                if ((rv = DRV_REG_READ(unit, 
                        DRV_REG_ADDR(unit, macvid_reg, 0, 0), 
                        (uint32 *)&entry0, 8)) < 0) {
                    goto mem_search_exit;
                }
                COMPILER_64_ZERO(temp_mac_field);
                if ((rv = DRV_REG_FIELD_GET(unit, 
                        macvid_reg, (uint32 *)&entry0, INDEX(ARL_MACADDRf), 
                        (uint32 *)&temp_mac_field)) < 0){
                    goto mem_search_exit;
                }

                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                
                /* Only need write the selected Entry Register */
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_MAC, (uint32 *)rep_entry, 
                        (uint32 *)&temp_mac_field)) < 0){
                    goto mem_search_exit;
                }

                /* VID */
                if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                    SOC_IS_ROBO53128(unit)) {
                    if ((rv = DRV_REG_FIELD_GET(unit, macvid_reg, 
                            (uint32 *)&entry0, INDEX(VIDf), &temp)) < 0){
                        goto mem_search_exit;
                    }
                } else {
                    if ((rv = DRV_REG_FIELD_GET(unit, macvid_reg,
                            (uint32 *)&entry0, INDEX(VID_Rf), &temp)) < 0){
                        goto mem_search_exit;
                    }
                }
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }

                if ((rv = DRV_REG_READ(unit, 
                        DRV_REG_ADDR(unit, result_reg, 0, 0), 
                        (uint32 *)&result, 4)) < 0) {
                    goto mem_search_exit;
                }

                if (MAC_IS_MCAST(temp_mac_addr)) { /* is mcast address */
                    mcast_pbmp = 0;
                    if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                            SOC_IS_ROBO53128(unit)) {
                        if ((rv = DRV_REG_FIELD_GET(unit, result_reg, 
                                &result, INDEX(PORTIDf), &temp)) < 0){
                            goto mem_search_exit;
                        }
                    } else {
                        if ((rv = DRV_REG_FIELD_GET(unit, result_reg, 
                                &result, INDEX(PORTID_Rf), &temp)) < 0){
                            goto mem_search_exit;
                        }
                    }
                    mcast_pbmp = temp;
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_DEST_BITMAP, (uint32 *)rep_entry, 
                            &temp)) < 0){
                        goto mem_search_exit;
                    }
                } else { /* The input is the unicast address */
                    if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                            SOC_IS_ROBO53128(unit)) {
                        if ((rv = DRV_REG_FIELD_GET(unit, result_reg, 
                                (uint32 *)&result, INDEX(PORTIDf), 
                                &temp)) < 0){
                            goto mem_search_exit;
                        }
                    } else {
                        if ((rv = DRV_REG_FIELD_GET(unit, result_reg, 
                                (uint32 *)&result, INDEX(PORTID_Rf), 
                                &temp)) < 0){
                            goto mem_search_exit;
                        }
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, 
                            DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                            (uint32 *)rep_entry, &temp)) < 0){
                        goto mem_search_exit;
                    }
                }
                
                if ((rv = DRV_REG_FIELD_GET(unit, result_reg, 
                        (uint32 *)&result, INDEX(ARL_AGEf), &temp)) < 0){
                    goto mem_search_exit;
                }
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_AGE, (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }

                if ((rv = DRV_REG_FIELD_GET(unit, result_reg, 
                        (uint32 *)&result, INDEX(ARL_VALIDf), &temp)) < 0){
                    goto mem_search_exit;
                }
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_VALID, (uint32 *)rep_entry, 
                        &temp)) < 0){
                    goto mem_search_exit;
                }

                if ((rv = DRV_REG_FIELD_GET(unit, result_reg, 
                        (uint32 *)&result, INDEX(ARL_PRIf), &temp)) < 0){
                    goto mem_search_exit;
                }
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_PRIORITY, (uint32 *)rep_entry, 
                        &temp)) < 0){
                    goto mem_search_exit;
                }
                
                if ((rv = DRV_REG_FIELD_GET(unit, result_reg, 
                        (uint32 *)&result, INDEX(ARL_STATICf), &temp)) < 0){
                    goto mem_search_exit;
                }
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_STATIC, (uint32 *)rep_entry, 
                        &temp)) < 0){
                    goto mem_search_exit;
                }

                if ((rv = DRV_REG_FIELD_GET(unit, result_reg, 
                        (uint32 *)&result, INDEX(ARL_CONf), &temp)) < 0){
                    goto mem_search_exit;
                }
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_ARL_CONTROL, (uint32 *)rep_entry, 
                        &temp)) < 0){
                    goto mem_search_exit;
                }
            }

            /* force one time report process only */
            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                break;
            }
        }

        /* reporting the search result */
        if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){     /* normal search */
            if (matched_id != -1){
                /* assume binNum = matched_id in this condition  */
                if (binNum < 0 || binNum >= ROBO_53115_L2_BUCKET_SIZE){
                    soc_cm_debug(DK_WARN, "%s, unexpected search result!\n", 
                            FUNCTION_NAME());
                    rv = SOC_E_INTERNAL;
                } else {
                    /* matched entry founded */
                    rv = SOC_E_EXISTS;
                }
            } else if (binNum == -1){ 
                /* no matched entry and 4 bins all valid */
                rv = SOC_E_FULL;
            } else {
                /* processing here is no matched entry but a free bin_id 
                 * will be indicated to accept new insert entry.
                 */
                rv = SOC_E_NOT_FOUND;
            }
        } else {        /* conflict search */
            if (is_conflict == TRUE){
                rv = SOC_E_EXISTS;
            } else {
                /* no conflict entry. (no valid entry in this hash bucket */
                rv = SOC_E_NOT_FOUND;
            }
        }
mem_search_exit:
        ARL_MEM_SEARCH_UNLOCK(soc);
        soc_cm_debug(DK_ARL | DK_TESTS, 
                "%s,%d,binNum=%d, matched_id=%d, isConflict=%d\n",
                FUNCTION_NAME(), rv, binNum, matched_id, is_conflict);
        return rv;
    }else {
        soc_cm_debug(DK_WARN, "%s,%d, Unexpect process here!\n",
                FUNCTION_NAME(), __LINE__);
        return SOC_E_INTERNAL;
    }
}

/*
 *  Function : _drv_bcm53115_egr_v2v_mem_clear
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
_drv_bcm53115_egr_v2v_mem_clear(int unit)
{
    int rv = SOC_E_NONE;
    uint32 retry;
    uint32  reg_len, reg_addr, temp, reg_value;

    soc_cm_debug(DK_VLAN, "%s\n", FUNCTION_NAME());

    /* read control setting */                
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, EGRESS_VID_RMK_TBL_ACSr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, EGRESS_VID_RMK_TBL_ACSr);

    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        goto egrv2v_mem_clear_exit;
    }

    temp = 1;   /* set reset bit */
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, RESET_EVTf, &temp);

    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, START_DONEf, &temp);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        goto egrv2v_mem_clear_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto egrv2v_mem_clear_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                START_DONEf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto egrv2v_mem_clear_exit;
    }

egrv2v_mem_clear_exit:     
    return rv;
}

/*
 *  Function : _drv_bcm53115_egr_v2v_mem_read
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :   the entry's index (include port and entry_id).
 *      count       :   one or more netries to be read.
 *      entry_data  :   pointer to a buffer of 32-bit words 
 *                              to contain the read result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53115_egr_v2v_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int     rv = SOC_E_NONE;
    int     i, port_id, index, temp_id;
    uint32  retry;
    uint32  reg_len, reg_addr, temp, reg_value;
    egress_vid_remark_entry_t *v2v_mem = 0;

    soc_cm_debug(DK_VLAN, "%s,entry_id=%d,count=%d\n", 
            FUNCTION_NAME(), entry_id, count);
    v2v_mem = (egress_vid_remark_entry_t *)entry;
    
    /* get port id from original entry id */
    port_id = (entry_id & DRV_EGR_V2V_PORT_ADDR_MASK) >> 
                DRV_EGR_V2V_PORT_ADDR_OFFSET;
    index = (entry_id & DRV_EGR_V2V_ENTRY_ADDR_MASK) >> 
                DRV_EGR_V2V_ENTRY_ADDR_OFFSET;
    
    /* check if the port_id is not in the valid range */
    if (port_id > DRV_EGR_V2V_IMP_PORT_ADDR_ID){
        rv = SOC_E_FULL;
        goto mem_read_exit;
    } else if (port_id == DRV_EGR_V2V_IMP_PORT_ADDR_ID){
        port_id = DRV_EGR_V2V_IMP_PORT_SEARCH_ID;
    }
    
    /* 1. per port containing up to 1~255 entries.
     * 2. the counter number might large enough to force SW to process at next 
     *      port's entryies.
     * 3. read process will be stopped if the reading count over the max entry.
     */
    for (i = 0, temp_id = index; i < count; i++ ) {
        
        /* ---- reading process ---- */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, EGRESS_VID_RMK_TBL_ACSr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, EGRESS_VID_RMK_TBL_ACSr);
        
        /* set index key(port) */
        reg_value = 0;
        temp = port_id;
        if (SOC_IS_ROBO53115(unit)) {
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                EGRESS_PORT_Rf, &temp);
        } else {
            /* BCM53125 */
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                INDEX(EGRESS_PORTf), &temp);
        }
        /* set index key(index) */
        temp = temp_id;
        (DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                TBL_ADDRf, &temp);
                
        /* set read/write OP */
        temp = 0;   /* 0 means read OP in this table */
        (DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                OPf, &temp);
        
        /* Start to read */
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                START_DONEf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_read_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                    (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                    START_DONEf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }
        
        /* get result */        
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, EGRESS_VID_RMK_TBL_DATAr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, EGRESS_VID_RMK_TBL_DATAr);

        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        (DRV_SERVICES(unit)->reg_field_get)
                (unit, EGRESS_VID_RMK_TBL_DATAr, &reg_value, 
                OUTER_OPf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_OP, 
                (uint32 *)v2v_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_get)
                (unit, EGRESS_VID_RMK_TBL_DATAr, &reg_value, 
                OUTER_VIDf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_VID, 
                (uint32 *)v2v_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_get)
                (unit, EGRESS_VID_RMK_TBL_DATAr, &reg_value, 
                INNER_OPf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_OP, 
                (uint32 *)v2v_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_get)
                (unit, EGRESS_VID_RMK_TBL_DATAr, &reg_value, 
                INNER_VIDf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_VID, 
                (uint32 *)v2v_mem, &temp);

        v2v_mem++;
        
        /* ---- index to next for count assignment ---- */
        temp_id++ ;
        if (temp_id >= DRV_EGRESS_V2V_NUM_PER_PORT){
            if (port_id == DRV_EGR_V2V_IMP_PORT_SEARCH_ID){
                rv = SOC_E_FULL;
                goto mem_read_exit;
            } else {
                port_id = ((port_id + 1) < DRV_EGR_V2V_IMP_PORT_ADDR_ID) ? 
                            (port_id + 1) : DRV_EGR_V2V_IMP_PORT_SEARCH_ID;
                temp_id = 0;
            }
        }
    }
    
mem_read_exit:     
    return rv;
}

/*
 *  Function : _drv_bcm53115_egr_v2v_mem_write
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :   the entry's index (include port and entry_id).
 *      count       :   one or more netries to be writen.
 *      entry_data  :   pointer to a buffer of 32-bit words 
 *                              to contain the writing result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53115_egr_v2v_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int     rv = SOC_E_NONE;
    int     i, port_id, index, temp_id;
    uint32  retry;
    uint32  reg_len, reg_addr, temp, reg_value;
    egress_vid_remark_entry_t *v2v_mem = 0;

    soc_cm_debug(DK_VLAN, "%s,entry_id=%d,count=%d\n", 
            FUNCTION_NAME(), entry_id, count);
    v2v_mem = (egress_vid_remark_entry_t *)entry;
    
    /* get port id from original entry id */
    port_id = (entry_id & DRV_EGR_V2V_PORT_ADDR_MASK) >> 
                DRV_EGR_V2V_PORT_ADDR_OFFSET;
    index = (entry_id & DRV_EGR_V2V_ENTRY_ADDR_MASK) >> 
                DRV_EGR_V2V_ENTRY_ADDR_OFFSET;
    
    /* check if the port_id is not in the valid range */
    if (port_id > DRV_EGR_V2V_IMP_PORT_ADDR_ID){
        rv = SOC_E_FULL;
        goto mem_write_exit;
    } else if (port_id == DRV_EGR_V2V_IMP_PORT_ADDR_ID){
        port_id = DRV_EGR_V2V_IMP_PORT_SEARCH_ID;
    }
    
    /* 1. per port containing up to 1~255 entries.
     * 2. the counter number might large enough to force SW to process at next 
     *      port's entryies.
     * 3. read process will be stopped if the reading count over the max entry.
     */
    for (i = 0, temp_id = index; i < count; i++ ) {
        
        /* set user data */        
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, EGRESS_VID_RMK_TBL_DATAr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, EGRESS_VID_RMK_TBL_DATAr);

        reg_value = 0;
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_OP, 
             (uint32 *)v2v_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_set)(unit, EGRESS_VID_RMK_TBL_DATAr, 
             &reg_value, OUTER_OPf, &temp);
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_VID, 
             (uint32 *)v2v_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_set)(unit, EGRESS_VID_RMK_TBL_DATAr, 
             &reg_value, OUTER_VIDf, &temp);
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_OP, 
             (uint32 *)v2v_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_set)(unit, EGRESS_VID_RMK_TBL_DATAr, 
             &reg_value, INNER_OPf, &temp);
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_VID, 
             (uint32 *)v2v_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_set)(unit, EGRESS_VID_RMK_TBL_DATAr, 
             &reg_value, INNER_VIDf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_write_exit;
        }

        /* ---- writing process ---- */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, EGRESS_VID_RMK_TBL_ACSr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, EGRESS_VID_RMK_TBL_ACSr);
        
        /* set index key(port) */
        reg_value = 0;
        temp = port_id;
        if (SOC_IS_ROBO53115(unit)) {
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                EGRESS_PORT_Rf, &temp);
        } else {
            /* BCM53125 */
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                INDEX(EGRESS_PORTf), &temp);
        }
        /* set index key(index) */
        temp = temp_id;
        (DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                TBL_ADDRf, &temp);
                
        /* set read/write OP */
        temp = 1;   /* 1 means write OP in this table */
        (DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                OPf, &temp);
        
        /* Start to write */
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                START_DONEf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_write_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                    (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value, 
                    START_DONEf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_write_exit;
        }
        

        v2v_mem++;
        
        /* ---- index to next for count assignment ---- */
        temp_id++ ;
        if (temp_id >= DRV_EGRESS_V2V_NUM_PER_PORT){
            if (port_id == DRV_EGR_V2V_IMP_PORT_SEARCH_ID){
                rv = SOC_E_FULL;
                goto mem_write_exit;
            } else {
                port_id = ((port_id + 1) < DRV_EGR_V2V_IMP_PORT_ADDR_ID) ? 
                            (port_id + 1) : DRV_EGR_V2V_IMP_PORT_SEARCH_ID;
                temp_id = 0;
            }
        }
    }
    
mem_write_exit:     
    return rv;
}


/*
 *  Function : _drv_vlan_mem_read
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
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
_drv_bcm53115_vlan_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int     rv = SOC_E_NONE;
    int     i, index;
    uint32  retry;
    uint32  reg_len, reg_addr, temp, reg_value;
    vlan_1q_entry_t *vlan_mem = 0;

    vlan_mem = (vlan_1q_entry_t *)entry;

    for (i = 0;i < count; i++ ) {        
        index = entry_id + i;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_ADDRr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_ADDRr);

        /* fill index */
        reg_value = 0;
        temp = index;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_ADDRr, &reg_value, VTBL_ADDR_INDEXf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        /* read control setting */                
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_RWCTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_RWCTRLr);

        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        temp = BCM53115_MEM_OP_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_RW_CLRf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_STDNf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_read_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }

        /* get result */        
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_ENTRYr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_ENTRYr);

        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_VTBL_ENTRYr, 
            &reg_value, MSPT_INDEXf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_SPT_GROUP_ID, 
                 (uint32 *)vlan_mem, &temp);
        
        (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_VTBL_ENTRYr, 
            &reg_value, FWD_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP, 
            (uint32 *)vlan_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_VTBL_ENTRYr, 
            &reg_value, UNTAG_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG, 
            (uint32 *)vlan_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_VTBL_ENTRYr, 
            &reg_value, FWD_MODEf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE, 
            (uint32 *)vlan_mem, &temp);
        
        vlan_mem++;
    }

mem_read_exit:     
    return rv;
}

/*
 *  Function : _drv_bcm53115_vlan_mem_write
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the write result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53115_vlan_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32  reg_len, reg_addr, reg_value;
    uint32  retry, temp;
    vlan_1q_entry_t *vlan_mem = 0;

    vlan_mem = (vlan_1q_entry_t *)entry;

    for (i = 0;i < count; i++ ) {
        index = entry_id + i;

        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_ENTRYr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_ENTRYr);

       reg_value = 0;
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_SPT_GROUP_ID, 
             (uint32 *)vlan_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_set)(unit, ARLA_VTBL_ENTRYr, 
             &reg_value, MSPT_INDEXf, &temp);
       
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP, 
             (uint32 *)vlan_mem, &temp);           
        (DRV_SERVICES(unit)->reg_field_set)(unit, ARLA_VTBL_ENTRYr, 
             &reg_value, FWD_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG, 
             (uint32 *)vlan_mem, &temp);            
        (DRV_SERVICES(unit)->reg_field_set)(unit, ARLA_VTBL_ENTRYr, 
             &reg_value, UNTAG_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE, 
             (uint32 *)vlan_mem, &temp);            
        (DRV_SERVICES(unit)->reg_field_set)(unit, ARLA_VTBL_ENTRYr, 
             &reg_value, FWD_MODEf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_write_exit;
        }

        /* fill index */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_ADDRr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_ADDRr);
        reg_value = 0;
        temp = index;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_ADDRr, &reg_value, VTBL_ADDR_INDEXf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_write_exit;
        }

        /* read control setting */                
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_RWCTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_RWCTRLr);

        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_write_exit;
        }

        temp = BCM53115_MEM_OP_WRITE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_RW_CLRf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_STDNf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_write_exit;
        }
        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_write_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_write_exit;
        }

            vlan_mem ++;
    }

mem_write_exit:     
    return rv;
}


/*
 *  Function : _drv_vlan_mem_clear
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
_drv_bcm53115_vlan_mem_clear(int unit)
{
    int rv = SOC_E_NONE;
    uint32 retry;
    uint32  reg_len, reg_addr, temp, reg_value;


    /* read control setting */                
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_VTBL_RWCTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, ARLA_VTBL_RWCTRLr);

    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        goto vlan_mem_clear_exit;
    }

    temp = BCM53115_MEM_OP_CLEAR;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_RW_CLRf, &temp);

    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_STDNf, &temp);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        goto vlan_mem_clear_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto vlan_mem_clear_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_STDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto vlan_mem_clear_exit;
    }

vlan_mem_clear_exit:     
    return rv;
}

/*
 *  Function : _drv_mstp_mem_read
 *
 *  Purpose :
 *      Get the width of mstp memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
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
_drv_bcm53115_mstp_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32  reg_len, reg_addr;

    for (i = 0;i < count; i++ ) {        
        index = entry_id + i;
#if defined(BCM_53125) || defined(BCM_53128)
        if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MST_TABr, 0, index);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MST_TABr);
        } else {
#else
        {
#endif        
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MST_TBLr, 0, index);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MST_TBLr);
        }
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, entry, reg_len)) < 0) {
            goto mem_read_exit;
        }
        entry++;
    }

mem_read_exit:     
    return rv;
}

/*
 *  Function : _drv_bcm53115_mstp_mem_write
 *
 *  Purpose :
 *      Get the width of mstp memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the write result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53115_mstp_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32  reg_len, reg_addr;

    for (i = 0;i < count; i++ ) {
        index = entry_id + i;
#if defined(BCM_53125) || defined(BCM_53128)
        if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MST_TABr, 0, index);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MST_TABr);
        } else {
#else
        {
#endif
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MST_TBLr, 0, index);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MST_TBLr);
        }
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, entry, reg_len)) < 0) {
            goto mem_write_exit;
        }
        entry++;
    }

mem_write_exit:     
    return rv;
}

/*
 *  Function : _drv_bcm53115_mem_delete_all
 *
 *  Purpose :
 *      Delete all arl entries by fast aging all ports. 
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
_drv_bcm53115_arl_mem_clear(int unit, int mcast)
{
    uint32            reg_len, reg_addr, reg_value;
    uint32            temp, count;
    int rv = SOC_E_NONE;


        /* start fast aging process */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FAST_AGE_CTRLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FAST_AGE_CTRLr, 0, 0);
        reg_value = 0;

        /* Fast aging static and dynamic entries */
        temp = 1;    
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGE_CTRLr, &reg_value, 
                EN_FAST_AGE_STATICf, &temp));
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGE_CTRLr, &reg_value, 
                EN_AGE_DYNAMICf, &temp));
        if (mcast) {
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGE_CTRLr, &reg_value, 
                EN_AGE_MCASTf, &temp));
        }

        /* Start Aging */
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGE_CTRLr, &reg_value, 
                FAST_AGE_STR_DONEf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, FAST_AGE_CTRLr, &reg_value, 
                    FAST_AGE_STR_DONEf, &temp));
            if (!temp) {
                break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            return rv;
        }
    /*
     * Restore register value to 0x2, otherwise aging will fail
     */
    reg_value = 0x2;
    (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len);

    return rv;
}

int
_drv_bcm53115_cfp_read(int unit, uint32 mem, 
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
                "_drv_bcm53115_cfp_read(mem=0x%x,entry_id=0x%x)\n",
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
                "_drv_bcm53115_cfp_read(mem=0x%x,entry_id=0x%x)\n",
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
                "_drv_bcm53115_cfp_read(mem=0x%x,entry_id=0x%x)\n",
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
                "_drv_bcm53115_cfp_read(mem=0x%x,entry_id=0x%x)\n",
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
                "_drv_bcm53115_cfp_read(mem=0x%x,entry_id=0x%x)\n",
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
_drv_bcm53115_cfp_write(int unit, uint32 mem, 
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
                "_drv_bcm53115_cfp_read(mem=0x%x,entry_id=0x%x)\n",
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
                "_drv_bcm53115_cfp_write(mem=0x%x,entry_id=0x%x)\n",
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
                "_drv_bcm53115_cfp_write(mem=0x%x,entry_id=0x%x)\n",
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
                "_drv_bcm53115_cfp_write(mem=0x%x,entry_id=0x%x)\n",
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
                "_drv_bcm53115_cfp_read(mem=0x%x,entry_id=0x%x)\n",
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
                "_drv_bcm53115_cfp_read(mem=0x%x,entry_id=0x%x)\n",
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
_drv_bcm53115_cfp_field_get(int unit, uint32 mem, uint32 field_index, 
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
            "_drv_bcm53115_cfp_field_get(mem=0x%x,field=0x%x)\n",
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
            "_drv_bcm53115_cfp_field_get(mem=0x%x,field=0x%x)\n",
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
            "_drv_bcm53115_cfp_field_get(mem=0x%x,field=0x%x)\n",
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
            "_drv_bcm53115_cfp_field_get(mem=0x%x,field=0x%x)\n",
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
_drv_bcm53115_cfp_field_set(int unit, uint32 mem, uint32 field_index, 
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
            "_drv_bcm53115_cfp_field_set(mem=0x%x,field=0x%x)\n",
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
            "_drv_bcm53115_cfp_field_set(mem=0x%x,field=0x%x)\n",
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
            "_drv_bcm53115_cfp_field_set(mem=0x%x,field=0x%x)\n",
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
            "_drv_bcm53115_cfp_field_set(mem=0x%x,field=0x%x)\n",
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


#if defined(BCM_53125) || defined(BCM_53128)
int
_drv_bcm53125_8051_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int     i, index;
    uint32  retry;
    uint32  temp, reg_value;
    uint64  mem_data;
    int_8051_ram_entry_t    *mem_8051 = (int_8051_ram_entry_t *)entry;

    MEM_LOCK(unit, GEN_MEMORYm);
    if ( (rv = REG_READ_MEM_CTRLr(unit, &reg_value)) < 0) {
        goto mem_8051_read_exit;
    }

    if (mem == DRV_MEM_8051_RAM) {
        temp = BCM53125_8051_RAM_MEMORY;
    } else if (mem == DRV_MEM_8051_ROM) {
        /* 8051 ROM */
        temp = BCM53125_8051_ROM_MEMORY;
    } else {
        rv = SOC_E_UNAVAIL;
        goto mem_8051_read_exit;
    }
    soc_MEM_CTRLr_field_set(unit, &reg_value, MEM_TYPEf, &temp);

    if ( (rv = REG_WRITE_MEM_CTRLr(unit, &reg_value)) < 0) {
        goto mem_8051_read_exit;
    }
      

    for (i = 0;i < count; i++ ) {        
        index = entry_id + i;

        /* fill index */
        reg_value = 0;
        temp = index;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_ADRf, &temp);
        /* READ operation */
        temp  = BCM53115_MEM_OP_READ;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_RWf, &temp);
        /* Kick off */
        temp = 1;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_STDNf, &temp);
        
        if ( (rv = REG_WRITE_MEM_ADDRr(unit, &reg_value)) < 0) {
            goto mem_8051_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ( (rv = REG_READ_MEM_ADDRr(unit, &reg_value)) < 0) {
                goto mem_8051_read_exit;
            }
            soc_MEM_ADDRr_field_get(unit, &reg_value, 
                MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_8051_read_exit;
        }

        /* get result */
        COMPILER_64_ZERO(mem_data);
        if ( (rv = REG_READ_MEM_DEBUG_DATA_0_0r(
            unit, (uint32 *)&mem_data)) < 0) {
            goto mem_8051_read_exit;
        }
        sal_memcpy(mem_8051, &mem_data, sizeof(int_8051_ram_entry_t));
        
        mem_8051++;
    }

mem_8051_read_exit:    
    MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;
}

int
_drv_bcm53125_8051_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int     i, index;
    uint32  retry;
    uint32  temp, reg_value;
    uint64  mem_data;
    int_8051_ram_entry_t    *mem_8051 = (int_8051_ram_entry_t *)entry;

    MEM_LOCK(unit, GEN_MEMORYm);
    if ( (rv = REG_READ_MEM_CTRLr(unit, &reg_value)) < 0) {
        goto mem_8051_write_exit;
    }

    if (mem == DRV_MEM_8051_RAM) {
        temp = BCM53125_8051_RAM_MEMORY;
     } else {
        rv = SOC_E_UNAVAIL;
        goto mem_8051_write_exit;
    }
    soc_MEM_CTRLr_field_set(unit, &reg_value, MEM_TYPEf, &temp);

    if ( (rv = REG_WRITE_MEM_CTRLr(unit, &reg_value)) < 0) {
        goto mem_8051_write_exit;
    }
      

    for (i = 0;i < count; i++ ) {   
        COMPILER_64_ZERO(mem_data);
        sal_memcpy(&mem_data, mem_8051, sizeof(int_8051_ram_entry_t));
        if ( (rv = REG_WRITE_MEM_DEBUG_DATA_0_0r(
            unit, (uint32 *)&mem_data)) < 0) {
            goto mem_8051_write_exit;
        }
        
        index = entry_id + i;

        /* fill index */
        reg_value = 0;
        temp = index;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_ADRf, &temp);
        /* READ operation */
        temp  = BCM53115_MEM_OP_WRITE;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_RWf, &temp);
        /* Kick off */
        temp = 1;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_STDNf, &temp);
        
        if ( (rv = REG_WRITE_MEM_ADDRr(unit, &reg_value)) < 0) {
            goto mem_8051_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ( (rv = REG_READ_MEM_ADDRr(unit, &reg_value)) < 0) {
                goto mem_8051_write_exit;
            }
            soc_MEM_ADDRr_field_get(unit, &reg_value, 
                MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_8051_write_exit;
        }
        
        mem_8051++;
    }

mem_8051_write_exit:    
    MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;
}


#endif /* BCM_53125 */

/*
 *  Function : drv_bcm53115_mem_length_get
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
drv_bcm53115_mem_length_get(int unit, uint32 mem, uint32 *data)
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
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            meminfo = &SOC_MEM_INFO(unit, EGRESS_VID_REMARKm);
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_GEN:
        meminfo = &SOC_MEM_INFO(unit, GEN_MEMORYm);
        break;
    case DRV_MEM_VLANVLAN:
    default:
        return SOC_E_PARAM;
    }

  *data = meminfo->index_max - meminfo->index_min + 1;

  return SOC_E_NONE;
}


/*
 *  Function : drv_bcm53115_mem_width_get
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
drv_bcm53115_mem_width_get(int unit, uint32 mem, uint32 *data)
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
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            *data = sizeof(egress_vid_remark_entry_t);
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_GEN:
        *data = sizeof(gen_memory_entry_t);
        break;
    case DRV_MEM_VLANVLAN:
    default:
        return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}


 /*
 *  Function : drv_bcm53115_mem_read
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
drv_bcm53115_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    
    int rv = SOC_E_NONE;
    int i;
    uint32 retry, index_min, index_max, index;
    uint32 mem_id = 0;
    uint32 acc_ctrl = 0;
    uint32    temp;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint32 reg_addr;
    int reg_len;

    soc_cm_debug(DK_MEM, 
        "drv_mem_read(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
    switch (mem)
    {
    case DRV_MEM_ARL:
    case DRV_MEM_ARL_HW:
    case DRV_MEM_MARL:
        mem_id = L2_ARLm;
        break;
    case DRV_MEM_VLAN:
        mem_id = VLAN_1Qm;
        break;
    case DRV_MEM_MSTP:
        mem_id = MSPT_TABm;
        break;
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            mem_id = EGRESS_VID_REMARKm;
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
    case DRV_MEM_CFP_STAT_IB:
    case DRV_MEM_CFP_STAT_OB:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            rv = _drv_bcm53115_cfp_read(
                unit, mem, entry_id, count, entry);
        } else {
            rv = SOC_E_UNAVAIL;
        }
        return rv;
    case DRV_MEM_8051_RAM:
    case DRV_MEM_8051_ROM:
#if defined(BCM_53125) || defined(BCM_53128)
        if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
            _drv_bcm53125_8051_mem_read(
                unit, mem, entry_id, count, entry);
        } else {
            rv = SOC_E_UNAVAIL;
        }
#else   /* !BCM53125 */
        rv = SOC_E_UNAVAIL;
#endif /* BCM_53125 */
        return rv;
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_MCAST:
    case DRV_MEM_SECMAC:
    case DRV_MEM_GEN:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }

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
    /* add code here to check addr */
    if (mem_id == L2_ARLm ) {        
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, MEM_CTRLr, 0, 0), &acc_ctrl, 1)) < 0) {
            goto mem_read_exit;
        }


        temp = BCM53115_ARL_MEMORY;

        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_CTRLr, &acc_ctrl, MEM_TYPEf, &temp);
        
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, MEM_CTRLr, 0, 0), &acc_ctrl, 1)) < 0) {
            goto mem_read_exit;}
    } else if (mem_id ==VLAN_1Qm) {
        rv = _drv_bcm53115_vlan_mem_read(unit, entry_id, count, entry);
        goto mem_read_exit;
    } else if (mem_id ==EGRESS_VID_REMARKm) {
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            rv = _drv_bcm53115_egr_v2v_mem_read(unit, entry_id, count, entry);
        } else {
            rv =  SOC_E_UNAVAIL;
        }
        goto mem_read_exit;
    } else {
        rv = _drv_bcm53115_mstp_mem_read(unit, entry_id, count, entry);
        goto mem_read_exit;
    }

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

        /* Read memory control register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, MEM_ADDRr, 0, 0);
        reg_len = DRV_SERVICES(unit)->reg_length_get(unit,MEM_ADDRr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            goto mem_read_exit;
        }
        temp = BCM53115_MEM_OP_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_RWf, &temp);

        index = entry_id + i;
        temp = index / 2;
        /* Set memory entry address */
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_ADRf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            goto mem_read_exit;}

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_STDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            goto mem_read_exit;}

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
                goto mem_read_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, MEM_ADDRr, &acc_ctrl, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }

        /* Read the current generic memory entry */
        if ((index % 2) == 0) {
            /* Read bin 0 entry */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                    (unit, MEM_DEBUG_DATA_0_0r, 0, 0);
            reg_len = DRV_SERVICES(unit)->reg_length_get(unit, MEM_DEBUG_DATA_0_0r);

            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, gmem_entry, reg_len)) < 0) {
                goto mem_read_exit;
            }
            gmem_entry++;
            /* Read bin 1 entry */
            if (++i < count) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                    (unit, MEM_DEBUG_DATA_0_1r, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit,MEM_DEBUG_DATA_0_1r);

                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, gmem_entry, reg_len)) < 0) {
                    goto mem_read_exit;
                }
                gmem_entry++;    
            }
        } else {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                    (unit, MEM_DEBUG_DATA_0_1r, 0, 0);
            reg_len = DRV_SERVICES(unit)->reg_length_get(unit,MEM_DEBUG_DATA_0_1r);

            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, gmem_entry, reg_len)) < 0) {
                goto mem_read_exit;
            }
            gmem_entry++;  
        }
        

    }

 mem_read_exit:
     MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;

}

 /*
 *  Function : drv_bcm53115_mem_write
 *
 *  Purpose :
 *      Writes an internal memory.
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
drv_bcm53115_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max;
    uint32 i;
    uint32 mem_id = 0;
    uint32 acc_ctrl = 0;
    uint32    temp, index;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint32 reg_addr;
    int reg_len;

    soc_cm_debug(DK_MEM, "drv_mem_write(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
         
    switch (mem)
    {
    case DRV_MEM_ARL:
    case DRV_MEM_ARL_HW:
    case DRV_MEM_MARL:
        mem_id = L2_ARLm;
        break;
    case DRV_MEM_VLAN:
        mem_id = VLAN_1Qm;
        break;
    case DRV_MEM_MSTP:
        mem_id = MSPT_TABm;
        break;
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            mem_id = EGRESS_VID_REMARKm;
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
    case DRV_MEM_CFP_STAT_IB:
    case DRV_MEM_CFP_STAT_OB:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            rv = _drv_bcm53115_cfp_write(
                unit, mem, entry_id, count, entry);
        } else {
            rv = SOC_E_UNAVAIL;
        }
        return rv;
    case DRV_MEM_8051_RAM:
#if defined(BCM_53125) || defined(BCM_53128)
        if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
            _drv_bcm53125_8051_mem_write(
                unit, mem, entry_id, count, entry);
        } else {
            rv = SOC_E_UNAVAIL;
        }
#else   /* !BCM53125 */
        rv = SOC_E_UNAVAIL;
#endif /* BCM_53125 */
        return rv;
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_GEN:
    case DRV_MEM_MCAST:
    case DRV_MEM_SECMAC:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }

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
    if (mem_id == L2_ARLm ){        
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, MEM_CTRLr, 0, 0), &acc_ctrl, 1)) < 0) {
            goto mem_write_exit;
        }
        temp = BCM53115_ARL_MEMORY;

        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_CTRLr, &acc_ctrl, MEM_TYPEf, &temp);
        
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, MEM_CTRLr, 0, 0), &acc_ctrl, 1)) < 0) {
            goto mem_write_exit;}
    } else if (mem_id == VLAN_1Qm){
        rv = _drv_bcm53115_vlan_mem_write(unit, entry_id, count, entry);
        goto mem_write_exit;    
    } else if (mem_id ==EGRESS_VID_REMARKm) {
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            rv = _drv_bcm53115_egr_v2v_mem_write(unit, entry_id, count, entry);
        } else {
            rv =  SOC_E_UNAVAIL;
        }
        goto mem_write_exit;
    } else {
        rv = _drv_bcm53115_mstp_mem_write(unit, entry_id, count, entry);
        goto mem_write_exit;
    }
    
    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, GEN_MEMORYm);
            return SOC_E_PARAM;
        }

        /* write data */
        index = entry_id + i;
        if ((index % 2) == 0) {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, MEM_DEBUG_DATA_0_0r, 0, 0);
            reg_len = DRV_SERVICES(unit)->reg_length_get(unit, MEM_DEBUG_DATA_0_0r);

            if ((rv =(DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, gmem_entry, reg_len)) < 0) {
                goto mem_write_exit;
            }
            if (++i < count) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, MEM_DEBUG_DATA_0_1r, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit,MEM_DEBUG_DATA_0_1r);

                if ((rv =(DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, ++gmem_entry, reg_len)) < 0) {
                    goto mem_write_exit;
                }
            }
        } else {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, MEM_DEBUG_DATA_0_1r, 0, 0);
            reg_len = DRV_SERVICES(unit)->reg_length_get(unit,MEM_DEBUG_DATA_0_1r);

            if ((rv =(DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, gmem_entry, reg_len)) < 0) {
                goto mem_write_exit;
            }
        }
        
        /* set memory address */
        temp = index / 2;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, MEM_ADDRr, 0, 0);
        reg_len = DRV_SERVICES(unit)->reg_length_get(unit,MEM_ADDRr);
        
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_ADRf, &index);

        temp = BCM53115_MEM_OP_WRITE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_RWf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            goto mem_write_exit;
        }

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_STDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            goto mem_write_exit;}

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
                goto mem_write_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, MEM_ADDRr, &acc_ctrl, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_write_exit;
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

 mem_write_exit:
     MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;
    
}

/*
 *  Function : drv_bcm53115_mem_field_get
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
drv_bcm53115_mem_field_get(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{

    soc_mem_info_t    *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int            mem_id, field_id;
    int            i, wp, bp, len;
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
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(MULTCAST_PORTMAPf);
            } else {
                field_id = INDEX(PORTID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(TCf);
            } else {    
                field_id = INDEX(PRIORITY_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VIDf);
            } else {
                field_id = INDEX(VID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VALIDf);
            } else {
                INDEX(field_id = VALID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(MULTCAST_PORTMAPf);
            } else {
                INDEX(field_id = RESERVED_Rf);
            }
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_ARL:
        mem_id = L2_ARL_SWm;
        if (field_index == DRV_MEM_FIELD_MAC) {
            field_id = INDEX(MACADDRf);
        }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(PORTIDf);
            } else {
                field_id = INDEX(PORTID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(TCf);
            } else {    
                field_id = INDEX(PRIORITY_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VIDf);
            } else {
                field_id = INDEX(VID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VALIDf);
            } else {
                INDEX(field_id = VALID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_MARL:
        mem_id = L2_MARL_SWm;
        if (field_index == DRV_MEM_FIELD_MAC) {
            field_id = INDEX(MACADDRf);            
        }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
            field_id =INDEX(PORTBMPf);
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(TCf);
            } else {    
                field_id = INDEX(PRIORITY_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VIDf);
            } else {
                field_id = INDEX(VID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VALIDf);
            } else {
                INDEX(field_id = VALID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
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
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            mem_id = EGRESS_VID_REMARKm;;
            if (field_index == DRV_MEM_FIELD_OUTER_OP) {
                field_id = OUTER_OPf;
            }else if (field_index == DRV_MEM_FIELD_OUTER_VID) {
                field_id = OUTER_VIDf;
            }else if (field_index == DRV_MEM_FIELD_INNER_OP) {
                field_id = INNER_OPf;
            }else if (field_index == DRV_MEM_FIELD_INNER_VID) {
                field_id = INNER_VIDf;
            }else {
                return SOC_E_PARAM;
            }
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            return _drv_bcm53115_cfp_field_get
                (unit, mem, field_index, entry, fld_data);
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_PROTOCOLVLAN:
        return SOC_E_UNAVAIL;
    case DRV_MEM_MCAST:
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
    } else if ((mem_id == L2_ARLm)  || (mem_id == MAC2VLANm)) {
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
    } else if ((mem_id == L2_ARLm) || (mem_id == MAC2VLANm)) {
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
    
    return SOC_E_NONE;
}

 /*
 *  Function : drv_bcm53115_mem_field_set
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
drv_bcm53115_mem_field_set(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t    *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int            mem_id, field_id;
    int            i, wp, bp, len;
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
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(MULTCAST_PORTMAPf);
            } else {
                field_id = INDEX(PORTID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(TCf);
            } else {    
                field_id = INDEX(PRIORITY_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VIDf);
            } else {
                field_id = INDEX(VID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VALIDf);
            } else {
                INDEX(field_id = VALID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(MULTCAST_PORTMAPf);
            } else {
                INDEX(field_id = RESERVED_Rf);
            }
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_ARL:
        mem_id = L2_ARL_SWm;
        if (field_index == DRV_MEM_FIELD_MAC) {
            field_id = INDEX(MACADDRf);
        }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(PORTIDf);
            } else {
                field_id = INDEX(PORTID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(TCf);
            } else {    
                field_id = INDEX(PRIORITY_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VIDf);
            } else {
                field_id = INDEX(VID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VALIDf);
            } else {
                INDEX(field_id = VALID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_MARL:
        mem_id = L2_MARL_SWm;
        if (field_index == DRV_MEM_FIELD_MAC) {
            field_id = INDEX(MACADDRf);            
        }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
            field_id =INDEX(PORTBMPf);
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(TCf);
            } else {    
                field_id = INDEX(PRIORITY_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VIDf);
            } else {
                field_id = INDEX(VID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                field_id = INDEX(VALIDf);
            } else {
                INDEX(field_id = VALID_Rf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
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
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            mem_id = EGRESS_VID_REMARKm;
            if (field_index == DRV_MEM_FIELD_OUTER_OP) {
                field_id = OUTER_OPf;
            }else if (field_index == DRV_MEM_FIELD_OUTER_VID) {
                field_id = OUTER_VIDf;
            }else if (field_index == DRV_MEM_FIELD_INNER_OP) {
                field_id = INNER_OPf;
            }else if (field_index == DRV_MEM_FIELD_INNER_VID) {
                field_id = INNER_VIDf;
            }else {
                return SOC_E_PARAM;
            }
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            return _drv_bcm53115_cfp_field_set
                (unit, mem, field_index, entry, fld_data);
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_PROTOCOLVLAN:
        return SOC_E_UNAVAIL;
    case DRV_MEM_MCAST:
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
#ifdef BE_HOST
    if (mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if ((mem_id == L2_ARLm) || (mem_id == MAC2VLANm)) {
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
    } else if ((mem_id == L2_ARLm) || (mem_id == MAC2VLANm)) {
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
    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm53115_mem_clear
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
drv_bcm53115_mem_clear(int unit, uint32 mem)
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
        rv = _drv_bcm53115_arl_mem_clear(unit, 0);
        return rv;
        break;
    case DRV_MEM_MARL:
        rv = _drv_bcm53115_arl_mem_clear(unit, 1);
        return rv;
        break;
    case DRV_MEM_VLAN:
        rv = _drv_bcm53115_vlan_mem_clear(unit);
        return rv;
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) {
            _drv_bcm53115_egr_v2v_mem_clear(unit);
            return rv;
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_MSTP:
        mem_id = MSPT_TABm;
        break;
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_PROTOCOLVLAN:
        return SOC_E_UNAVAIL;
    case DRV_MEM_MCAST:
    case DRV_MEM_GEN:
    case DRV_MEM_SECMAC:
    default:
        return SOC_E_PARAM;
    }
   
    count = soc_mem_index_count(unit, mem_id);

    /* Prevent the Coverity Check(multiple sizeof(uint32))*/
    entry = (uint32 *)sal_alloc((sizeof(_soc_mem_entry_null_zeroes) / 
            sizeof(uint32)) * sizeof(uint32),
            "null_entry");
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
 *  Function : drv_bcm53115_mem_search
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
drv_bcm53115_mem_search(int unit, uint32 mem, 
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
    case DRV_MEM_GEN:
    case DRV_MEM_SECMAC:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }

    rv = _drv_bcm53115_mem_search(unit, mem, key, 
        entry, entry_1, flags, &index);
    
    return rv;

    
}

/*
 *  Function : drv_bcm53115_mem_insert
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
drv_bcm53115_mem_insert(int unit, uint32 mem, uint32 *entry, 
        uint32 flags)
{
    int                 rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t        output, output1;
    uint32            temp, count, mem_id;
    uint64            entry_reg;
    uint8            mac_addr[6];
    uint64               mac_field, mac_field_output, mac_field_entry;
    uint32            vid, control, vid_output, vid_entry;
    int             index, reg_macvid, reg_fwd, reg_len;
    uint32          mcast_pbmp, reg32;

    soc_cm_debug(DK_MEM, "drv_mem_insert : mem=0x%x, flags = 0x%x\n",
         mem, flags);
    switch(mem) {
    case DRV_MEM_ARL:
    case DRV_MEM_MARL:
        mem_id = L2_ARL_SWm;
        break;
    case DRV_MEM_VLAN:
    case DRV_MEM_MSTP:
    case DRV_MEM_MCAST:
    case DRV_MEM_GEN:
    case DRV_MEM_SECMAC:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }
    MEM_LOCK(unit,L2_ARLm);
    /* search entry */
    sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
    if ((rv =  _drv_bcm53115_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            (uint32 *)&output1, flags, &index))< 0) {
        if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS))
            goto mem_insert_exit;              
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
        }
    }

    /* write entry */
    /* form entry */
    /* VLAN ID */
    if ((rv = (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid)) < 0) {
        goto mem_insert_exit;
    }
    if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
        SOC_IS_ROBO53128(unit)) {
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_MACVID_ENTRY0r, (uint32 *)&entry_reg, 
            INDEX(VIDf), &vid);
    } else {
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_MACVID_ENTRY0r, (uint32 *)&entry_reg, VID_Rf, &vid);
    }

    /* MAC Address */
    if ((rv = (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            entry, (uint32 *)&mac_field)) < 0) {
        goto mem_insert_exit;
    }
    SAL_MAC_ADDR_FROM_UINT64(mac_addr, mac_field);
        
    (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_MACVID_ENTRY0r, (uint32 *)&entry_reg, 
            ARL_MACADDRf, (uint32 *)&mac_field);

    reg32 = 0;
    if (mac_addr[0] & 0x01) { /* The input is the mcast address */
        /* multicast group index */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, entry, &mcast_pbmp);
        temp = mcast_pbmp;
        if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
            SOC_IS_ROBO53128(unit)) {
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY0r, &reg32, INDEX(PORTIDf), &temp);
        } else {
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY0r, &reg32, PORTID_Rf, &temp);
        }
    } else { /* unicast address */
        /* source port id */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
        if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
            SOC_IS_ROBO53128(unit)) {
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY0r, &reg32, INDEX(PORTIDf), &temp);
        } else {
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY0r, &reg32, PORTID_Rf, &temp);
        }
    }

     /* age */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp);
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_FWD_ENTRY0r, &reg32, ARL_AGEf, &temp);
    /* priority0 */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, entry, &temp);
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_FWD_ENTRY0r, &reg32, ARL_PRIf, &temp);
    /* ARL_CON */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_FWD_ENTRY0r, &reg32, ARL_CONf, &temp);            
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
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_FWD_ENTRY0r, &reg32, ARL_STATICf, &temp);
    /* valid bit */
    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_FWD_ENTRY0r, &reg32, ARL_VALIDf, &temp);

    /* write ARL and MAC/VID entry register*/
    switch(index) {
    case 0:
        reg_macvid = ARLA_MACVID_ENTRY0r;
        reg_fwd = ARLA_FWD_ENTRY0r;
        break;
    case 1:
        reg_macvid = ARLA_MACVID_ENTRY1r;
        reg_fwd = ARLA_FWD_ENTRY1r;
        break;
    case 2:
        reg_macvid = ARLA_MACVID_ENTRY2r;
        reg_fwd = ARLA_FWD_ENTRY2r;
        break;
    case 3:
        reg_macvid = ARLA_MACVID_ENTRY3r;
        reg_fwd = ARLA_FWD_ENTRY3r;
        break;
    default:
        rv =  SOC_E_PARAM;
        goto mem_insert_exit;
    }
    
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_fwd);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, reg_macvid, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
        goto mem_insert_exit;
    }
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, reg_fwd, 0, 0), &reg32, reg_len)) < 0) {
        goto mem_insert_exit;
    }

    /* Write ARL Read/Write Control Register */
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
        goto mem_insert_exit;
    }
    temp = BCM53115_MEM_OP_WRITE;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &control, ARL_RWf, &temp);
    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
        goto mem_insert_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            goto mem_insert_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto mem_insert_exit;
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
 *  Function : drv_bcm53115_mem_delete
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
drv_bcm53115_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int         rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t   output, output1;
    uint32      entry_reg;
    uint32      temp, count, mem_id;
    uint32      control;
    int         index;
    uint32      reg_len, reg_addr, reg_value;
    uint32      ag_port_mode = 0, ag_vlan_mode = 0;
    uint32      ag_static_mode = 0;
    uint32      reg_fwd;
    uint32      src_port= 0, vlanid = 0;


    soc_cm_debug(DK_MEM, "drv_mem_delete : mem=0x%x, flags = 0x%x)\n",
         mem, flags);
    switch(mem) {
    case DRV_MEM_ARL:
    case DRV_MEM_MARL:
        mem_id = L2_ARL_SWm;
        break;
    case DRV_MEM_VLAN:
    case DRV_MEM_MSTP:
    case DRV_MEM_MCAST:
    case DRV_MEM_GEN:
    case DRV_MEM_SECMAC:
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_MACVLAN:
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
        return SOC_E_UNAVAIL;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_STATIC) {
        ag_static_mode = 1;
    }
    
    if ((ag_port_mode) || (ag_vlan_mode)) {
        if (ag_port_mode) {
        /* aging port mode */
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, FAST_AGE_PORTr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, FAST_AGE_PORTr, 0, 0);
            reg_value = 0;
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
               goto mem_delete_exit;
            }            
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
            src_port = temp;
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGE_PORTr, &reg_value, AGE_PORTf, &temp));

            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
        }  
        if (ag_vlan_mode) {
        /* aging vlan mode */
            temp = 0;
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, FAST_AGE_VIDr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, FAST_AGE_VIDr, 0, 0);
            reg_value = 0;

            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &temp);
            vlanid = temp;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGE_VIDr, &reg_value, AGE_VIDf, &temp));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
        }

        /* start fast aging process */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FAST_AGE_CTRLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FAST_AGE_CTRLr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_delete_exit;
        }

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, FAST_AGE_CTRLr, &reg_value, EN_AGE_PORTf, &ag_port_mode));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, FAST_AGE_CTRLr, &reg_value, EN_AGE_VLANf, &ag_vlan_mode));
        if ((mem == DRV_MEM_MARL) && (!ag_port_mode)) {
            temp = 1;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, FAST_AGE_CTRLr, &reg_value, 
                    EN_AGE_MCASTf, &temp));
        }

        /* Fast aging static and dynamic entries */
        if (ag_static_mode) {
            temp = 1;    
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, FAST_AGE_CTRLr, &reg_value, 
                    EN_FAST_AGE_STATICf, &temp));
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, FAST_AGE_CTRLr, &reg_value, 
                    EN_AGE_DYNAMICf, &temp));
        } else {
            temp = 0;    
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, FAST_AGE_CTRLr, &reg_value, 
                    EN_FAST_AGE_STATICf, &temp));
            temp = 1;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, FAST_AGE_CTRLr, &reg_value, 
                    EN_AGE_DYNAMICf, &temp));
        }

        temp = 1;
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGE_CTRLr, &reg_value, 
                FAST_AGE_STR_DONEf, &temp));
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
                    (unit, FAST_AGE_CTRLr, &reg_value, 
                    FAST_AGE_STR_DONEf, &temp));
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
        soc_cm_debug(DK_MEM, 
                    "\t Normal ARL DEL with Static=%d\n",
                    ag_static_mode);

        MEM_LOCK(unit,L2_ARLm);
        /* search entry */
        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
        if ((rv =  _drv_bcm53115_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            (uint32 *)&output1, flags, &index))< 0){
            if((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
                if (rv == SOC_E_FULL) {
                /* For mem_delete if mem_search return SOC_E_FULL it means
                  * the entry is not found. */
                    rv = SOC_E_NOT_FOUND;
                }
                goto mem_delete_exit;              
            }
        }

        /* write entry */
        if (rv == SOC_E_EXISTS) {
            /* clear the VALID bit*/
            temp = 0;
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit,ARLA_FWD_ENTRY0r);
            switch(index) {
                case 0:
                    reg_fwd = ARLA_FWD_ENTRY0r;
                    break;
                case 1:
                    reg_fwd = ARLA_FWD_ENTRY1r;
                    break;
                case 2:
                    reg_fwd = ARLA_FWD_ENTRY2r;
                    break;
                case 3:
                    reg_fwd = ARLA_FWD_ENTRY3r;
                    break;
                default:
                    rv =  SOC_E_PARAM;
                    goto mem_delete_exit;
            }
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, reg_fwd, 0, 0), &entry_reg, reg_len)) < 0){
                goto mem_delete_exit;
            }
            /* check static bit if set */
            if (!ag_static_mode){
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                                (unit, ARLA_FWD_ENTRY0r, (uint32 *)&entry_reg, 
                                ARL_STATICf, &temp));
                if (temp) {
                    soc_cm_debug(DK_MEM, 
                                "\t Entry exist with static=%d\n",
                                temp);
                    rv = SOC_E_NOT_FOUND;
                    goto mem_delete_exit;
                }
            }
            
            temp = 0;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, reg_fwd, &entry_reg, ARL_VALIDf, &temp));

            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, reg_fwd, 0, 0), &entry_reg, reg_len)) < 0){
                goto mem_delete_exit;
            }

            /* Write ARL Read/Write Control Register */
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
                goto mem_delete_exit;
            }
            temp = BCM53115_MEM_OP_WRITE;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_RWCTLr, &control, ARL_RWf, &temp));
            temp = 1;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
                goto mem_delete_exit;
            }

            /* wait for complete */
            for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
                    goto mem_delete_exit;
                }
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
                if (!temp) {
                    break;
                }
            }

            if (count >= SOC_TIMEOUT_VAL) {
                rv = SOC_E_TIMEOUT;
                goto mem_delete_exit;
            }

            sw_arl_update = 1;
        }
    }

mem_delete_exit:
    if (((ag_port_mode) || (ag_vlan_mode) )) {
        if(ag_static_mode){
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, FAST_AGE_CTRLr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, FAST_AGE_CTRLr, 0, 0);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            temp = 0;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGE_CTRLr, &reg_value, EN_FAST_AGE_STATICf, &temp));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
        }
    }else {
        MEM_UNLOCK(unit,L2_ARLm);
    }
    /*
     * Restore register value to 0x2, otherwise aging will fail
     */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, FAST_AGE_CTRLr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, FAST_AGE_CTRLr, 0, 0);
    reg_value = 0x2;
    (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len);

    if (sw_arl_update){
        /* Remove the entry from sw database */
        _drv_arl_database_delete(unit, index, &output);
    }

    return rv;

    
}
