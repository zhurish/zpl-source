/*
 * $Id: vlan.c,v 1.27 Broadcom SDK $
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
#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include "robo_53115.h"

#define VT_SPVID_MASK       0xfff
#define VT_OPMODE_MASK      0x1

/* Vulcan's Double Tagging and new Egress VLAN translation designing  :
 *  1. Tow double tagging modes. (DT_Mode and iDT_Mode)
 *      - in DT_Mode, the VLAN table should be in charge of the mission to 
 *          untag the outer when egress to none-ISP port.
 *      - in iDT_Mode, the untag job when egress to the none-ISP port should 
 *          be programmed in Engress VLAN Remark(EVR) table.
 *  2. both ISP and None-ISP ports can recogenize the S-Tag in the ingress 
 *      side when the devivce at double tagging mode.
 *  3. EVR table can only be work when device at iDT_Mode.
 *  4. No Ingress basis VLAN translation feature in Vulcan.
 *  5. Egress basis VLAN translation is designed as :
 *      - Mapping mode : only outer tag in egress frame.
 *      - Transparent mode : both outer and inner tags in egress frame.
 *  6. Only CFP action can drive a legal clssification-id to indicate a proper 
 *      EVR table entry. (means EVR table can't be work without CFP setting.)
 */

/* ------- egress port basis VLAN translation DataBase ---------- */
/* drv_vlan_egr_vt_info_t :
 *  - a sw table to represent the user configured EVR table (binding with CFP 
 *      table). And this sw table will be formed as port basis structure with 
 *      ori_vid sorted on each specific egress port.
 */
typedef struct drv_vlan_egr_vt_info_s {
    uint16  ori_vid;            /* original VID(entry key) */
    uint16  new_vid;            /* new VID */
    int     vt_mode;            /* transparent/mapping mode */
    int     prev_id;            /* pointer to previous index(init=-1) */
    int     next_id;            /* pointer to next index(init=-1) */
    /* prev_id & next_id :
     *  1. head node >> prev = -1
     *      a. if node_count == 1, next = DRV_EGRESS_V2V_NUM_PER_PORT
     *      b. if node_count > 1, next = n(the next node id)
     *  2. last node >> next = DRV_EGRESS_V2V_NUM_PER_PORT
     *      a. if node_count == 1, prev = -1
     *      b. if node_count > 1, prev = n(the previous node id)
     *  3. other node, prev = 0~DRV_EVRID_VT_SW_MAX_CNT, and
     *                  next = 0~DRV_EVRID_VT_SW_MAX_CNT
     */
    
    /* this is for port based implementation, not used for global basis */
    int     evr_id;     /* Egress VLAN Remark table index(0 is invalid)*/
    
} drv_vlan_egr_vt_info_t;

typedef struct drv_egr_vt_db_info_s {
    int     start_id;   /* log the first valid id (init = -1)*/
    int     isp_port;   /* 1 is isp port, 0 is non-isp port (init= -1) */
    int     count;      /* log the valid count in DB entry (init = 0)*/
    drv_vlan_egr_vt_info_t    egr_vt_info[DRV_EVRID_VT_SW_MAX_CNT];
}drv_egr_vt_db_info_t;
drv_egr_vt_db_info_t  egr_vt_db[SOC_MAX_NUM_PORTS];

int new_evr_vt_entry_id = 0;    /* read-clear for return to BCM layer */

#define IS_VALID_EGRVT_DB_ENTRY_ID(_id)    \
                ((_id) >= 0 && (_id) < DRV_EVRID_VT_SW_MAX_CNT)
#define EGRVT_DB_ID_TO_REAL_ID(_db_id)    \
                ((_db_id) + DRV_EVRID_VT_ACTION_FISRT)
#define EGRVT_REAL_ID_TO_DB_ID(_real_id)    \
                ((_real_id) - DRV_EVRID_VT_ACTION_FISRT)
                
#define IS_PORT_EGRVT_EMPTY(_p)    \
                (egr_vt_db[(_p)].count == 0)
#define IS_PORT_EGRVT_FULL(_p)    \
                (egr_vt_db[(_p)].count == DRV_EVRID_VT_SW_MAX_CNT)

#define EVR_SWDB_OP_FLAG_INSERT  1
#define EVR_SWDB_OP_FLAG_DELETE  2
#define EVR_SWDB_OP_FLAG_RESET   3

/* global var for 
 *  1. indicating the double tagging mode (none/ DT_MODE/ iDT_MODE)
 *  2. log the CFP for supporting EVR group and entry initial status.
 *      - this is a run-time init flag. 
 *      - Field bcm API was init after VLAN API so here we designed as a 
 *          runtime init mechanism.
 */
int device_dtag_mode = DRV_VLAN_DT_MODE_DISABLE;

#define IS_DRV_EVR_DB_NODE_FREE(_p, _i)     \
            ((((egr_vt_db[(_p)].egr_vt_info) + (_i)).prev_id == -1) && \
            (((egr_vt_db[(_p)].egr_vt_info) + (_i)).next_id == -1))
#define IS_DRV_EVR_DB_FULL     \
            ((egr_vt_db[(_p)].count) == DRV_EVRID_VT_SW_MAX_CNT)
        
#define DRV_EVR_VID_MASK        0xFFF

#define DRV_EVR_OP_FLAG_MASK    0x0F
#define DRV_EVR_OP_FLAG_ASIS    0x01
#define DRV_EVR_OP_FLAG_ASRX    0x02
#define DRV_EVR_OP_FLAG_REMOVE  0x04
#define DRV_EVR_OP_FLAG_MODIFY  0x08

#define IS_EVR_OP_VALID(_op)    \
            (((_op) & DRV_EVR_OP_FLAG_MASK) > 0)
#define EVT_OP_FLAG_TO_OP_VALUE(_op)    \
            ((((_op) == DRV_EVR_OP_FLAG_ASIS) ? 0 : (   \
                (((_op) == DRV_EVR_OP_FLAG_ASRX) ? 1 : (    \
                (((_op) == DRV_EVR_OP_FLAG_REMOVE) ? 2 : 3))))))


/*
 *  Function : _drv_bcm53115_evr_vt_sw_search
 *      - search if a entry with given port+vid is existed.
 *
 *  Parameter :
 *      - entry_id  : the target entry_id(sw) if existed. the most close
 *                  entry_id(sw) if not existed.
 *      - free_id   : the first found free entry_id(sw). if no entry been free
 *                  this value will = DRV_EVRID_VT_SW_MAX_CNT.
 *                  (the free_entry.next and free_entry.prev will be "-1")
 *  Return :
 *      TRUE(1)     : search result is existed.
 *      FALSE(0)    : search result is not existed. 
 *  Note : 
 *  1. return 0(False) is not found. and 
 *      a. entry_id = DRV_EVRID_VT_SW_MAX_CNT when the table on this port
 *          is full. else
 *      b. entry_id = (valid entry index) to point to the most close item in 
 *          this sorted table on this port. The real case for the search 
 *          result might be one of below:
 *          - (entry_id).ori_vid > vid(not full yet and this entry_id 
 *              indicating the fisrt one entry within vid large than given vid
 *          - (entry_id).ori_vid < vid(not full yet and all exist entries' vid
 *              are smaller than vid. 
 *      c. entry_id = -1. no entry created on this port.
 *  2. if entry is found, the entry_id indicate the match one(port+vid). 
 *
 */
int _drv_bcm53115_evr_vt_sw_search(int unit, uint32 port, 
                uint16 vid, int *entry_id, int *free_id){
    
    int     evr_sw_db_head;
    int     i, found = FALSE;
    uint16  temp_vid;
    
    drv_vlan_egr_vt_info_t  *temp_evr_sw_db_entry;
    
    /* for bcm53115, this routine is port based DB search */
    assert(port < SOC_ROBO_MAX_NUM_PORTS);
    evr_sw_db_head = egr_vt_db[port].start_id;
    
    /* check if VID is valid */
    if (vid < 1 || vid > 4094){
        soc_cm_debug(DK_VLAN, "%s: invalid VID=%d\n", FUNCTION_NAME(), vid);
        return FALSE;
    }
    
    /* check if sw_db is empty */
    if (egr_vt_db[port].count == 0){
        *entry_id = -1;
        *free_id = 0;
        return FALSE;
    }
    
    for (i = evr_sw_db_head; i < DRV_EVRID_VT_SW_MAX_CNT; 
                    i = temp_evr_sw_db_entry->next_id){
        
        temp_evr_sw_db_entry = egr_vt_db[port].egr_vt_info + i;
        *entry_id = i;
        
        temp_vid = temp_evr_sw_db_entry->ori_vid;
        if (temp_vid == vid){
            found = TRUE;
            break;
        } else if(temp_vid > vid) {
            found = FALSE;
            break;
        }
    }
    
    /* get the free entry index */
    *free_id = -1;
    for (i = 0; i < DRV_EVRID_VT_SW_MAX_CNT; i++){
        temp_evr_sw_db_entry = egr_vt_db[port].egr_vt_info + i;
        /* next = -1 and prev = -1 means this node is free */
        if ((temp_evr_sw_db_entry->next_id == -1) && 
                    (temp_evr_sw_db_entry->prev_id == -1)){
            *free_id = i;
            break;
        }
    }
    
    /* check if sw_db is full */
    if (found){
        return TRUE;
    } else {
        if (egr_vt_db[port].count == DRV_EVRID_VT_SW_MAX_CNT){
            *entry_id = DRV_EVRID_VT_SW_MAX_CNT;
            *free_id = DRV_EVRID_VT_SW_MAX_CNT;
        }
        return FALSE;
    }
    
}
  
/*
 *  Function : _drv_bcm53115_evr_vt_sw_db_update
 *      - update the EVR sw database for different operation.
 *  Parmeter :
 *      op      :   insert | delete | reset
 *      port    :   port
 *      vid     :   vid
 *      vt_mode :   mapping | trasparent
 *      fast_id :   the most closed index
 *      this_id :   the operating index
 *      
 *  Note : 
 *
 */
void _drv_bcm53115_evr_vt_sw_db_update(int op, uint32 port, 
                    uint16  ori_vid, uint16  new_vid, int vt_mode,
                    int fast_id, int this_id){
    int temp_id;
    
    /* check valid port */
    if (port >= SOC_ROBO_MAX_NUM_PORTS){
        soc_cm_debug(DK_ERR, "%s: invalid port=%d\n", FUNCTION_NAME(), port);
        return;   
    }
    
    if (!(IS_VALID_EGRVT_DB_ENTRY_ID(fast_id))){
        if (fast_id != -1) {
            soc_cm_debug(DK_ERR, "%s: invalid case on fast_id \n", FUNCTION_NAME());
            return;
        }
    }

    if (!(IS_VALID_EGRVT_DB_ENTRY_ID(this_id))){
        soc_cm_debug(DK_ERR, "%s: invalid case on this_id \n", FUNCTION_NAME());
        return;   
    }
    
    switch(op){
    case EVR_SWDB_OP_FLAG_INSERT :
        if (fast_id == -1){     /* means the first node */
            egr_vt_db[port].start_id = this_id;
            egr_vt_db[port].egr_vt_info[this_id].prev_id = -1;
            egr_vt_db[port].egr_vt_info[this_id].next_id = 
                            DRV_EVRID_VT_SW_MAX_CNT;
        } else {
            /* insert to the front of fast_id node */
            if (egr_vt_db[port].egr_vt_info[fast_id].ori_vid > ori_vid){
                /* check if head */
                if (egr_vt_db[port].egr_vt_info[fast_id].prev_id == -1){
                    /* insert to the head */
                    egr_vt_db[port].start_id = this_id;
                    
                    egr_vt_db[port].egr_vt_info[this_id].prev_id = -1;
                    egr_vt_db[port].egr_vt_info[this_id].next_id = fast_id;
                    
                    egr_vt_db[port].egr_vt_info[fast_id].prev_id = this_id;
                } else {
                    /* insert to normal */
                    temp_id = egr_vt_db[port].egr_vt_info[fast_id].prev_id;
                    
                    egr_vt_db[port].egr_vt_info[temp_id].next_id = this_id;
                    
                    egr_vt_db[port].egr_vt_info[this_id].prev_id = temp_id;
                    egr_vt_db[port].egr_vt_info[this_id].next_id = fast_id;
                    
                    egr_vt_db[port].egr_vt_info[fast_id].prev_id = this_id;
                    
                }
            } else {    /* insert to the end of fast_id node */
                temp_id = egr_vt_db[port].egr_vt_info[fast_id].next_id;
            
                egr_vt_db[port].egr_vt_info[fast_id].next_id = this_id;
                
                egr_vt_db[port].egr_vt_info[this_id].prev_id = fast_id;
                egr_vt_db[port].egr_vt_info[this_id].next_id = temp_id;
                
                if (temp_id != DRV_EVRID_VT_SW_MAX_CNT){
                    /* this case should not be happened */
                    soc_cm_debug(DK_ERR, 
                            "%s: invalid case on fast_id=%d \n", 
                            FUNCTION_NAME(), fast_id);
                    egr_vt_db[port].egr_vt_info[temp_id].prev_id = this_id;
                }
            }
        }
        egr_vt_db[port].egr_vt_info[this_id].ori_vid = ori_vid;
        egr_vt_db[port].egr_vt_info[this_id].new_vid = new_vid;
        egr_vt_db[port].egr_vt_info[this_id].vt_mode = vt_mode;
        
        egr_vt_db[port].count ++;
        break;
    case EVR_SWDB_OP_FLAG_DELETE : 
        /* the first node */
        if (egr_vt_db[port].egr_vt_info[this_id].prev_id == -1){ 
            temp_id = egr_vt_db[port].egr_vt_info[this_id].next_id;
            egr_vt_db[port].start_id = temp_id;
            
            egr_vt_db[port].egr_vt_info[temp_id].prev_id = -1;
        
        /* the last node */
        } else if(egr_vt_db[port].egr_vt_info[this_id].next_id == 
                        -1){
            temp_id = egr_vt_db[port].egr_vt_info[this_id].prev_id;
            egr_vt_db[port].egr_vt_info[temp_id].next_id = -1;                
        
        /* normal node */
        } else {  
            temp_id = egr_vt_db[port].egr_vt_info[this_id].prev_id;
            egr_vt_db[port].egr_vt_info[temp_id].next_id = 
                        egr_vt_db[port].egr_vt_info[this_id].next_id;
                        
            temp_id = egr_vt_db[port].egr_vt_info[this_id].next_id;
            egr_vt_db[port].egr_vt_info[temp_id].prev_id = 
                        egr_vt_db[port].egr_vt_info[this_id].prev_id;
        }

        egr_vt_db[port].egr_vt_info[this_id].ori_vid = 0;
        egr_vt_db[port].egr_vt_info[this_id].new_vid = 0;
        egr_vt_db[port].egr_vt_info[this_id].vt_mode = 0;
        egr_vt_db[port].egr_vt_info[this_id].prev_id = -1;
        egr_vt_db[port].egr_vt_info[this_id].next_id = -1;
        
        egr_vt_db[port].count --;
        break;
    case EVR_SWDB_OP_FLAG_RESET :
        /* -------- TBD -------- */
        break;
    default :
        break;
    }
}
    

/*
 *  Function : _drv_bcm53115_evr_entry_set
 *      - to set a Egress VLAN Remark(EVR) table entry.
 *
 *  Parameter :
 *      - ent_id : must be read id on EVR table (not sw used DB_ID)
 *
 *  Note : 
 *      1. ent_id will be valid only in the pre-defined id range for 
 *          bcm53115 used rang. (check the robo_53115.h)
 */
int _drv_bcm53115_evr_entry_set(int unit, uint32 port, int ent_id, 
                    uint16 out_op, uint16 out_vid, 
                    uint16 in_op, uint16 in_vid){

    egress_vid_remark_entry_t   evr_ent;
    uint32  fld_val32;
    uint32  entry_addr;
    
    /* Check the validation on entry index */
    soc_cm_debug(DK_VLAN, "%s: port=%d,entry_id=%d,out_op=%d,in_op=%d\n", 
            FUNCTION_NAME(), port, ent_id, out_op, in_op);
    if ((ent_id < 0) || 
            (ent_id > (DRV_EVRID_VT_ACTION_FISRT + 
                    DRV_EVRID_VT_SW_MAX_CNT - 1))){
        soc_cm_debug(DK_ERR, "%s: invalid ent_id=%d\n", FUNCTION_NAME(), ent_id);
        return SOC_E_FAIL;
    }
    
    /* check valid op */
    if (!(IS_EVR_OP_VALID(out_op) && IS_EVR_OP_VALID(in_op))){
        soc_cm_debug(DK_ERR, "%s: invalid op flag! out_op=%d, in_op=%d\n", 
                    FUNCTION_NAME(), out_op,in_op);
        return SOC_E_PARAM;
    }
    
    /* ---- set default entry into EVR table ---- */
    sal_memset(&evr_ent, 0, sizeof (evr_ent));

    fld_val32 = EVT_OP_FLAG_TO_OP_VALUE(out_op);
    (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_OP,
                (uint32 *)&evr_ent, 
                (uint32 *)&fld_val32);
    fld_val32 = out_vid & DRV_EVR_VID_MASK;
    (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_VID,
                (uint32 *)&evr_ent, 
                (uint32 *)&fld_val32);
    fld_val32 = EVT_OP_FLAG_TO_OP_VALUE(in_op);
    (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_OP,
                (uint32 *)&evr_ent, 
                (uint32 *)&fld_val32);
    fld_val32 = in_vid & DRV_EVR_VID_MASK;
    (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_VID,
                (uint32 *)&evr_ent, 
                (uint32 *)&fld_val32);

    entry_addr = DRV_EGR_V2V_ENTRY_ID_BUILD(port, ent_id);
    (DRV_SERVICES(unit)->mem_write)
                    (unit, DRV_MEM_EGRVID_REMARK, entry_addr, 1, 
                    (uint32 *)&evr_ent);
    
    return SOC_E_NONE;
}
    
/*
 *  Function : _drv_bcm53115_evr_entry_get
 *      - to get a Egress VLAN Remark(EVR) table entry.
 *
 *  Parameter :
 *      - ent_id : must be read id on EVR table (not sw used DB_ID)
 *
 *  Note : 
 *      1. ent_id will be valid only in the pre-defined id range for 
 *          bcm53115 used rang. (check the robo_53115.h)
 */
int _drv_bcm53115_evr_entry_get(int unit, uint32 port, int ent_id, 
                uint16 *out_op, uint16 *out_vid, 
                uint16 *in_op, uint16 *in_vid){

    /* dummy currently! for no request on this routine */
    return SOC_E_NONE;
}
    
    
/*
 *  Function : _drv_bcm53115_evr_init
 *      - clear every entry on each port.
 *
 *  Note : 
 *  1. init mem and the sw database.
 *  2. this routine will operates when VLAN init.
 */
int _drv_bcm53115_evr_init(int unit){
    uint32  reg_addr, reg_value, temp;
    int     reg_len, rv, i, j;
    int     retry;
    uint16  t_outer_op, t_inner_op;
    int     port;
    soc_pbmp_t temp_bmp;

    /* Debug only, Have to Remove!! */
    soc_cm_debug(DK_VLAN, "%s: unit = %d\n", FUNCTION_NAME(), unit);
    /* clear EVR table 
     *  1. here we use the HW provied reset bit to speed the init progress.
     */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, EGRESS_VID_RMK_TBL_ACSr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, EGRESS_VID_RMK_TBL_ACSr, 0, 0);
    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value,
                RESET_EVTf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, EGRESS_VID_RMK_TBL_ACSr, &reg_value,
                START_DONEf, &temp));
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
    
    /* 1.1 assigning the default EVR entry on each port 
     *  - entry 0 on EVR table will be referenced on the iDT mode if no 
     *      CFP hit to drive a newClassID.
     *  a. normal egress port : {OuterOp(Removed),InnerOp(AsRx)}
     *      >> all ports default is none-isp port so the DefaultOp=Remove.
     *  b. CPU egress port : As default {OuterOp(AsIs),InnerOp(AsRx)}
     * 
     * Note :
     *  1. CFP action is bypast on the CPU egress direct packet when device 
     *     is at iDT mode. Such packet will use the ClassID=0 to reference 
     *     EVR table.
     */
    SOC_PBMP_ASSIGN(temp_bmp, PBMP_PORT_ALL(unit));
    SOC_PBMP_PORT_ADD(temp_bmp, CMIC_PORT(unit));
    SOC_PBMP_ITER(temp_bmp, port){ 
        if (IS_CPU_PORT(unit, port)){
            /* innerOp set to AsRx is the most reasonable for normal double 
             * tagging process. (The same behavior as other ROBO devices)
             */
            t_outer_op = DRV_EVR_OP_FLAG_ASIS;
            t_inner_op = DRV_EVR_OP_FLAG_ASRX;
        } else {
            t_outer_op = DRV_EVR_OP_FLAG_REMOVE;
            t_inner_op = DRV_EVR_OP_FLAG_ASRX;
        }
        rv = _drv_bcm53115_evr_entry_set(unit, port, DRV_EVRID_TAG_ACTION_DEF, 
                        t_outer_op, 0, t_inner_op, 0);
    }
    
    /* 2. -------- reset iDT_Mode ----------- */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, VLAN_CTRL4r);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, VLAN_CTRL4r, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, VLAN_CTRL4r, &reg_value, EN_DOUBLE_TAGf, &temp));
        
    temp &= ~DRV_VLAN_INTELLIGENT_DT_MODE;

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, VLAN_CTRL4r, &reg_value, EN_DOUBLE_TAGf, &temp));
        
    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    device_dtag_mode = temp;
   
    /* 2.1 -------- reset ISP port ----------- */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, ISP_SEL_PORTMAPr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, ISP_SEL_PORTMAPr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    /* CPU set to ISP port when init */
    reg_value |= (1 << CMIC_PORT(unit));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    /* assigning the isp port bitmap */
    SOC_PBMP_CLEAR(temp_bmp);
    SOC_PBMP_WORD_SET(temp_bmp, 0, reg_value);

    /* 3. -------- EVR database init ----------- */
    sal_memset(egr_vt_db, 0 , 
                sizeof(drv_egr_vt_db_info_t) * SOC_MAX_NUM_PORTS);
    
    /* assign the none zero initial value */
    for (i=0; i<SOC_MAX_NUM_PORTS; i++){
        egr_vt_db[i].start_id = -1;
        egr_vt_db[i].isp_port = SOC_PBMP_MEMBER(temp_bmp, i) ? TRUE : FALSE;
        for (j=0; j<DRV_EVRID_VT_SW_MAX_CNT; j++){
            egr_vt_db[i].egr_vt_info[j].prev_id = -1;
            egr_vt_db[i].egr_vt_info[j].next_id = -1;
        }
    }
    
mem_write_exit:     
    return rv;
}


/*
 *  Function : _drv_bcm53115_evr_vt_isp_change
 *      - maintain the EVR entry for VT and DT_mode action on those given 
 *          ports (dt_mode changed ports)
 *
 *  Note : 
 *  1. ISP port : The Outer tag action is "As Is"
 *  2. None-ISP port : The Outer tag action is "Remove"
 */
int _drv_bcm53115_evr_vt_isp_change(int unit, soc_pbmp_t changed_bmp){
    
    int     new_isp, vt_mode, port, evr_db_id, real_id, rv = 0;
    uint16  vt_new_vid;
    drv_vlan_egr_vt_info_t  *evr_sw_db_entry;
    uint16  t_outer_op, t_inner_op;
    
    /* set the default EVR table entry no matther the iDT or DT mode :
     *  - loop on each port in port bitmap.
     */
    SOC_PBMP_ITER(changed_bmp, port){
        /* CPU default entry won't be change */
        if (IS_CPU_PORT(unit, port)){
            continue;
        }
        
        SOC_IF_ERROR_RETURN(
                    (DRV_SERVICES(unit)->vlan_prop_port_enable_get)
                        (unit, DRV_VLAN_PROP_ISP_PORT, 
                         port,  (uint32 *) &new_isp));
        if (new_isp){   /* ISP port */
            t_outer_op = DRV_EVR_OP_FLAG_ASIS;
            t_inner_op = DRV_EVR_OP_FLAG_ASRX;
            
        } else {    /* None-ISP port */
            t_outer_op = DRV_EVR_OP_FLAG_REMOVE;
            t_inner_op = DRV_EVR_OP_FLAG_ASRX;
        }
        rv = _drv_bcm53115_evr_entry_set(unit, port, DRV_EVRID_TAG_ACTION_DEF, 
                        t_outer_op, 0, t_inner_op, 0);
    }
    
    /* check if iDT_Mode is enabled */
    if (!(device_dtag_mode & DRV_VLAN_INTELLIGENT_DT_MODE)){
        /* debug message */
        soc_cm_debug(DK_VLAN, 
            "EVR table can't work if iDT_Mode is not enabled!\n");
    }
    
    /* loop on each port in port bitmap and assigning new isp_mode action on 
     *  each port's VT entries.
     */
    SOC_PBMP_ITER(changed_bmp, port){
        
        /* check if EVR is empty */
        if (IS_PORT_EGRVT_EMPTY(port)){
            continue;
        }
        
        new_isp = egr_vt_db[port].isp_port;
        for (evr_db_id = egr_vt_db[port].start_id; 
                    evr_db_id < DRV_EVRID_VT_SW_MAX_CNT;
                    evr_db_id = evr_sw_db_entry->next_id){
                        
            evr_sw_db_entry = egr_vt_db[port].egr_vt_info + evr_db_id;
            vt_mode = evr_sw_db_entry->vt_mode;
            vt_new_vid = evr_sw_db_entry->new_vid;
            real_id = EGRVT_DB_ID_TO_REAL_ID(evr_db_id);
            
            if (new_isp){
                /* vt_mode : 1 is Mapping mode; 0 is transparent mode */
                if (vt_mode){ 
                    /* Action :
                     *  1. outer->Modify; 
                     *  2. inner->remove; 
                     *  3. outer/inner vid reassign 
                     */
                    rv = _drv_bcm53115_evr_entry_set(
                                unit, port, real_id, 
                                DRV_EVR_OP_FLAG_MODIFY, vt_new_vid, 
                                DRV_EVR_OP_FLAG_REMOVE, 0);
                    assert(rv == SOC_E_NONE);
                } else {
                    /* Action :
                     *  1. outer->Modify; 
                     *  2. inner->as_is; 
                     *  3. outer/inner vid reassign 
                     */
                    rv = _drv_bcm53115_evr_entry_set(
                                unit, port, real_id, 
                                DRV_EVR_OP_FLAG_MODIFY, vt_new_vid, 
                                DRV_EVR_OP_FLAG_ASIS, 0);
                }
            } else {    /* new none-isp */
            
                /* Action :
                 *  1. outer->Remove; 
                 *  2. inner->Modify; 
                 *  3. outer/inner vid reassign 
                 */
                rv = _drv_bcm53115_evr_entry_set(
                            unit, port, real_id, 
                            DRV_EVR_OP_FLAG_REMOVE, 0, 
                            DRV_EVR_OP_FLAG_MODIFY, vt_new_vid);
            }
        }
            
    }
    
    return rv;
}

/*
 *  Function : drv_vlan_vt_add
 *
 *  Purpose :
 *      Add the a specific VLAN translation entry.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      vt_type     :   VT table type. (ingress/egress/..) 
 *      port        :   port id. 
 *      cvid        :   customer vid(= inner_vid = old_vid)
 *      sp_vid      :   service provide vid( = outer_vid = new_vid)
 *      pri         :   priority (not used in ingress VT)
 *      mode        :   vt_mode (trasparent / mapping)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. bcm53115 support device basis VLAN translation though the 
 *          Egress VLAN Remark(EVR) table is egress port basis.
 *          - this is for the VLAN XLATE have to work with ingress basis 
 *              filetering (CFP).
 *      2. priority is not supported in EVR table.
 */
int 
drv_bcm53115_vlan_vt_add(int unit, uint32 vt_type, uint32 port,  uint32 cvid, 
                uint32 sp_vid, uint32 pri, uint32 mode)
{
    int     i, rv = SOC_E_NONE;
    int  search_id, free_id,real_id;
    int     temp_isp_mode;
    soc_pbmp_t  temp_bmp;

    soc_cm_debug(DK_VLAN, 
            "%s: vt_type=%d,port=%d,ori_vid=%d,new_vid=%d\n",
            FUNCTION_NAME(), vt_type, port, cvid, sp_vid);
    switch (vt_type){
    case DRV_VLAN_XLAT_EVR :
        /* in bcm53115, VLAN XLATE is ingress filtering but egress action */
                
        /* can't keep going if previous creating id still not been read */
        if (new_evr_vt_entry_id != 0){
            return SOC_E_BUSY;
        }
        
        /* 1. check the the port+vid is existed 
         *  - return if the entry is existed already.
         *  - else popup the proper sw entry id which is sorted by 
         *      vid in this port(keep the sw entry id).
         *  - hard coded the port = 0, for the VLAN XLATE in bcm53115 is 
         *      designed as device basis currently. That means all port on 
         *      the same entry id will serivce the same VLAN trnaslation.
         */
        if (_drv_bcm53115_evr_vt_sw_search(unit, 0, cvid, 
                        &search_id, &free_id)){     /* exist */
            soc_cm_debug(DK_VLAN, 
                "%s:EVR for port=%d,vid=%d is existed! Can't Add it!\n", 
                FUNCTION_NAME(), port, cvid);
    
            return SOC_E_EXISTS;
        } else {    /* not exist */
            /* check if full */
            if ((search_id == DRV_EVRID_VT_SW_MAX_CNT) ||
                        (free_id == DRV_EVRID_VT_SW_MAX_CNT)){
                soc_cm_debug(DK_VERBOSE | DK_VLAN, 
                    "%s:EVR for port=%d,vid=%d is existed! Can't Add it!\n", 
                    FUNCTION_NAME(), port, cvid);
                return SOC_E_FULL;
            }
            
        }
        
        /* 2. add the EVR entry on NNI and UNI ports */
        assert(IS_VALID_EGRVT_DB_ENTRY_ID(free_id));
        
        real_id = EGRVT_DB_ID_TO_REAL_ID(free_id);

        SOC_PBMP_ASSIGN(temp_bmp, PBMP_PORT_ALL(unit));
        SOC_PBMP_REMOVE(temp_bmp, PBMP_CMIC(unit));

        SOC_PBMP_ITER(temp_bmp, i){
            if (IS_CPU_PORT(unit, i)){
                soc_cm_debug(DK_ERR, "%s: EVR on CPU is not implemented\n", 
                        FUNCTION_NAME());
                continue;
            }

            temp_isp_mode = egr_vt_db[i].isp_port;
            sp_vid &= DRV_EVR_VID_MASK; 
            if (temp_isp_mode == TRUE){    /* set ISP port */
                /* mode : 1 is means Mapping mode; 0 is transparent mode */
                if (mode){  
                    /* outer -> Modify; inner -> remove */  
                    rv = _drv_bcm53115_evr_entry_set(
                                unit, i, real_id, 
                                DRV_EVR_OP_FLAG_MODIFY, sp_vid, 
                                DRV_EVR_OP_FLAG_REMOVE, 0);
                } else {
                    /* outer -> Modify; inner -> as_is */  
                    rv = _drv_bcm53115_evr_entry_set(
                                unit, i, real_id, 
                                DRV_EVR_OP_FLAG_MODIFY, sp_vid, 
                                DRV_EVR_OP_FLAG_ASIS, 0);
                }
                
                assert(rv == SOC_E_NONE);
                
            } else {    /* set none-ISP port */
                /* mode is not proper to set on UNI port  */
                /* outer -> remove; inner -> modify */  
                rv = _drv_bcm53115_evr_entry_set(
                            unit, i, real_id, 
                            DRV_EVR_OP_FLAG_REMOVE, 0, 
                            DRV_EVR_OP_FLAG_MODIFY, sp_vid);
            }
            assert(rv == SOC_E_NONE);
            
            /* 3. maintain port based sw database */
            _drv_bcm53115_evr_vt_sw_db_update(EVR_SWDB_OP_FLAG_INSERT, i,
                            cvid, sp_vid, mode, search_id, free_id);
        }
                
        /* keep the new created id and waiting for read-clear */
        new_evr_vt_entry_id = real_id;
        
        break;
    case DRV_VLAN_XLAT_INGRESS :
    case DRV_VLAN_XLAT_EGRESS:
        rv = SOC_E_UNAVAIL;
        break;
    default :
        rv = SOC_E_UNAVAIL;
        break;
    }
    
  return rv;
}

/*
 *  Function : drv_vlan_vt_delete
 *
 *  Purpose :
 *      Delete the a specific VLAN translation entry.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      vt_type     :   VT table type. (ingress/egress/..) 
 *      port        :   port id. (not used for ingress VT)
 *      vid         :   VLAN ID 
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. bcm53115 VLAN XLATE is designed as device basis.
 *      2. not port VT delete allowed in bcm53115.
 */
int 
drv_bcm53115_vlan_vt_delete(int unit, uint32 vt_type, uint32 port, uint32 vid)
{
    int     i, rv = SOC_E_NONE;
    int  search_id, free_id, real_id;
    drv_vlan_egr_vt_info_t  *temp_evt_sw_db_entry;
    soc_pbmp_t  temp_bmp;

    soc_cm_debug(DK_VLAN, 
            "%s: vt_type=%d,port=%d,vid=%d\n",
            FUNCTION_NAME(), vt_type, port, vid);
    switch (vt_type){
    case DRV_VLAN_XLAT_EVR :
        /* can't keep going if previous creating id still not been read */
        if (new_evr_vt_entry_id != 0){
            return SOC_E_BUSY;
        }
        
        /* 1. check the existence on the port+vid 
         *  - hard coded the port = 0, for the VLAN XLATE in bcm53115 is 
         *      designed as device basis currently. That means all port on 
         *      the same entry id will serivce the same VLAN trnaslation.
         */
        if ((_drv_bcm53115_evr_vt_sw_search(unit, 0, vid, 
                        &search_id, &free_id)) == FALSE){ 
            soc_cm_debug(DK_VLAN, 
                "%s:EVR for port=%d,vid=%d is not existed!\n", 
                FUNCTION_NAME(), port, vid);
    
            return SOC_E_NOT_FOUND;
        } else {    /* exist */
            if (!(IS_VALID_EGRVT_DB_ENTRY_ID(search_id))){
                soc_cm_debug(DK_VERBOSE | DK_VLAN, 
                    "%s:Unexcepted EVR search result!\n", FUNCTION_NAME());
                return SOC_E_INTERNAL;
            }
        }
        
        /* 2. remove EVR table : set all zero value to the entry */
        temp_evt_sw_db_entry = egr_vt_db[port].egr_vt_info + search_id;
        real_id = EGRVT_DB_ID_TO_REAL_ID(search_id); 

        SOC_PBMP_ASSIGN(temp_bmp, PBMP_PORT_ALL(unit));
        SOC_PBMP_REMOVE(temp_bmp, PBMP_CMIC(unit));

        SOC_PBMP_ITER(temp_bmp, i){
            rv = _drv_bcm53115_evr_entry_set(unit, i, real_id, 
                            DRV_EVR_OP_FLAG_ASIS, 0, 
                            DRV_EVR_OP_FLAG_ASIS, 0);
                            
            /* 3. maintain port based sw database */
            _drv_bcm53115_evr_vt_sw_db_update(EVR_SWDB_OP_FLAG_DELETE, i,
                            vid, 0, 0, 0, search_id);
        }

        break;
    case DRV_VLAN_XLAT_INGRESS :
    case DRV_VLAN_XLAT_EGRESS:
        
        rv = SOC_E_UNAVAIL;
        break;
    default :
        rv = SOC_E_UNAVAIL;
        break;
    }
    return rv;
}

/*
 *  Function : drv_vlan_vt_delete_all
 *
 *  Purpose :
 *      Delete all the a specific VLAN translation entry.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      vt_type     :   VT table type. (ingress/egress/..) 
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *
 */
int 
drv_bcm53115_vlan_vt_delete_all(int unit, uint32 vt_type)
{
    int rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, 
            "%s: vt_type=%d\n", FUNCTION_NAME(), vt_type);
    switch (vt_type){
    case DRV_VLAN_XLAT_EVR :
        
        /* no calling reference currently */
        rv = SOC_E_NONE;
        break;
    case DRV_VLAN_XLAT_INGRESS :
    case DRV_VLAN_XLAT_EGRESS:
        rv = SOC_E_UNAVAIL;
        break;
    default :
        rv = SOC_E_UNAVAIL;
        break;
    }
    return rv;
}

/*
 *  Function : drv_vlan_vt_set
 *
 *  Purpose :
 *      Set the VLAN translation property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      port         : port id. (not used for ingress VT)
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. if port_n = 0xffffffff (int = -1), means get device basis value.
 *      2. sp_tpid currently will be 0x8100 / 0x9100 / 0x9200 /???
 *
 */
int 
drv_bcm53115_vlan_vt_set(int unit, uint32 prop_type, uint32 vid, 
                                uint32 port, uint32 prop_val)
{
    uint32  field = 0, field_val32;
    uint32  reg_addr, reg_value;
    int     reg_len;
    int     rv = SOC_E_NONE;
    
    soc_cm_debug(DK_VLAN, 
            "%s: unit=%d, prop_type=%d,vid=%d,port=%d,val=%x\n",
            FUNCTION_NAME(), unit, prop_type, vid, port, prop_val);
    switch(prop_type){
    case DRV_VLAN_PROP_VT_MODE:      /*  trasparent / mapping */
        /* for bcm53115, no upper layer calling reference yet! */
        break;
    case DRV_VLAN_PROP_EGR_VT_SPVID:     /* egress SP VID */
    case DRV_VLAN_PROP_EGR_VT_PRI:
    case DRV_VLAN_PROP_ING_VT_SPVID:     /* ingress SP VID */
    case DRV_VLAN_PROP_ING_VT_PRI:
    case DRV_VLAN_PROP_EGR_VT_SPTPID:    /* egress SP TPID */
        rv = SOC_E_UNAVAIL;
        break;
    case DRV_VLAN_PROP_ING_VT_SPTPID:    /* ingress SP TPID */
        field = ISP_TPIDf;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, DTAG_TPIDr, port, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, DTAG_TPIDr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        
        field_val32 = prop_val;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, DTAG_TPIDr, &reg_value, ISP_TPIDf, &field_val32));
        
        break;
    default :
        rv = SOC_E_UNAVAIL;
        break;
    }

    return rv;
}

/*
 *  Function : drv_vlan_vt_get
 *
 *  Purpose :
 *      Get the VLAN translation property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      port        : port id. (not used for ingress VT)
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. if port_n = 0xffffffff (int = -1), means get device basis value.
 *      2. VLAN translation in Vulcan is designed as system basis.
 *
 */
int 
drv_bcm53115_vlan_vt_get(int unit, uint32 prop_type, uint32 vid, 
                                uint32 port, uint32 *prop_val)
{
    uint32  field = 0, field_val32;
    uint32  reg_addr, reg_value;
    int     reg_len;
    int     rv = SOC_E_NONE;
    uint32  vt_mode, new_vid;
    int  search_id, free_id ;
    drv_vlan_egr_vt_info_t  *temp_evt_sw_db_entry;

    soc_cm_debug(DK_VLAN, 
            "%s: unit=%d, prop_type=%d,vid=%d,port=%d\n",
            FUNCTION_NAME(), unit, prop_type, vid, port);

    switch(prop_type){
    case DRV_VLAN_PROP_ING_VT_SPTPID:
        field = ISP_TPIDf;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, DTAG_TPIDr, port, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, DTAG_TPIDr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, DTAG_TPIDr, &reg_value, ISP_TPIDf, &field_val32));
        *prop_val = field_val32;
        
        break;
    case DRV_VLAN_PROP_VT_MODE:      /*  trasparent / mapping */
    case DRV_VLAN_PROP_EGR_VT_SPVID:     /* egress SP VID */

        /* can't keep going if previous creating id still not been read */
        if (new_evr_vt_entry_id != 0){
            return SOC_E_BUSY;
        }
        
        /* 1. check the existence on the port+vid 
         *  - hard coded the port = 0, for the VLAN XLATE in bcm53115 is 
         *      designed as device basis currently. That means all port on 
         *      the same entry id will serivce the same VLAN trnaslation.
         */
        if ((_drv_bcm53115_evr_vt_sw_search(unit, 0, vid, 
                        &search_id, &free_id)) == FALSE){ 
            soc_cm_debug(DK_VLAN, 
                "%s:EVR for port=%d,vid=%d is not existed!\n", 
                FUNCTION_NAME(), port, vid);
    
            return SOC_E_NOT_FOUND;
        } else {    /* exist */
            if (!(IS_VALID_EGRVT_DB_ENTRY_ID(search_id))){
                soc_cm_debug(DK_VERBOSE | DK_VLAN, 
                    "%s:Unexcepted EVR search result!\n", FUNCTION_NAME());
                return SOC_E_INTERNAL;
            }
        }
        
        /* 2. get the searched VT_Mode */
        temp_evt_sw_db_entry = egr_vt_db[port].egr_vt_info + search_id;
        
        if (prop_type == DRV_VLAN_PROP_VT_MODE) {
            
            vt_mode = temp_evt_sw_db_entry->vt_mode;
            
            *prop_val = vt_mode;
        } else { /* prop_type == DRV_VLAN_PROP_EGR_VT_SPVID */
            
            new_vid = temp_evt_sw_db_entry->new_vid;
            
            *prop_val = new_vid;
        }
        break;
    case DRV_VLAN_PROP_EGR_VT_PRI:
    case DRV_VLAN_PROP_ING_VT_SPVID:     /* ingress SP VID */
    case DRV_VLAN_PROP_ING_VT_PRI:
    case DRV_VLAN_PROP_EGR_VT_SPTPID:    /* egress SP TPID */
    default :
        rv = SOC_E_UNAVAIL;
        break;
    }

    return rv;
}

/*
 *  Function : drv_vlan_prop_set
 *
 *  Purpose :
 *      Set the VLAN property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *  1. in bcm53115, double tagging mode can be DT_Mode(original design)
 *      or iDT_Mode(new design, means intelligent DT_Mode).
 *      - Our design for this feature are :
 *          a. user enable double tagging mode -> set to DT_Mode.
 *          b. user enable vlan translation -> set to iDT_Mode.
 *
 */
int 
drv_bcm53115_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32  reg_addr, reg_value, temp;
    int reg_len, rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, 
            "%s: unit = %d, property type = %d, value = %x\n",
            FUNCTION_NAME(), unit, prop_type, prop_val);
    switch (prop_type) {
    case DRV_VLAN_PROP_VTABLE_MISS_DROP:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL5r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL5r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
             return rv;
        }
        if (prop_val) {
           temp = 1;
        } else {
           temp = 0;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, VLAN_CTRL5r, &reg_value, DROP_VTABLE_MISSf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
             return rv;
        }
        break;
    case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL0r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        if (prop_val) {
           temp = 0;
        } else {
           temp = 3;
        }        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp));

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        break;            
    case DRV_VLAN_PROP_SP_TAG_TPID:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, DTAG_TPIDr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, DTAG_TPIDr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }

        temp = prop_val & 0xFFFF;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, DTAG_TPIDr, &reg_value, ISP_TPIDf, &temp));

        if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
            
        break;
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
    case DRV_VLAN_PROP_IDT_MODE_ENABLE:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, VLAN_CTRL4r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, VLAN_CTRL4r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }

        /* if prefer iDT_mode is set, the working flow here will be 
         *  - no DT_mode through SoC driver. 
         *
         * if prefer DT_mode is set, for the concern about VT feature, the 
         *  working flow must handle mode transfer between DT_mode and 
         *  iDT_mode when device DT enabled.
         *  - DT_mode is set when DT is enabled from disable.
         *  - iDT_mode is set internally only. Once the VT related process is
         *      requested, the iDT_mode will be set. 
         */ 
        if (DRV_VLAN_PREFER_DT_MODE == DRV_VLAN_INTELLIGENT_DT_MODE) {
            temp = (prop_val == TRUE) ? 
                    DRV_VLAN_INTELLIGENT_DT_MODE : DRV_VLAN_DT_MODE_DISABLE;
            
        } else {    /* DT_Mode */
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, VLAN_CTRL4r, &reg_value, EN_DOUBLE_TAGf, &temp));
            if (prop_type == DRV_VLAN_PROP_IDT_MODE_ENABLE){ /* iDT_mode */
                /* check current value */
                if (temp & DRV_VLAN_INTELLIGENT_DT_MODE){
                    if (prop_val){
                        /* return for nothing to change */
                        return SOC_E_NONE;
                    } else {
                        temp &= ~DRV_VLAN_INTELLIGENT_DT_MODE;
                    }
                } else {
                    if (prop_val){
                        temp = DRV_VLAN_INTELLIGENT_DT_MODE;
                    } else {
                        /* return for nothing to change */
                        return SOC_E_NONE;
                    }
                }
            } else {    /* DT_mode flow */
                /* check current value */
                if (temp & DRV_VLAN_FALCON_DT_MODE){
                    if (prop_val){
                        /* return for nothing to change */
                        return SOC_E_NONE;
                    } else {
                        temp &= ~DRV_VLAN_FALCON_DT_MODE;
                        if (temp & DRV_VLAN_INTELLIGENT_DT_MODE){
                            temp &= ~DRV_VLAN_INTELLIGENT_DT_MODE;
                        }
                    }
                } else {
                    if (prop_val){
                        if (temp & DRV_VLAN_INTELLIGENT_DT_MODE){
                            /* return for Vulcan is at iDT_mode */
                            return SOC_E_NONE;
                        } else {
                            temp = DRV_VLAN_FALCON_DT_MODE;
                        }
                    } else {
                        /* return for nothing to change */
                        return SOC_E_NONE;
                    }
                }
            }
        }
        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, VLAN_CTRL4r, &reg_value, EN_DOUBLE_TAGf, &temp));
            
        if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        device_dtag_mode = temp;
                        
        break;
    case DRV_VLAN_PROP_V2V:  /* device based VT enabling */
        
        /* enable/disable global VT feature */
        if ((prop_val) == (device_dtag_mode & DRV_VLAN_INTELLIGENT_DT_MODE)){
            /* current setting is the same */
            return SOC_E_NONE;
        }
        temp = (prop_val) ? DRV_VLAN_INTELLIGENT_DT_MODE : 
                (device_dtag_mode & (~DRV_VLAN_INTELLIGENT_DT_MODE));
        
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, VLAN_CTRL4r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, VLAN_CTRL4r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, VLAN_CTRL4r, &reg_value, EN_DOUBLE_TAGf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        
        device_dtag_mode = temp;
        
        break;
    case DRV_VLAN_PROP_EVR_INIT: /* Egress VLAN Remark(EVR) init */
        if (prop_val){
            /* init EVR only, the related CFP will be init in the runtime 
             *  (This is for the Field bcm API is not init yet during VLAN
             *      init section.)
             */
            rv =_drv_bcm53115_evr_init(unit);
        }
        break;
    case DRV_VLAN_PROP_EVR_VT_ISP_CHANGE: /* Egress VLAN Remark(EVR) init */
        if (prop_val){
            soc_pbmp_t  changed_bmp;
            
            SOC_PBMP_CLEAR(changed_bmp);
            SOC_PBMP_WORD_SET(changed_bmp, 0, prop_val);
            
            rv =_drv_bcm53115_evr_vt_isp_change(unit, changed_bmp);
        }
        break;
    case DRV_VLAN_PROP_PER_PORT_TRANSLATION:
        rv = SOC_E_UNAVAIL;
        break;
    default:
        rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}


/*
 *  Function : drv_vlan_prop_get
 *
 *  Purpose :
 *      Get the VLAN property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *
 */
int 
drv_bcm53115_vlan_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32  reg_addr, reg_value, temp;
    int reg_len, rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, 
            "%s: unit = %d, property type = %d, value = %x\n",
            FUNCTION_NAME(), unit, prop_type, *prop_val);
    switch (prop_type) {
    case DRV_VLAN_PROP_VTABLE_MISS_DROP:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL5r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL5r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, VLAN_CTRL5r, &reg_value, DROP_VTABLE_MISSf, &temp));
        if (temp) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }
        break;
    case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL0r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp));
        
        if (temp) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }
        break;            
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
    case DRV_VLAN_PROP_IDT_MODE_ENABLE:     /* check iDT_Mode status */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, VLAN_CTRL4r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, VLAN_CTRL4r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, VLAN_CTRL4r, &reg_value, EN_DOUBLE_TAGf, &temp));

        /* if the DT mode is preferred at iDT_mode.
         *  - only iDT mode is the recoganized DT mode.
         *  - when the DT_mode is retrived then the DT mode will be set to 
         *    DT mode disabled and return false.
         */
        temp &= DRV_VLAN_DT_MODE_MASK;
        if (DRV_VLAN_PREFER_DT_MODE == DRV_VLAN_INTELLIGENT_DT_MODE) {
            /* preferred at iDT_mode and indicating to get iDT_mode */
            if (prop_type == DRV_VLAN_PROP_IDT_MODE_ENABLE){
                *prop_val = (temp & DRV_VLAN_INTELLIGENT_DT_MODE) ? 
                                TRUE : FALSE ;
            } else {   
                if (temp & DRV_VLAN_INTELLIGENT_DT_MODE){
                    *prop_val = TRUE;
                } else {
                    *prop_val = FALSE;
                    if (temp & DRV_VLAN_FALCON_DT_MODE){
                        /* disable double tagging mode */
                        temp = DRV_VLAN_DT_MODE_DISABLE;
                        SOC_IF_ERROR_RETURN(
                                (DRV_SERVICES(unit)->reg_field_set)
                                        (unit, VLAN_CTRL4r, &reg_value, 
                                        EN_DOUBLE_TAGf, &temp));
                            
                        if ((rv = (DRV_SERVICES(unit)->reg_write)
                                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                            return rv;
                        }
                    }
                }
            }
        } else {
            if (prop_type == DRV_VLAN_PROP_IDT_MODE_ENABLE){
                *prop_val = (temp & DRV_VLAN_INTELLIGENT_DT_MODE) ? 
                                TRUE : FALSE ;
            } else {
                *prop_val = (temp & DRV_VLAN_FALCON_DT_MODE) ? 
                            TRUE : ((temp & DRV_VLAN_INTELLIGENT_DT_MODE) ? 
                                TRUE : FALSE) ;
            }
        }
         
        break;
    case DRV_VLAN_PROP_V2V:  /* device based VT enabling */
        *prop_val = (device_dtag_mode & DRV_VLAN_INTELLIGENT_DT_MODE); 

        break;
    case DRV_VLAN_PROP_EVR_VT_NEW_ENTRY_ID:
        *prop_val = new_evr_vt_entry_id;
        new_evr_vt_entry_id = 0;
        break;
    case DRV_VLAN_PROP_PER_PORT_TRANSLATION:
        rv = SOC_E_UNAVAIL;
        break;
    default:
        rv = SOC_E_UNAVAIL;
        break;
    }
    
    return rv;
}

/*
 *  Function : drv_vlan_prop_port_enable_set
 *
 *  Purpose :
 *      Set the port enable status by different VLAN property.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_val    :   vlan property value.
 *      bmp         : port bitmap
 *      val         : value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *
 */
int 
drv_bcm53115_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val)
{
    uint32 reg_addr, reg_len;
    uint32 reg_index = 0, fld_index = 0;    
    uint64  reg_value64;
    uint32 temp;
    int rv = SOC_E_NONE, i;
    soc_pbmp_t set_bmp, temp_bmp;

    soc_cm_debug(DK_VLAN, "%s: unit=%d, prop=%d, value=0x%x\n", 
            FUNCTION_NAME(), unit, prop_type, val);
    
    switch(prop_type){
    case    DRV_VLAN_PROP_ISP_PORT :
        reg_index = ISP_SEL_PORTMAPr;
        fld_index  = ISP_PORTMAPf;

        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index,(uint32 *)&reg_value64, fld_index, &temp));
    
        SOC_PBMP_CLEAR(temp_bmp);
        SOC_PBMP_WORD_SET(temp_bmp, 0, temp);
        
        /* check the action process */
        SOC_PBMP_CLEAR(set_bmp);
        SOC_PBMP_OR(set_bmp, temp_bmp);
        
        if (val == TRUE){       /* set for enable */
            SOC_PBMP_OR(set_bmp, bmp);
        }else {
            SOC_PBMP_REMOVE(set_bmp, bmp);
        }
    
        /* Port is CPU */
        /* Remark : still allow user to set CPU to none-ISP port. 
         *  - In bcm53115, when CPU is set to none-ISP and TPID is set to
         *    0x8100, there will be two 0x8100 tags been observed if the 
         *    egress port is NNI port.
         */ 
        /*
        SOC_PBMP_PORT_ADD(set_bmp, CMIC_PORT(unit));
         */
        
        /* check if the set value is equal to current setting */
        if (SOC_PBMP_EQ(temp_bmp, set_bmp)){
            /* do nothing */
            return SOC_E_NONE;
        }
        
        /* GNATS 41031 :
         *  - After VT entries created, once the ISP is changed to NON-ISP, 
         *    the unexpected outer tag will be transmitted from this changed 
         *    port(NON-ISP alrady). 
         */
        /* set sw flag on isp_port */
        SOC_PBMP_ITER(PBMP_ALL(unit), i){
            if (SOC_PBMP_MEMBER(set_bmp, i)){
                egr_vt_db[i].isp_port = TRUE;
            } else {
                egr_vt_db[i].isp_port = FALSE;
            }
        }

        /* write to register */
        temp = SOC_PBMP_WORD_GET(set_bmp, 0);    
    
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len));

        break;

    case    DRV_VLAN_PROP_V2V_PORT :
        /* 1. VLAN XLATE was supported in bcm53115 with device basis 
         *       (port will be ignored)
         * 2. function boundled with iDT_mode.
         */
        SOC_IF_ERROR_RETURN(drv_bcm53115_vlan_prop_set
                        (unit, DRV_VLAN_PROP_V2V, val));
        break;
    case    DRV_VLAN_PROP_INNER_TAG_PORT :
        /* bcm53115 can support this , TBD */
    default :
        return SOC_E_UNAVAIL;
        break;

    }
     
    return rv;
}

/*
 *  Function : drv_vlan_prop_port_enable_get
 *
 *  Purpose :
 *      Get the port enable status by different VLAN property.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_val    :   vlan property value.
 *      port_n      : port number. 
 *      val         : (OUT) value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. if port_n = 0xffffffff , means get device basis value.

 *
 */
int 
drv_bcm53115_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 *val)
{
    uint32 reg_addr, reg_len;
    uint32 reg_index = 0, fld_index = 0;    
    uint64  reg_value64;
    uint32 temp;
    soc_pbmp_t pbmp;
    int rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, "%s: unit=%d, prop=%d, port=%d\n", 
            FUNCTION_NAME(), unit, prop_type, port_n);

    switch(prop_type){
    case    DRV_VLAN_PROP_ISP_PORT :
        reg_index = ISP_SEL_PORTMAPr;
        fld_index  = ISP_PORTMAPf;

        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index,(uint32 *)&reg_value64, fld_index, &temp));
    
        /* check if the value get is port basis or device basis. */
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_WORD_SET(pbmp, 0, temp);
        
        if (port_n == 0xffffffff) {     /* device basis */
            int     i;
    
            for (i = 0; i < SOC_PBMP_WORD_MAX; i++){
                *(val + i) = SOC_PBMP_WORD_GET(pbmp, i);
            }
        } else {
            *val = (SOC_PBMP_MEMBER(pbmp, port_n)) ? TRUE : FALSE;
        }
    
        break;
    case    DRV_VLAN_PROP_V2V_PORT :
        /* 1. VLAN XLATE was supported in bcm53115 with device basis 
         * 2. function boundled with iDT_mode.
         */
    case    DRV_VLAN_PROP_INNER_TAG_PORT :
        /* bcm53115 can support this , TBD */
    default :
        return SOC_E_UNAVAIL;

    }
    return rv;

}
