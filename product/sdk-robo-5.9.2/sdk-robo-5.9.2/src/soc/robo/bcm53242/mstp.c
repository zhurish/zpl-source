/*
 * $Id: mstp.c,v 1.7 Broadcom SDK $
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
#include <soc/drv.h>
#include <soc/debug.h>
#include  <bcm/types.h>
/*
 *  Function : drv_bcm53242_mstp_port_set
 *
 *  Purpose :
 *      Set the port state of a selected stp id.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mstp_gid : multiple spanning tree id.
 *      port    :   port number.
 *      port_state  :   state of the port.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_mstp_port_set(int unit, uint32 mstp_gid, uint32 port, uint32 port_state)
{
    uint32          shift, wd = 0;
    mspt_tab_entry_t    mstp_entry;
    int rv = SOC_E_NONE,reg_name;
    uint32  reg_len, reg_addr;
    uint32 reg_value,reg_field;
    uint32 max_gid;
    uint64          *data_ptr, temp;

    soc_cm_debug(DK_STP, 
    "drv_mstp_port_set : unit %d, STP id = %d, port = %d, port_state = %d \n",
         unit, mstp_gid, port, port_state);

    (DRV_SERVICES(unit)->dev_prop_get)
        (unit, DRV_DEV_PROP_MSTP_NUM, &max_gid);

    if (!soc_feature(unit, soc_feature_mstp)){
        /* error checking */
        if (mstp_gid != BCM_STG_DEFAULT) {
            return SOC_E_PARAM;
        }
        if(IS_GE_PORT(unit, port)) {
            reg_name = G_PCTLr;
            reg_field = G_MISTP_STATEf;
        } else {
            reg_name = TH_PCTLr;
            reg_field = MISTP_STATEf;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, reg_name, port, 0);

        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, reg_name);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }    

        switch (port_state)
        {
            case DRV_PORTST_DISABLE:
                COMPILER_64_SET(temp, 0x0, 0x1);
                break;
            case DRV_PORTST_BLOCK:
                COMPILER_64_SET(temp, 0x0, 0x2);
                break;
            case DRV_PORTST_LISTEN:
                COMPILER_64_SET(temp, 0x0, 0x3);
                break;
            case DRV_PORTST_LEARN:
                COMPILER_64_SET(temp, 0x0, 0x4);
                break;
            case DRV_PORTST_FORWARD:
                COMPILER_64_SET(temp, 0x0, 0x5);
                break;
            default:
                return SOC_E_PARAM;
        }
    

        (DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_name, &reg_value, reg_field, (uint32 *)&temp);  

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
    
        return rv;

    } else {
        /* error checking */
        if (mstp_gid > max_gid) {
            return SOC_E_PARAM;
        }

        mstp_gid = mstp_gid % max_gid;
        sal_memset(&mstp_entry, 0, sizeof(mstp_entry));

        /* write mstp id to vlan entry */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_read)
            (unit, DRV_MEM_MSTP, mstp_gid, 1, (uint32 *)&mstp_entry));

        /* 
         * Because the memory field services didn't contain port information, 
         * we can't access the port state by port
         */
        if (port < 16) {
            shift = 3 * port + 76;  /* skip length of RESERVED bit */
        } else {                    /* TABLE_DATA0[63:0], TABLE_DATA1[11:0] */
            shift = 3 * port + 80;  /* skip length of RESERVED bit TABLE_DATA0[63:0], */
        }                           /* TABLE_DATA1[11:0], TABLE_DATA1[63:60] */

        data_ptr = (uint64 *)&mstp_entry;

        wd = (shift / 64);
        shift -= (shift / 64) * 64;
        COMPILER_64_SET(temp, 0x0, 0x7);
        COMPILER_64_SHL(temp, shift);
        COMPILER_64_NOT(temp);

        COMPILER_64_AND(*(data_ptr+wd), temp);
        switch (port_state)
        {
            case DRV_PORTST_DISABLE:
                COMPILER_64_SET(temp, 0x0, 0x1);
                break;
            case DRV_PORTST_BLOCK:
                COMPILER_64_SET(temp, 0x0, 0x2);
                break;
            case DRV_PORTST_LISTEN:
                COMPILER_64_SET(temp, 0x0, 0x3);
                break;
            case DRV_PORTST_LEARN:
                COMPILER_64_SET(temp, 0x0, 0x4);
                break;
            case DRV_PORTST_FORWARD:
                COMPILER_64_SET(temp, 0x0, 0x5);
                break;
            default:
                return SOC_E_PARAM;
        }
        COMPILER_64_SHL(temp, shift);
        COMPILER_64_OR(*(data_ptr+wd), temp);

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_write)
            (unit, DRV_MEM_MSTP, mstp_gid, 1, (uint32 *)&mstp_entry));
        return SOC_E_NONE;
    }
}

/*
 *  Function : drv_bcm53242_mstp_port_get
 *
 *  Purpose :
 *      Get the port state of a selected stp id.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mstp_gid : multiple spanning tree id.
 *      port    :   port number.
 *      port_state  :   state of the port.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */

int 
drv_bcm53242_mstp_port_get(int unit, uint32 mstp_gid, uint32 port, uint32 *port_state)
{
    uint32          portstate;
    mspt_tab_entry_t    mstp_entry;
    int rv = SOC_E_NONE, reg_name;
    uint32  reg_len, reg_addr, reg_value,reg_field;
    uint32 max_gid;
    uint32          shift, wd = 0;
    uint64          *data_ptr, temp;

    (DRV_SERVICES(unit)->dev_prop_get)
        (unit, DRV_DEV_PROP_MSTP_NUM, &max_gid);

    if (!soc_feature(unit, soc_feature_mstp)){

        /* error checking */
        if (mstp_gid != BCM_STG_DEFAULT) {
            return SOC_E_PARAM;
        }

        if(IS_GE_PORT(unit, port) ){
            reg_name = G_PCTLr;
            reg_field = G_MISTP_STATEf;
        } else {
            reg_name = TH_PCTLr;
            reg_field = MISTP_STATEf;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, reg_name, port, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, reg_name);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_name, &reg_value, reg_field, &portstate); 
    
        switch (portstate)
        {
            case 1:
                *port_state = DRV_PORTST_DISABLE;
                break;
            case 2:
                *port_state = DRV_PORTST_BLOCK;
                break;
            case 3:
                *port_state = DRV_PORTST_LISTEN;
                break;
            case 4:
                *port_state = DRV_PORTST_LEARN;
                break;
            case 5:
                *port_state = DRV_PORTST_FORWARD;
                break;
            default:
            return SOC_E_INTERNAL;
        }
        soc_cm_debug(DK_STP, 
            "drv_mstp_port_get : unit %d, STP id = %d, port = %d, port_state = %d \n",
             unit, mstp_gid, port, *port_state);

        return SOC_E_NONE;

    }else{
        /* error checking */
        if (mstp_gid > max_gid) {
            return SOC_E_PARAM;
        }
        mstp_gid = mstp_gid % max_gid;

        /* write mstp id to vlan entry */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_read)
            (unit, DRV_MEM_MSTP, mstp_gid, 1, (uint32 *)&mstp_entry));

        /* 
         * Because the memory field services didn't contain port information, 
         * we can't access the port state by port
         */
        if (port < 16) {
            shift = 3 * port + 76;  /* skip length of RESERVED bit */
        } else {                    /* TABLE_DATA0[63:0], TABLE_DATA1[11:0] */
            shift = 3 * port + 80;  /* skip length of RESERVED bit TABLE_DATA0[63:0], */
        }                           /* TABLE_DATA1[11:0], TABLE_DATA1[63:60] */
        
        data_ptr = (uint64 *)&mstp_entry;

        wd = (shift / 64);
        shift -= (shift / 64) * 64;
        COMPILER_64_SET(temp, 0x0, 0x7);
        COMPILER_64_SHR(*(data_ptr+wd), shift);
        COMPILER_64_AND(*(data_ptr+wd), temp);
        COMPILER_64_TO_32_LO(portstate, *(data_ptr+wd));
        switch (portstate)
        {
            case 1:
                *port_state = DRV_PORTST_DISABLE;
                break;
            case 2:
                *port_state = DRV_PORTST_BLOCK;
                break;
            case 3:
                *port_state = DRV_PORTST_LISTEN;
                break;
            case 4:
                *port_state = DRV_PORTST_LEARN;
                break;
            case 5:
                *port_state = DRV_PORTST_FORWARD;
                break;
            default:
                return SOC_E_INTERNAL;
        }
        soc_cm_debug(DK_STP, 
        "drv_mstp_port_get : unit %d, STP id = %d, port = %d, port_state = %d \n",
             unit, mstp_gid, port, *port_state);

        return SOC_E_NONE;
    }
}
