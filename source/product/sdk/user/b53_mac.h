#ifndef __B53_MAC_REGS_H__
#define __B53_MAC_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif

#define B53_ARLCTRL_PAGE		0x04 /* ARL Control */
#define B53_ARLIO_PAGE			0x05 /* ARL Access */



/*************************************************************************
 * ARL Access Page Registers
 *************************************************************************/

/* VLAN Table Access Register (8 bit) */
#define B53_VLAN_TBL_ACCESS			0x80
#define   VLAN_TBL_CMD_WRITE			0
#define   VLAN_TBL_CMD_READ			1
#define   VLAN_TBL_CMD_CLEAR			2
#define   VLAN_TBL_START_CMD			BIT(7)

/* VLAN Table Index Register (16 bit) */
#define B53_VLAN_TBL_INDEX			0x81

/* VLAN Table Entry Register (32 bit) */
#define B53_VLAN_TBL_ENTRY			0x83
#define   VLAN_FWD_MODE_VLAN			BIT(21)
#define   VLAN_TBL_MSTP_INDEX_OFFSET		18
#define   VLAN_TBL_MSTP_INDEX_MASK		3
#define   VLAN_UNTAG_OFFSET			9
#define   VLAN_VID_MASK			0x01FF

/*****************MAC********************/
/* ARL Table Read/Write Register (8 bit) */
#define B53_ARLTBL_RW_CTRL		0x00
#define    ARLTBL_RW			BIT(0)
#define    ARLTBL_START_DONE		BIT(7)
#define    ARLTBL_IVL_SVL_SELECT		BIT(6)
/* MAC Address Index Register (48 bit) */
#define B53_MAC_ADDR_IDX		0x02

/* VLAN ID Index Register (16 bit) */
#define B53_VLAN_ID_IDX			0x08

/* ARL Table MAC/VID Entry N Registers (64 bit)
 *
 * BCM5325 and BCM5365 share most definitions below
 */
#define B53_ARLTBL_MAC_VID_ENTRY(n)	(0x10 * (n))+0x10
#define   ARLTBL_MAC_MASK		0xffffffffffffULL
#define   ARLTBL_VID_S			48
#define   ARLTBL_VID_MASK		0xfff


/* ARL Table Data Entry N Registers (32 bit) */
#define B53_ARLTBL_DATA_ENTRY(n)	((0x10 * (n)) + 0x18)
#define   ARLTBL_DATA_PORT_ID_MASK	0x1ff
#define   ARLTBL_TC(tc)			((3 & tc) << 11)
#define   ARLTBL_AGE			BIT(14)
#define   ARLTBL_STATIC			BIT(15)
#define   ARLTBL_VALID			BIT(16)
/* Maximum number of bin entries in the ARL for all switches */
#define B53_ARLTBL_MAX_BIN_ENTRIES	4

/* ARL Search Control Register (8 bit) */
#define B53_ARL_SRCH_CTL		0x50
#define   ARL_SRCH_VLID			BIT(0)
#define   ARL_SRCH_STDN			BIT(7)

/* ARL Search Address Register (16 bit) */
#define B53_ARL_SRCH_ADDR		0x51
#define  ARL_ADDR_MASK			0x3fff

/* ARL Search MAC/VID Result (64 bit) */
#define B53_ARL_SRCH_RSTL_0_MACVID	0x60

/* ARL Search Data Result (32 bit) */
#define B53_ARL_SRCH_RSTL_0		0x68

#define B53_ARL_SRCH_RSTL_MACVID(x)	(B53_ARL_SRCH_RSTL_0_MACVID + ((x) * 0x10))
#define B53_ARL_SRCH_RSTL(x)		(B53_ARL_SRCH_RSTL_0 + ((x) * 0x10))
/*ARL Table Search MAC/VID Result*/
#define   ARL_SRCH_MAC_MASK		0xffffffffffffULL
#define   ARL_SRCH_VID_S			48
#define   ARL_SRCH_VID_MASK		0xfff
/*ARL Table Search Data Result*/
#define   ARL_SRCH_DATA_PORT_ID_MASK	0x1ff
#define   ARL_SRCH_TC(tc)			((3 & tc) << 11)
#define   ARL_SRCH_AGE			BIT(14)
#define   ARL_SRCH_STATIC			BIT(15)
#define   ARL_SRCH_VALID			BIT(16)

/*************************************************************************
 * ARL I/O Registers
 *************************************************************************/

/* Aging Time Control Register */
#define B53_AGING_TIME			0x03
#define   BRCM_AGE_CHANGE_EN	BIT(20) /* Enable tagging on port 8 */


/* Fast Aging Control register (8 bit) */
#define B53_FAST_AGE_CTRL		0x88
#define   FAST_AGE_STATIC		BIT(0)
#define   FAST_AGE_DYNAMIC		BIT(1)
#define   FAST_AGE_PORT			BIT(2)
#define   FAST_AGE_VLAN			BIT(3)
#define   FAST_AGE_STP			BIT(4)
#define   FAST_AGE_MC			BIT(5)
#define   FAST_AGE_DONE			BIT(7)

/* Fast Aging Port Control register (8 bit) */
#define B53_FAST_AGE_PORT_CTRL		0x89

/* Fast Aging VID Control register (16 bit) */
#define B53_FAST_AGE_VID_CTRL		0x8a


/* Reserved Multicast Control Register */
#define B53_MULTICAST_LEARNING			0x2F
#define  	B53_MC_LEARN_EN			BIT(7)
#define  	B53_BPDU_20				BIT(4)
#define  	B53_BPDU_11				BIT(3)
#define  	B53_BPDU_10				BIT(2)
#define  	B53_BPDU_02				BIT(1)
#define  	B53_BPDU_00				BIT(0)
/* (16 bit) */
#define B53_UC_FLOOD_MASK		0x32
#define B53_MC_FLOOD_MASK		0x34
#define B53_IPMC_FLOOD_MASK		0x36


#ifdef __cplusplus
}
#endif


#endif /* __B53_MAC_REGS_H__ */