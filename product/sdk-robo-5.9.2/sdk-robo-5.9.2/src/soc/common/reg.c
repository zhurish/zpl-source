/*
 * $Id: reg.c,v 1.20.6.2 Broadcom SDK $
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
 * Register address and value manipulations.
 */


#include <sal/core/libc.h>
#include <sal/core/boot.h>

#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/cmic.h>
#include <soc/register.h>


/*
 * Function:   soc_reg_datamask
 * Purpose:    Generate data mask for the fields in a register
 *             whose flags match the flags parameter
 * Returns:    The data mask
 *
 * Notes:  flags can be SOCF_RO, SOCF_WO, or zero (read/write)
 */
uint32
soc_reg_datamask(int unit, soc_reg_t reg, int flags)
{
    int			i, start, end;
    soc_field_info_t	*fieldp;
    soc_reg_info_t	*regp;
    uint32		result, mask;

    if (!SOC_REG_IS_VALID(unit, reg)) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s is invalid\n", soc_reg_name[reg]);
#endif
        assert(SOC_REG_IS_VALID(unit, reg));
    }

    regp = &(SOC_REG_INFO(unit, reg));

    result = 0;
    for (i = 0; i < (int)(regp->nFields); i++) {
	fieldp = &(regp->fields[i]);

	if ((fieldp->flags & flags) == flags) {
	    start = fieldp->bp;
	    if (start > 31) {
		continue;
	    }
	    end = fieldp->bp + fieldp->len;
	    if (end < 32) {
		mask = (1 << end) - 1;
	    } else {
		mask = -1;
	    }
	    result |= ((uint32)-1 << start) & mask;
	}
    }

    return result;
}

/*
 * Function:   soc_reg64_datamask
 * Purpose:    Generate data mask for the fields in a 64-bit register
 *             whose flags match the flags parameter
 * Returns:    The data mask
 *
 * Notes:  flags can be SOCF_RO, SOCF_WO, or zero (read/write)
 */
uint64
soc_reg64_datamask(int unit, soc_reg_t reg, int flags)
{
    int			i, start, end;
    soc_field_info_t	*fieldp;
    soc_reg_info_t	*regp;
    uint64		mask, tmp, result;

    if (!SOC_REG_IS_VALID(unit, reg)) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s is invalid\n", soc_reg_name[reg]);
#endif
        assert(SOC_REG_IS_VALID(unit, reg));
    }

    regp = &(SOC_REG_INFO(unit, reg));

    COMPILER_64_ZERO(result);

    for (i = 0; i < (int)(regp->nFields); i++) {
	fieldp = &(regp->fields[i]);

	if ((fieldp->flags & flags) == flags) {
	    start = fieldp->bp;
	    end = fieldp->bp + fieldp->len;
	    COMPILER_64_SET(mask, 0, 1);
	    COMPILER_64_SHL(mask, end);
	    COMPILER_64_SUB_32(mask, 1);
	    COMPILER_64_ZERO(tmp);
    /*    coverity[overflow_assign]    */
	    COMPILER_64_SUB_32(tmp, 1);
	    COMPILER_64_SHL(tmp, start);
	    COMPILER_64_AND(tmp, mask);
	    COMPILER_64_OR(result, tmp);
	}
    }

    return result;
}

/************************************************************************/
/* Routines for reading/writing SOC internal registers			*/
/************************************************************************/

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)

#ifdef BROADCOM_DEBUG

STATIC void
_soc_reg_debug(int unit, int access_width, char *access_type,
	       uint32 addr, uint32 data_hi, uint32 data_lo)
{
    soc_regaddrinfo_t	ainfo;
    char			buf[80];

    soc_regaddrinfo_get(unit, &ainfo, addr);

    if (!ainfo.valid || (int)ainfo.reg < 0) {
	sal_strcpy(buf, "??");
    } else {
	soc_reg_sprint_addr(unit, buf, &ainfo);
    }

    if (data_hi != 0) {
	soc_cm_debug(DK_REG,
		     "soc_reg%d_%s unit %d: "
		     "%s[0x%x] data=0x%08x_%08x\n",
		     access_width, access_type, unit,
		     buf, addr, data_hi, data_lo);
    } else {
	soc_cm_debug(DK_REG,
		     "soc_reg%d_%s unit %d: "
		     "%s[0x%x] data=0x%08x\n",
		     access_width, access_type, unit,
		     buf, addr, data_lo);
    }
}

#endif /* BROADCOM_DEBUG */

/* List of registers that need iterative read/write operations */
STATIC int
iterative_op_required(soc_reg_t reg)
{
    switch (reg) {
        case MAC_RXCTRLr:
        case MAC_RXMACSAr:
        case MAC_RXMAXSZr:
        case MAC_RXLSSCTRLr:
        case MAC_RXLSSSTATr:
        case MAC_RXSPARE0r:
        case IR64r:
        case IR127r:
        case IR255r:
        case IR511r:
        case IR1023r:
        case IR1518r:
        case IR2047r:
        case IR4095r:
        case IR9216r:
        case IR16383r:
        case IRMAXr:
        case IRPKTr:
        case IRFCSr:
        case IRUCr:
        case IRMCAr:
        case IRBCAr:
        case IRXPFr:
        case IRXPPr:
        case IRXUOr:
        case IRJBRr:
        case IROVRr:
        case IRXCFr:
        case IRFLRr:
        case IRPOKr:
        case IRMEGr:
        case IRMEBr:
        case IRBYTr:
        case IRUNDr:
        case IRFRGr:
        case IRERBYTr:
        case IRERPKTr:
        case IRJUNKr:
        case MAC_RXLLFCMSGCNTr:
        case MAC_RXLLFCMSGFLDSr:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

/*
 * Iterative read procedure for MAC registers on Hyperlite ports.
 */
STATIC int
soc_reg64_read_iterative(int unit,
  		          uint32 addr,
                          soc_port_t port,
		          uint64 *data)
{
    int rv, i, diff;
    uint64 xgxs_stat;
    uint32 locked;
    sal_usecs_t t1, t2;
    soc_timeout_t to;
    for (i = 0; i < 100; i++) {
       /* Read PLL lock status */
       t1 = sal_time_usecs();
       soc_timeout_init(&to, 25 * MILLISECOND_USEC, 0);
       do {
           rv = READ_MAC_XGXS_STATr(unit, port, &xgxs_stat);
           locked = soc_reg64_field32_get(unit, MAC_XGXS_STATr, xgxs_stat, 
                                          TXPLL_LOCKf);
           if (locked || SOC_FAILURE(rv)) {
               break;
           }
       } while (!soc_timeout_check(&to));
       if (SOC_FAILURE(rv)) {
           return rv;
       }
       if (!locked) {
           continue;
       }
       /* Write register value */
       SOC_IF_ERROR_RETURN(soc_reg64_read(unit, addr, data));
       /* Read PLL lock status */
       SOC_IF_ERROR_RETURN(READ_MAC_XGXS_STATr(unit, port, &xgxs_stat));
       locked = soc_reg64_field32_get(unit, MAC_XGXS_STATr, xgxs_stat, 
                                      TXPLL_LOCKf);
       t2 = sal_time_usecs();
       diff = SAL_USECS_SUB(t2, t1);
       if (locked && (diff < 20 * MILLISECOND_USEC)) {
           return SOC_E_NONE;
       }
       soc_cm_debug(DK_WARN | DK_VERBOSE,
 	            "soc_reg64_read_iterative: WARNING: "
		    "iteration %d PLL went out of lock",
                    i);
    }
    soc_cm_debug(DK_ERR,
  	         "soc_reg64_read_iterative: "
		 "operation failed:\n"); 
    return SOC_E_FAIL;    
}

/*
 * Read an internal SOC register through S-Channel messaging buffer.
 * Checks if the register is 32 or 64 bits.
 */

int
soc_reg_read(int unit, soc_reg_t reg, uint32 addr, uint64 *data)
{
    if (!SOC_REG_IS_VALID(unit, reg)) {
        return SOC_E_PARAM;
    }

    if (SOC_REG_IS_64(unit, reg)) {
        soc_port_t port;
        soc_block_t regblktype = SOC_REG_INFO(unit, reg).block;
        int blk, pindex, bindex, block;
        pindex = (addr >> SOC_REGIDX_BP) & 0x3f;
        block = ((addr >> SOC_BLOCK_BP) & 0xf) |
                (((addr >> SOC_BLOCK_MSB_BP) & 0x3) << 4);
        if ((regblktype & SOC_BLK_PORT) && iterative_op_required(reg)) {
            PBMP_HYPLITE_ITER(unit, port) {
                blk = SOC_PORT_INFO(unit, port).blk;
                bindex = SOC_PORT_INFO(unit, port).bindex;
                if ((SOC_BLOCK2SCH(unit, blk) == block) && (bindex == pindex)) {
                    break;
                }
            }         
            if (!IS_HYPLITE_PORT(unit, port)) {
                return soc_reg64_read(unit, addr, data);
            } else {   
                return soc_reg64_read_iterative(unit, addr, port, data);
            }
        } else {
            return soc_reg64_read(unit, addr, data);
        }
    } else {
        uint32 data32;

        SOC_IF_ERROR_RETURN(soc_reg32_read(unit, addr, &data32));
        COMPILER_64_SET(*data, 0, data32);
    }

    return SOC_E_NONE;
}

/*
 * Read an internal SOC register through S-Channel messaging buffer.
 */

int
soc_reg32_read(int unit,
	     uint32 addr,
	     uint32 *data)
{
    schan_msg_t schan_msg;
    uint32 block = 0;

    /*
     * Write message to S-Channel.
     */
    schan_msg_clear(&schan_msg);
    schan_msg.readcmd.header.opcode = READ_REGISTER_CMD_MSG;
    schan_msg.readcmd.header.datalen = (SOC_IS_XGS12_FABRIC(unit) ? 8 : 4);
    schan_msg.readcmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    /* required on XGS3. Optional on other devices */ 
    schan_msg.readcmd.header.dstblk = ((addr >> SOC_BLOCK_BP) & 0xf) | 
                                  (((addr >> SOC_BLOCK_MSB_BP) & 0x3) << 4);
#endif /* BCM_XGS3_SWITCH_SUPPORT || defined(BCM_SIRIUS_SUPPORT) */
    schan_msg.readcmd.address = addr;

    if (SOC_IS_SIRIUS(unit)) {
	schan_msg.readcmd.header.datalen = 0;
	schan_msg.readcmd.header.srcblk = 0;
	if ((!SAL_BOOT_PLISIM || SAL_BOOT_BCMSIM) && (schan_msg.readcmd.header.dstblk != CMIC_BLOCK(unit))) {
	    block = schan_msg.readcmd.header.dstblk;
	    schan_msg.readcmd.address &= 0xC0FFF;
	}
    }
    if (SOC_IS_SHADOW(unit)) {
       schan_msg.readcmd.header.srcblk = 0;
    }

    /* Write header word + address DWORD, read header word + data DWORD */
    SOC_IF_ERROR_RETURN(soc_schan_op(unit, &schan_msg, 2, 2, 0));

    /* Check result */
    
    if (schan_msg.readresp.header.opcode != READ_REGISTER_ACK_MSG) {
	soc_cm_debug(DK_ERR,
		     "soc_reg32_read: "
		     "invalid S-Channel reply, expected READ_REG_ACK:\n");
	soc_schan_dump(unit, &schan_msg, 2);
	return SOC_E_INTERNAL;
    }

    *data = schan_msg.readresp.data[0];

#ifdef BROADCOM_DEBUG
    if (soc_cm_debug_check(DK_REG)) {
	_soc_reg_debug(unit, 32, "read", addr, 0, *data);
    }
#endif /* BROADCOM_DEBUG */

    return SOC_E_NONE;
}

/*
 * Read an internal 64-bit SOC register through S-Channel messaging buffer.
 */
int
soc_reg64_read(int unit,
	       uint32 addr,
	       uint64 *reg)
{
    schan_msg_t		schan_msg;
    uint32 block = 0;

    /*
     * Write message to S-Channel.
     */
    schan_msg_clear(&schan_msg);
    schan_msg.readcmd.header.opcode = READ_REGISTER_CMD_MSG;
    schan_msg.readcmd.header.datalen = 8;
    schan_msg.readcmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
    schan_msg.readcmd.address = addr;
#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    /* required on XGS3. Optional on other devices */ 
    schan_msg.readcmd.header.dstblk = ((addr >> SOC_BLOCK_BP) & 0xf) | 
                                  (((addr >> SOC_BLOCK_MSB_BP) & 0x3) << 4);
#endif /* BCM_XGS3_SWITCH_SUPPORT || defined(BCM_SIRIUS_SUPPORT) */

    if (SOC_IS_SIRIUS(unit)) {
	schan_msg.readcmd.header.datalen = 0;
	schan_msg.readcmd.header.srcblk = 0;
	if ((!SAL_BOOT_PLISIM || SAL_BOOT_BCMSIM) && (schan_msg.readcmd.header.dstblk != CMIC_BLOCK(unit))) {
	    block = schan_msg.readcmd.header.dstblk;
	    schan_msg.readcmd.address &= 0xC0FFF;
	}
    }
    if (SOC_IS_SHADOW(unit)) {
       schan_msg.readcmd.header.srcblk = 0;
    }


    /* Write header word + address DWORD, read header word + data DWORD */
    SOC_IF_ERROR_RETURN(soc_schan_op(unit, &schan_msg, 2, 3, 0));

    /* Check result */
    
    if (schan_msg.readresp.header.opcode != READ_REGISTER_ACK_MSG) {
	soc_cm_debug(DK_ERR,
		     "soc_reg64_read: "
		     "invalid S-Channel reply, expected READ_REG_ACK:\n");
	soc_schan_dump(unit, &schan_msg, 2);
	return SOC_E_INTERNAL;
    }

#ifdef BROADCOM_DEBUG
    if (soc_cm_debug_check(DK_REG)) {
	_soc_reg_debug(unit, 64, "read", addr,
		       schan_msg.readresp.data[1],
		       schan_msg.readresp.data[0]);
    }
#endif /* BROADCOM_DEBUG */

    COMPILER_64_SET(*reg,
		    schan_msg.readresp.data[1],
		    schan_msg.readresp.data[0]);

    return SOC_E_NONE;
}

/*
 * Iterative write procedure for MAC registers on Hyperlite ports.
 */
STATIC int
soc_reg64_write_iterative(int unit,
  		          uint32 addr,
                          soc_port_t port,
		          uint64 data)
{
    int rv, i, diff;
    uint64 xgxs_stat;
    uint32 locked;
    sal_usecs_t t1, t2;
    soc_timeout_t to;
    for (i = 0; i < 100; i++) {
       /* Read PLL lock status */
       t1 = sal_time_usecs();
       soc_timeout_init(&to, 25 * MILLISECOND_USEC, 0);
       do {
           rv = READ_MAC_XGXS_STATr(unit, port, &xgxs_stat);
           locked = soc_reg64_field32_get(unit, MAC_XGXS_STATr, xgxs_stat, 
                                          TXPLL_LOCKf);
           if (locked || SOC_FAILURE(rv)) {
               break;
           }
       } while (!soc_timeout_check(&to));
       if (SOC_FAILURE(rv)) {
           return rv;
       }
       if (!locked) {
           continue;
       }
       /* Write register value */
       SOC_IF_ERROR_RETURN(soc_reg64_write(unit, addr, data));
       /* Read PLL lock status */
       SOC_IF_ERROR_RETURN(READ_MAC_XGXS_STATr(unit, port, &xgxs_stat));
       locked = soc_reg64_field32_get(unit, MAC_XGXS_STATr, xgxs_stat, 
                                      TXPLL_LOCKf);
       t2 = sal_time_usecs();
       diff = SAL_USECS_SUB(t2, t1);
       if (locked && (diff < 20 * MILLISECOND_USEC)) {
           return SOC_E_NONE;
       }
       soc_cm_debug(DK_WARN | DK_VERBOSE,
 	            "soc_reg64_write_iterative: WARNING: "
		    "iteration %d PLL went out of lock",
                    i);
    }
    soc_cm_debug(DK_ERR,
  	         "soc_reg64_write_iterative: "
		 "operation failed:\n"); 
    return SOC_E_FAIL;    
}

/*
 * Write an internal SOC register through S-Channel messaging buffer.
 * Checks if the register is 32 or 64 bits.
 */

int
soc_reg_write(int unit, soc_reg_t reg, uint32 addr, uint64 data)
{
    if (!SOC_REG_IS_VALID(unit, reg)) {
        return SOC_E_PARAM;
    }

    if (SOC_REG_IS_64(unit, reg)) {
        soc_port_t port;
        soc_block_t regblktype = SOC_REG_INFO(unit, reg).block;
        int blk, pindex, bindex, block;
        pindex = (addr >> SOC_REGIDX_BP) & 0x3f;
        block = ((addr >> SOC_BLOCK_BP) & 0xf) |
                (((addr >> SOC_BLOCK_MSB_BP) & 0x3) << 4);
        if ((regblktype & SOC_BLK_PORT) && iterative_op_required(reg)) {
            PBMP_HYPLITE_ITER(unit, port) {
                blk = SOC_PORT_INFO(unit, port).blk;
                bindex = SOC_PORT_INFO(unit, port).bindex;
                if ((SOC_BLOCK2SCH(unit, blk) == block) && (bindex == pindex)) {
                    break;
                }
            }         
            if (!IS_HYPLITE_PORT(unit, port)) {
                return soc_reg64_write(unit, addr, data);
            } else {   
                return soc_reg64_write_iterative(unit, addr, port, data);
            }
        } else {
            return soc_reg64_write(unit, addr, data);
        }
    } else {
        if (COMPILER_64_HI(data)) {
            soc_cm_debug(DK_WARN,
			 "soc_reg_write: WARNING: "
			"write to 32-bit reg %s with hi order data, 0x%x\n",
                         SOC_REG_NAME(unit, reg),
                         COMPILER_64_HI(data));
        }
        SOC_IF_ERROR_RETURN(soc_reg32_write(unit, addr,
                                            COMPILER_64_LO(data)));
    }

    return SOC_E_NONE;
}

/*
 * Write an internal SOC register through S-Channel messaging buffer.
 */

int
soc_reg32_write(int unit,
	      uint32 addr,
	      uint32 data)
{
    schan_msg_t schan_msg;
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
    uint32 block = 0;
#endif /* BCM_SIRIUS_SUPPORT || BCM_SHADOW_SUPPORT */

    /*
     * Setup S-Channel command packet
     *
     * NOTE: the datalen field matters only for the Write Memory and
     * Write Register commands, where it is used only by the CMIC to
     * determine how much data to send, and is in units of bytes.
     */

    schan_msg_clear(&schan_msg);
    schan_msg.writecmd.header.opcode = WRITE_REGISTER_CMD_MSG;
    schan_msg.writecmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
    schan_msg.writecmd.header.datalen = SOC_IS_XGS12_FABRIC(unit) ? 8 : 4;
#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    /* required on XGS3. Optional on other devices */ 
    schan_msg.writecmd.header.dstblk = ((addr >> SOC_BLOCK_BP) & 0xf) | 
                                  (((addr >> SOC_BLOCK_MSB_BP) & 0x3) << 4);
#endif /* BCM_XGS3_SWITCH_SUPPORT || defined(BCM_SIRIUS_SUPPORT) */
    schan_msg.writecmd.address = addr;
    schan_msg.writecmd.data[0] = data;

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
    schan_msg.writecmd.header.datalen = (SOC_IS_SIRIUS(unit) ? 4 : schan_msg.writecmd.header.datalen);
    schan_msg.writecmd.header.srcblk = ((SOC_IS_SIRIUS(unit) || SOC_IS_SHADOW(unit)) ? 
                                        0 : schan_msg.writecmd.header.srcblk);
    if (SOC_IS_SIRIUS(unit) && (!SAL_BOOT_PLISIM || SAL_BOOT_BCMSIM) && 
	(schan_msg.writecmd.header.dstblk != CMIC_BLOCK(unit))) {
	block = schan_msg.writecmd.header.dstblk;
	schan_msg.writecmd.address &= 0xC0FFF;
    }
#endif /* BCM_SIRIUS_SUPPORT || BCM_SHADOW_SUPPORT */

#ifdef BROADCOM_DEBUG
    if (soc_cm_debug_check(DK_REG)) {
	_soc_reg_debug(unit, 32, "write", addr, 0, data);
    }
#endif /* BROADCOM_DEBUG */

    /* Write header word + address + data DWORD */
    /* Note: The hardware does not send WRITE_REGISTER_ACK_MSG. */
    
    
    

    return soc_schan_op(unit, &schan_msg, 3, 0, 0);
}

/*
 * Write an internal 64-bit SOC register through S-Channel messaging buffer.
 */
int
soc_reg64_write(int unit,
		uint32 addr,
		uint64 data)
{
    schan_msg_t		schan_msg;
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
    uint32 block = 0;
#endif /* BCM_SIRIUS_SUPPORT || BCM_SHADOW_SUPPORT */

    /*
     * Setup S-Channel command packet
     *
     * NOTE: the datalen field matters only for the Write Memory and
     * Write Register commands, where it is used only by the CMIC to
     * determine how much data to send, and is in units of bytes.
     */

    schan_msg_clear(&schan_msg);
    schan_msg.writecmd.header.opcode = WRITE_REGISTER_CMD_MSG;
    schan_msg.writecmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
    schan_msg.writecmd.header.datalen = 8;
#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    /* required on XGS3. Optional on other devices */ 
    schan_msg.writecmd.header.dstblk = ((addr >> SOC_BLOCK_BP) & 0xf) | 
                                  (((addr >> SOC_BLOCK_MSB_BP) & 0x3) << 4);
#endif /* BCM_XGS3_SWITCH_SUPPORT || defined(BCM_SIRIUS_SUPPORT) */
    schan_msg.writecmd.address = addr;
    schan_msg.writecmd.data[0] = COMPILER_64_LO(data);
    schan_msg.writecmd.data[1] = COMPILER_64_HI(data);

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
    schan_msg.writecmd.header.srcblk = ((SOC_IS_SIRIUS(unit) || SOC_IS_SHADOW(unit)) ?
                                       0 : schan_msg.writecmd.header.srcblk);
    if (SOC_IS_SIRIUS(unit) && (!SAL_BOOT_PLISIM || SAL_BOOT_BCMSIM) && 
	(schan_msg.writecmd.header.dstblk != CMIC_BLOCK(unit))) {
	block = schan_msg.writecmd.header.dstblk;
	schan_msg.writecmd.address &= 0xC0FFF;
    }
#endif /* BCM_SIRIUS_SUPPORT || BCM_SHADOW_SUPPORT */

#ifdef BROADCOM_DEBUG
    if (soc_cm_debug_check(DK_REG)) {
	_soc_reg_debug(unit, 64, "write", addr,
		       schan_msg.writecmd.data[1],
		       schan_msg.writecmd.data[0]);
    }
#endif /* BROADCOM_DEBUG */

    /* Write header word + address + 2*data DWORD */
    /* Note: The hardware does not send WRITE_REGISTER_ACK_MSG. */
    
    
    

    return soc_schan_op(unit, &schan_msg, 4, 0, 0);
}

/*
 * Write internal register for a group of ports.
 * The specified register must be a port reg (type soc_portreg).
 */

int
soc_reg_write_ports(int unit,
		    soc_reg_t reg,
		    pbmp_t pbmp,
		    uint32 value)
{
    soc_port_t		port;
    soc_block_t		ptype, rtype;

    /* assert(reg is a port register) */
    if (!SOC_REG_IS_VALID(unit, reg) ||
	SOC_REG_INFO(unit, reg).regtype != soc_portreg) {
	return SOC_E_UNAVAIL;
    }

    rtype = SOC_REG_INFO(unit, reg).block;

    /*
     * each port block type must match one of the register block types
     * or the register block type can be the MMU
     */
    PBMP_ITER(pbmp, port) {
	ptype = SOC_PORT_TYPE(unit, port);
	if ((ptype & rtype) || rtype == SOC_BLK_MMU) {
		SOC_IF_ERROR_RETURN(soc_reg32_write(unit,
					    soc_reg_addr(unit, reg, port, 0),
					    value));
	}
    }
    return SOC_E_NONE;
}

/*
 * Write internal register for a block or group of blocks.
 * The specified register must be a generic reg (type soc_genreg).
 *
 * This routine will write to all possible blocks for the given
 * register.
 */
int
soc_reg64_write_all_blocks(int unit,
                           soc_reg_t reg,
                           uint64 value)
{
    int		blk, port;
    soc_block_t	rtype;

    /* assert(reg is not a port or cos register) */
    if (!SOC_REG_IS_VALID(unit, reg) ||
	SOC_REG_INFO(unit, reg).regtype != soc_genreg) {
	return SOC_E_UNAVAIL;
    }

    rtype = SOC_REG_INFO(unit, reg).block;

    SOC_BLOCK_ITER(unit, blk, rtype) {
	port = SOC_BLOCK_PORT(unit, blk);
	SOC_IF_ERROR_RETURN(soc_reg_write(unit, reg,
					  soc_reg_addr(unit, reg, port, 0),
					  value));
    }
    return SOC_E_NONE;
}

/*
 * Write internal register for a block or group of blocks.
 * The specified register must be a generic reg (type soc_genreg).
 *
 * This routine will write to all possible blocks for the given
 * register.
 */
int
soc_reg_write_all_blocks(int unit,
                         soc_reg_t reg,
                         uint32 value)
{
    uint64	val64;

    if (!SOC_REG_IS_VALID(unit, reg)) {
	return SOC_E_PARAM;
    }

    COMPILER_64_SET(val64, 0, value);
    return soc_reg64_write_all_blocks(unit, reg, val64);
}

/*
 * Read a general register from any block that has a copy
 */
int
soc_reg64_read_any_block(int unit,
                         soc_reg_t reg,
                         uint64 *datap)
{
    int		blk, port;
    soc_block_t	rtype;

    /* assert(reg is not a port or cos register) */
    if (!SOC_REG_IS_VALID(unit, reg) ||
	SOC_REG_INFO(unit, reg).regtype != soc_genreg) {
	return SOC_E_UNAVAIL;
    }

    rtype = SOC_REG_INFO(unit, reg).block;
    SOC_BLOCK_ITER(unit, blk, rtype) {
	port = SOC_BLOCK_PORT(unit, blk);
	SOC_IF_ERROR_RETURN(soc_reg_read(unit, reg,
                                         soc_reg_addr(unit, reg, port, 0),
                                         datap));
	break;
    }

    return SOC_E_NONE;
}

/*
 * Read a general register from any block that has a copy
 */
int
soc_reg_read_any_block(int unit,
		     soc_reg_t reg,
		     uint32 *datap)
{
    uint64 val64;

    SOC_IF_ERROR_RETURN(soc_reg64_read_any_block(unit, reg, &val64));
    COMPILER_64_TO_32_LO(*datap, val64);

    return SOC_E_NONE;
}

/****************************************************************
 * Register field manipulation functions
 ****************************************************************/

/* Define a macro so the assertion printout is informative. */
#define	REG_FIELD_IS_VALID	finfop

/*
 * Function:     soc_reg_field_length
 * Purpose:      Return the length of a register field in bits.
 *               Value is 0 if field is not found.
 * Returns:      bits in field
 */
int
soc_reg_field_length(int unit, soc_reg_t reg, soc_field_t field)
{
    soc_field_info_t	*finfop;

    if (!SOC_REG_IS_VALID(unit, reg)) {
        return 0;
    }

    SOC_FIND_FIELD(field,
		   SOC_REG_INFO(unit, reg).fields,
		   SOC_REG_INFO(unit, reg).nFields,
		   finfop);
    if (finfop == NULL) {
	return 0;
    }
    return finfop->len;
}

/*
 * Function:     soc_reg_field_valid
 * Purpose:      Determine if a field in a register is valid.
 * Returns:      Returns TRUE  if field is found.
 *               Returns FALSE if field is not found.
 */
int
soc_reg_field_valid(int unit, soc_reg_t reg, soc_field_t field)
{
    soc_field_info_t	*finfop;

    if (!SOC_REG_IS_VALID(unit, reg)) {
        return FALSE;
    }

    SOC_FIND_FIELD(field,
		   SOC_REG_INFO(unit, reg).fields,
		   SOC_REG_INFO(unit, reg).nFields,
		   finfop);
    return (finfop != NULL);
}


/*
 * Function:     soc_reg_field_get
 * Purpose:      Get the value of a field from a register
 * Parameters:
 * Returns:      Value of field
 */
uint32
soc_reg_field_get(int unit, soc_reg_t reg, uint32 regval, soc_field_t field)
{
    soc_field_info_t	*finfop;
    uint32		val;

    if (!SOC_REG_IS_VALID(unit, reg)) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s is invalid\n", soc_reg_name[reg]);
#endif
        assert(SOC_REG_IS_VALID(unit, reg));
    }

    SOC_FIND_FIELD(field,
		   SOC_REG_INFO(unit, reg).fields,
		   SOC_REG_INFO(unit, reg).nFields,
		   finfop);

    if (finfop == NULL) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s field %s is invalid\n",
                     soc_reg_name[reg], soc_fieldnames[field]);
#endif
        assert(finfop);
    }

    val = regval >> finfop->bp;
    if (finfop->len < 32) {
	return val & ((1 << finfop->len) - 1);
    } else {
	return val;
    }
}

/*
 * Function:     soc_reg64_field_get
 * Purpose:      Get the value of a field from a 64-bit register
 * Parameters:
 * Returns:      Value of field (64 bits)
 */
uint64
soc_reg64_field_get(int unit, soc_reg_t reg, uint64 regval, soc_field_t field)
{
    soc_field_info_t	*finfop;
    uint64		mask;

    if (!SOC_REG_IS_VALID(unit, reg)) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s is invalid\n", soc_reg_name[reg]);
#endif
        assert(SOC_REG_IS_VALID(unit, reg));
    }

    SOC_FIND_FIELD(field,
		   SOC_REG_INFO(unit, reg).fields,
		   SOC_REG_INFO(unit, reg).nFields,
		   finfop);
    if (finfop == NULL) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s field %s is invalid\n",
                     soc_reg_name[reg], soc_fieldnames[field]);
#endif
        assert(finfop);
    }

    COMPILER_64_MASK_CREATE(mask, finfop->len, 0);
    COMPILER_64_SHR(regval, finfop->bp);
    COMPILER_64_AND(regval, mask);

    return regval;
}

/* 
 * Function:     soc_reg64_field32_get
 * Purpose:      Get the value of a field from a 64-bit register
 * Parameters:
 * Returns:      Value of field (32 bits)
 */
uint32
soc_reg64_field32_get(int unit, soc_reg_t reg, uint64 regval,
                      soc_field_t field)
{
    soc_field_info_t	*finfop;
    uint32		val32;

    if (!SOC_REG_IS_VALID(unit, reg)) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s is invalid\n", soc_reg_name[reg]);
#endif
        assert(SOC_REG_IS_VALID(unit, reg));
    }

    SOC_FIND_FIELD(field,
		   SOC_REG_INFO(unit, reg).fields,
		   SOC_REG_INFO(unit, reg).nFields,
		   finfop);
    if (finfop == NULL) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s field %s is invalid\n",
                     soc_reg_name[reg], soc_fieldnames[field]);
#endif
        assert(finfop);
    }

    COMPILER_64_SHR(regval, finfop->bp);
    COMPILER_64_TO_32_LO(val32, regval);
    if (finfop->len < 32) {
	return val32 & ((1 << finfop->len) - 1);
    } else {
	return val32;
    }
}


/* Define a macro so the assertion printout is informative. */
#define VALUE_TOO_BIG_FOR_FIELD		((value & ~mask) != 0)

/*
 * Function:     soc_reg_field_set
 * Purpose:      Set the value of a register's field.
 * Parameters:
 * Returns:      void
 */
void
soc_reg_field_set(int unit, soc_reg_t reg, uint32 *regval,
		  soc_field_t field, uint32 value)
{
    soc_field_info_t	*finfop;
    uint32		mask;

    if (!SOC_REG_IS_VALID(unit, reg)) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s is invalid\n", soc_reg_name[reg]);
#endif
        assert(SOC_REG_IS_VALID(unit, reg));
    }

    SOC_FIND_FIELD(field,
		   SOC_REG_INFO(unit, reg).fields,
		   SOC_REG_INFO(unit, reg).nFields,
		   finfop);
    if (finfop == NULL) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s field %s is invalid\n",
                     soc_reg_name[reg], soc_fieldnames[field]);
#endif
        assert(finfop);
    }

    if (finfop->len < 32) {
	mask = (1 << finfop->len) - 1;
   if  (VALUE_TOO_BIG_FOR_FIELD) {
	assert(!VALUE_TOO_BIG_FOR_FIELD);
   }
    } else {
	mask = -1;
    }

    *regval = (*regval & ~(mask << finfop->bp)) | value << finfop->bp;
}

/*
 * Function:     soc_reg64_field_set
 * Purpose:      Set the value of a register's field.
 * Parameters:
 * Returns:      void
 */
void
soc_reg64_field_set(int unit, soc_reg_t reg, uint64 *regval,
		    soc_field_t field, uint64 value)
{
    soc_field_info_t	*finfop;
    uint64		mask, tmp;

    if (!SOC_REG_IS_VALID(unit, reg)) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s is invalid\n", soc_reg_name[reg]);
#endif
        assert(SOC_REG_IS_VALID(unit, reg));
    }

    SOC_FIND_FIELD(field,
		   SOC_REG_INFO(unit, reg).fields,
		   SOC_REG_INFO(unit, reg).nFields,
		   finfop);
    if (finfop == NULL) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s field %s is invalid\n",
                     soc_reg_name[reg], soc_fieldnames[field]);
#endif
        assert(finfop);
    }

    if (finfop->len < 64) {
	COMPILER_64_SET(mask, 0, 1);
	COMPILER_64_SHL(mask, finfop->len);
	COMPILER_64_SUB_32(mask, 1);
#ifndef NDEBUG
	/* assert(!VALUE_TOO_BIG_FOR_FIELD); */
#define	VALUE_TOO_BIG_FOR_FIELD64	(!COMPILER_64_IS_ZERO(tmp))
	tmp = mask;
	COMPILER_64_NOT(tmp);
	COMPILER_64_AND(tmp, value);
	assert(!VALUE_TOO_BIG_FOR_FIELD64);
#endif
    } else {
	COMPILER_64_SET(mask, -1, -1);
    }

    /* *regval = (*regval & ~(mask << finfop->bp)) | value << finfop->bp; */
    tmp = mask;
    COMPILER_64_SHL(tmp, finfop->bp);
    COMPILER_64_NOT(tmp);
    COMPILER_64_AND(*regval, tmp);
    COMPILER_64_SHL(value, finfop->bp);
    COMPILER_64_OR(*regval, value);
}

/* 
 * Function:     soc_reg64_field32_set
 * Purpose:      Set the value of a register's field; field must be < 32 bits
 * Parameters:
 * Returns:      void
 */
void
soc_reg64_field32_set(int unit, soc_reg_t reg, uint64 *regval,
                      soc_field_t field, uint32 value)
{
    soc_field_info_t	*finfop;
    uint64		mask, tmp;

    if (!SOC_REG_IS_VALID(unit, reg)) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s is invalid\n", soc_reg_name[reg]);
#endif
        assert(SOC_REG_IS_VALID(unit, reg));
    }

    SOC_FIND_FIELD(field,
		   SOC_REG_INFO(unit, reg).fields,
		   SOC_REG_INFO(unit, reg).nFields,
		   finfop);
    if (finfop == NULL) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s field %s is invalid\n",
                     soc_reg_name[reg], soc_fieldnames[field]);
#endif
        assert(finfop);
    }

    if (finfop->len < 64) {
	COMPILER_64_SET(mask, 0, 1);
	COMPILER_64_SHL(mask, finfop->len);
	COMPILER_64_SUB_32(mask, 1);
    } else {
	COMPILER_64_SET(mask, -1, -1);
    }

    /* *regval = (*regval & ~(mask << finfop->bp)) | value << finfop->bp; */
    COMPILER_64_SHL(mask, finfop->bp);
    COMPILER_64_NOT(mask);
    COMPILER_64_AND(*regval, mask);
    if (value != 0) {
	COMPILER_64_SET(tmp, 0, value);
	COMPILER_64_SHL(tmp, finfop->bp);
	COMPILER_64_OR(*regval, tmp);
    }

}

/*
 * Function:	soc_reg_addr
 * Purpose:	calculate the address of a register
 * Parameters:
 *		unit	switch unit
 *		reg	register number
 *		port	port number or REG_PORT_ANY
 *		index	array index (or cos number)
 * Returns:	register address suitable for soc_reg_read and friends
 * Notes:	the block number to access is determined by the register
 *		and the port number
 *
 * cpureg	00SSSSSS 00000000 0000RRRR RRRRRRRR
 * genreg	00SSSSSS BBBB1000 0000RRRR RRRRRRRR
 * portreg	00SSSSSS BBBB00PP PPPPRRRR RRRRRRRR
 * cosreg	00SSSSSS BBBB01CC CCCCRRRR RRRRRRRR
 *
 * all regs of bcm88230
 *              00000000 00001000 0000RRRR RRRRRRRR
 *
 * where	B+ is the 4 bit block number
 *		P+ is the 6 bit port number (within a block or chip wide)
 *		C+ is the 6 bit class of service
 *		R+ is the 12 bit register number
 *		S+ is the 6 bit Pipe stage
 */
uint32
soc_reg_addr(int unit, soc_reg_t reg, int port, int index)
{
    uint32		base;		/* base address from reg_info */
    int			block;		/* block number */
    int			pindex;		/* register port/cos field */
    int			gransh;		/* index granularity shift */
    soc_block_t		regblktype;
    soc_block_t         portblktype;
    int                 phy_port;
    
    if (!SOC_REG_IS_VALID(unit, reg)) {
#if !defined(SOC_NO_NAMES)
        soc_cm_print("reg %s is invalid\n", soc_reg_name[reg]);
#endif
        assert(SOC_REG_IS_VALID(unit, reg));
    }

#define	SOC_REG_ADDR_INVALID_PORT	0	/* for asserts */


#ifdef  BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
	portblktype = SOC_BLK_SBX_PORT;
    } 
    else 
#endif
    {
	portblktype = SOC_BLK_PORT;
    }

    regblktype = SOC_REG_INFO(unit, reg).block;
    if (port >= 0) {
	if (regblktype & portblktype) {
	    assert(SOC_PORT_VALID(unit, port));
            if (soc_feature(unit, soc_feature_logical_port_num)) {
                phy_port = SOC_INFO(unit).port_l2p_mapping[port];
            } else {
                phy_port = port;
            }
	    block = SOC_PORT_INFO(unit, phy_port).blk;
	    pindex = SOC_PORT_INFO(unit, phy_port).bindex;
	} else {
	    block = pindex = -1;                /* multiple non-port block */
	}
    } else if (port == REG_PORT_ANY) {
	block = pindex = -1;
	PBMP_ALL_ITER(unit, port) {		/* try enabled ports */
            if (soc_feature(unit, soc_feature_logical_port_num)) {
                phy_port = SOC_INFO(unit).port_l2p_mapping[port];
            } else {
                phy_port = port;
            }
	    block = SOC_PORT_INFO(unit, phy_port).blk;
	    pindex = SOC_PORT_INFO(unit, phy_port).bindex;
	    if (regblktype & portblktype) {	/* match reg type */
		if (SOC_BLOCK_INFO(unit, block).type & regblktype) {
		    break;
		}
		block = -1;
	    } else {				/* match any port */
		break;
	    }
	}
	if (block < 0) {
	    assert(SOC_REG_ADDR_INVALID_PORT);	/* invalid port */
	}
    } else {
#ifdef  BCM_SIRIUS_SUPPORT
        if ((SOC_IS_SIRIUS(unit)) && (regblktype == SOC_BLK_CI)) {
            /* for SOC_BLK_CI, port is really an instance number */
            port &= (~SOC_REG_ADDR_INSTANCE_MASK);
	    block = pindex = -1;
        } else {
#endif
	assert(SOC_REG_ADDR_INVALID_PORT);	/* invalid port */
	block = pindex = -1;
#ifdef  BCM_SIRIUS_SUPPORT
        }
#endif
    }

#ifdef  BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
	if (!(regblktype & portblktype)) {
	    switch (regblktype) {
		case SOC_BLK_CMIC:
		    block = CMIC_BLOCK(unit);
		    break;
		case SOC_BLK_BP:
		    block = BP_BLOCK(unit);
		    break;
		case SOC_BLK_CI:
		    if (port >= 10) {
			assert(SOC_REG_ADDR_INVALID_PORT);	/* invalid port */
		    } else {
			block = CI_BLOCK(unit, port);
		    }
		    break;
		case SOC_BLK_CS:
		    block = CS_BLOCK(unit);
		    break;
		case SOC_BLK_EB:
		    block = EB_BLOCK(unit);
		    break;
		case SOC_BLK_EP:
		    block = EP_BLOCK(unit);
		    break;
		case SOC_BLK_ES:
		    block = ES_BLOCK(unit);
		    break;
		case SOC_BLK_FD:
		    block = FD_BLOCK(unit);
		    break;
		case SOC_BLK_FF:
		    block = FF_BLOCK(unit);
		    break;
		case SOC_BLK_FR:
		    block = FR_BLOCK(unit);
		    break;
		case SOC_BLK_TX:
		    block = TX_BLOCK(unit);
		    break;
		case SOC_BLK_QMA:
		    block = QMA_BLOCK(unit);
		    break;
		case SOC_BLK_QMB:
		    block = QMB_BLOCK(unit);
		    break;
		case SOC_BLK_QMC:
		    block = QMC_BLOCK(unit);
		    break;
		case SOC_BLK_QSA:
		    block = QSA_BLOCK(unit);
		    break;
		case SOC_BLK_QSB:
		    block = QSB_BLOCK(unit);
		    break;
		case SOC_BLK_RB:
		    block = RB_BLOCK(unit);
		    break;
		case SOC_BLK_SC_TOP:
		    block = SC_TOP_BLOCK(unit);
		    break;
		case SOC_BLK_SF_TOP:
		    block = SF_TOP_BLOCK(unit);
		    break;
		case SOC_BLK_TS:
		    block = TS_BLOCK(unit);
		    break;
		case SOC_BLK_OTPC:
		    block = OTPC_BLOCK(unit);
		    break;
		default:
		    block = -1;			/* unknown non-port block */
		    break;
	    }
	}
    } else {
#endif

	if (!(regblktype & portblktype)) {
	    switch (regblktype) {
		case SOC_BLK_ARL:
		    block = ARL_BLOCK(unit);
		    break;
		case SOC_BLK_IPIPE:
		    block = IPIPE_BLOCK(unit);
		    break;
		case SOC_BLK_IPIPE_HI:
		    block = IPIPE_HI_BLOCK(unit);
		    break;
		case SOC_BLK_EPIPE:
		    block = EPIPE_BLOCK(unit);
		    break;
		case SOC_BLK_EPIPE_HI:
		    block = EPIPE_HI_BLOCK(unit);
		    break;
		case SOC_BLK_IGR:
		    block = IGR_BLOCK(unit);
		    break;
		case SOC_BLK_EGR:
		    block = EGR_BLOCK(unit);
		    break;
		case SOC_BLK_BSE: /* Aliased with IL */
                    if (SOC_IS_SHADOW(unit)) {
                        if (port == 9) {
                            block = IL0_BLOCK(unit);
                        } else if (port == 13) {
                            block = IL1_BLOCK(unit);
                        }
                        pindex = 0;
                    } else {
		        block = BSE_BLOCK(unit);
                    }
		    break;
		case SOC_BLK_CSE: /* Aliased with MS_ISEC */
                    if (SOC_IS_SHADOW(unit)) {
                        if (port >= 1 && port <= 4) {
                            block = MS_ISEC0_BLOCK(unit);
                            pindex = port - 1;
                        } else {
                            block = MS_ISEC1_BLOCK(unit);
                            pindex = port - 5;
                        }
                    } else {
		        block = CSE_BLOCK(unit);
                    }
		    break;
		case SOC_BLK_HSE: /* Aliased with MS_ESEC */
                    if (SOC_IS_SHADOW(unit)) {
                        if (port >= 1 && port <= 4) {
                            block = MS_ESEC0_BLOCK(unit);
                            pindex = port - 1;
                        } else {
                            block = MS_ESEC1_BLOCK(unit);
                            pindex = port - 5;
                        }
                    } else {
		        block = HSE_BLOCK(unit);
                    }
		    break;
		case SOC_BLK_BSAFE:
#if defined(BCM_TRX_SUPPORT) || defined(BCM_RAPTOR_SUPPORT)
		    
		    if (SOC_IS_TRX(unit) || SOC_IS_RAVEN(unit) || SOC_IS_HAWKEYE(unit)) {
			block = OTPC_BLOCK(unit);
		    } else
#endif
		    {
			block = BSAFE_BLOCK(unit);
		    }
		    break;
		case SOC_BLK_MMU:
		    block = MMU_BLOCK(unit);
		    break;
		case SOC_BLK_MCU:
		    block = MCU_BLOCK(unit);
		    break;
		case SOC_BLK_CMIC:
		    block = CMIC_BLOCK(unit);
		    break;
		case SOC_BLK_ESM:
		    block = ESM_BLOCK(unit);
		    break;
                case SOC_BLK_PORT_GROUP4:
                    block = port == 0 ? PG4X_BLOCK(unit) : PG4Y_BLOCK(unit);
                    break;
                case SOC_BLK_PORT_GROUP5:
                    block = port == 0 ? PG5X_BLOCK(unit) : PG5Y_BLOCK(unit);
                    break;
		default:
		    block = -1;			/* unknown non-port block */
		    break;
	    }
	}
#ifdef  BCM_SIRIUS_SUPPORT
    }
#endif

    assert(block >= 0);			/* block must be valid */

    /* determine final block, pindex, and index */
    gransh = 0;
    switch (SOC_REG_INFO(unit, reg).regtype) {
    case soc_cpureg:
	block = -1;
	pindex = 0;
	gransh = 2;			/* 4 byte granularity */
        break;
    case soc_portreg:
	if (!(regblktype & portblktype) &&
            !(SOC_IS_SHADOW(unit) && ((regblktype & SOC_BLK_MS_ISEC) || 
            (regblktype & SOC_BLK_MS_ESEC)))) {
            if (soc_feature(unit, soc_feature_logical_port_num) &&
                block == MMU_BLOCK(unit)) {
                phy_port = SOC_INFO(unit).port_l2p_mapping[port];
                pindex = SOC_INFO(unit).port_p2m_mapping[phy_port];
            } else {
                pindex = port;
            }
	}
        break;
    case soc_cosreg:
        assert(index >= 0 && index < NUM_COS(unit));
        pindex = index;
	index = 0;
        break;
    case soc_genreg:
	pindex = 0;
	break;
    default:
	assert(0);			/* unknown register type */
        break;
    }

    /* put together address: base|block|pindex + index */
    base = SOC_REG_INFO(unit, reg).offset;
	
    if (block >= 0) {
        base |= ((SOC_BLOCK2OFFSET(unit, block) & 0xf) << SOC_BLOCK_BP) |
            (((SOC_BLOCK2OFFSET(unit, block) >> 4) & 0x3) <<
             SOC_BLOCK_MSB_BP);
    }
    
    if (pindex) {
	base |= pindex << SOC_REGIDX_BP;
    }
    
    if (index && SOC_REG_ARRAY(unit, reg)) {
	assert(index >= 0 && index < SOC_REG_NUMELS(unit, reg));
	if (index && SOC_REG_ARRAY2(unit, reg)) {
	    base += ((index*2) << gransh);
	} else {
	    base += (index << gransh);
	}
    }

    return base;
}


int
soc_regaddrlist_alloc(soc_regaddrlist_t *addrlist)
{
    if ((addrlist->ainfo = sal_alloc(_SOC_MAX_REGLIST *
                sizeof(soc_regaddrinfo_t), "regaddrlist")) == NULL) {
        return SOC_E_MEMORY;
    }
    addrlist->count = 0;

    return SOC_E_NONE;
}

int
soc_regaddrlist_free(soc_regaddrlist_t *addrlist)
{
    if (addrlist->ainfo) {
        sal_free(addrlist->ainfo);
    }

    return SOC_E_NONE;
}

/*
 * Function:   soc_reg_fields32_modify
 * Purpose:    Modify the value of a fields in a register.
 * Parameters:
 *       unit         - (IN) SOC unit number.
 *       reg          - (IN) Register.
 *       port         - (IN) Port number.
 *       field_count  - (IN) Number of fields to modify.
 *       fields       - (IN) Modified fields array.
 *       values       - (IN) New value for each member of fields array.
 * Returns:
 *       BCM_E_XXX
 */
int
soc_reg_fields32_modify(int unit, soc_reg_t reg, soc_port_t port,
                        int field_count, soc_field_t *fields, uint32 *values)
{
    uint64 data64;      /* Current 64 bit register data.  */
    uint64 odata64;     /* Original 64 bit register data. */
    uint32 data32;      /* Current 32 bit register data.  */
    uint32 odata32;     /* Original 32 bit register data. */
    uint32 reg_addr;    /* Register address.              */
    int idx;            /* Iteration index.               */
    uint32 max_val;     /* Max value to fit the field     */
    int field_len;      /* Bit length of the field        */

    /* Check that register is a valid one for this unit. */
    if (!SOC_REG_IS_VALID(unit, reg)) {
        return SOC_E_PARAM;
    }

    /*  Fields & values sanity check. */
    for (idx = 0; idx < field_count; idx++) {
        if ((NULL == fields + idx) || (NULL == values + idx)) {
            return SOC_E_PARAM;
        }

        /* Make sure field is present in register. */
        if (!soc_reg_field_valid(unit, reg, fields[idx])) {
            return SOC_E_PARAM;
        }
        /* Make sure value can fit into field */
        field_len = soc_reg_field_length(unit, reg, fields[idx]);
        max_val = (field_len < 32) ? ((1 << field_len) - 1) : 0xffffffff;
        if (values[idx] > max_val) {
            return SOC_E_PARAM;
        }
    }

    /* Calculate register address. */
    reg_addr = soc_reg_addr(unit, reg, port, 0);

    if (SOC_REG_IS_64(unit, reg)) {

        /* Read current register value. */
        SOC_IF_ERROR_RETURN(soc_reg64_read(unit, reg_addr, &data64));
        odata64 = data64;

        /* Update fields with new values. */
        for (idx = 0; idx < field_count; idx ++) {
            soc_reg64_field32_set(unit, reg, &data64, fields[idx], values[idx]);
        }
        if (COMPILER_64_NE(data64, odata64)) {
            /* Write new register value back to hw. */
            SOC_IF_ERROR_RETURN(soc_reg64_write(unit, reg_addr, data64));
        }
    } else {
        if (soc_cpureg == SOC_REG_TYPE(unit,  reg)) {
            reg_addr = soc_reg_addr(unit, reg, REG_PORT_ANY, port);
            /* Read PCI register value. */
            SOC_IF_ERROR_RETURN(soc_pci_getreg(unit, reg_addr, &data32));
        } else { 
            SOC_IF_ERROR_RETURN(soc_reg32_read(unit, reg_addr, &data32));
        }
        
        odata32 = data32;

        for (idx = 0; idx < field_count; idx ++) {
            soc_reg_field_set(unit, reg, &data32, fields[idx], values[idx]);
        }
        if (data32 != odata32) {
            /* Write new register value back to hw. */
            if (soc_cpureg == SOC_REG_TYPE(unit,  reg)) {
                SOC_IF_ERROR_RETURN(soc_pci_write(unit, reg_addr, data32));
            } else {
                SOC_IF_ERROR_RETURN(soc_reg32_write(unit, reg_addr, data32));
            }
            
        }
    }
    return (SOC_E_NONE);
}

/*
 * Function:   soc_reg_field32_modify
 * Purpose:    Modify the value of a field in a register.
 * Parameters:
 *       unit      - (IN) SOC unit number.
 *       reg       - (IN) Register.
 *       port      - (IN) Port number.
 *       field     - (IN) Modified field.
 *       value     - (IN) New field value.
 * Returns:
 *       SOC_E_XXX
 */
int
soc_reg_field32_modify(int unit, soc_reg_t reg, soc_port_t port, 
                       soc_field_t field, uint32 value)
{
    return soc_reg_fields32_modify(unit, reg, port, 1, &field, &value);
}

/*
 * Function:   soc_reg_egress_cell_count_get
 * Purpose:    Retrieves the number of egress cells for a <port, cos> pair.
 * Parameters:
 *       unit      - (IN) SOC unit number.
 *       port      - (IN) Port number.
 *       cos       - (IN) COS queue.
 *       data      - (OUT) Cell count.
 * Returns:
 *       SOC_E_XXX
 */
int
soc_reg_egress_cell_count_get(int unit, soc_port_t port, int cos, uint32 *data)
{
    if (!SOC_PORT_VALID(unit, port) || cos < 0 || cos >= NUM_COS(unit)) {
        return SOC_E_PARAM;
    }
    SOC_IF_ERROR_RETURN(READ_COSLCCOUNTr(unit, port, cos, data));
    return SOC_E_NONE;
}

#endif /* defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) */
