#ifndef __B53_VLAN_REGS_H__
#define __B53_VLAN_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif


/* Port VLAN Page */
#define B53_PVLAN_PAGE			0x31

/* VLAN Registers */
#define B53_VLAN_PAGE			0x34


/*************************************************************************
 * Port VLAN Registers
 *************************************************************************/
/* Port VLAN mask (16 bit) IMP port is always 8, also on 5325 & co */
#define B53_PVLAN_PORT(i)		((i) * 2)

/* Join all VLANs register (16 bit) */
#define B53_JOIN_ALL_VLAN_EN		0x50
#define B53_FORWARD_MASK(n)		    0x50|BIT(n)

/*************************************************************************
 * VLAN Page Registers
 *************************************************************************/

/* VLAN Control 0 (8 bit) */
#define B53_GLOBAL_8021Q			0x00
#define   IEEE8021Q_VLAN_EN			BIT(7)	/* 802.1Q VLAN Enabled */
#define   VLAN_LEARNING_MODE(n)		(n)<<5	/*  */
#define     VLAN_LEARNING_MODE_SVL	0	/* (Shared VLAN learning mode) (MAC hash ARL table). */
#define     VLAN_LEARNING_MODE_IVL	3	/* (Individual VLAN learning mode) (MAC and VID hash ARL table) */
#define     VLAN_LEARNING_MODE_LLLEGAL		1	/* lllegal setting */
/*Change 1Q VID to PVID.
0
1 =
• For a single-tag frame with VID not = 0,
change the VID to PVID.
• For a double-tag frame with outer VID not = 0,
change outer VID to PVID.
0 = No change for 1Q/ISP tag if VID is not 0.*/
#define   VLAN_CHANGE_PVID_EN			BIT(3)




/* VLAN Control 1 (8 bit) */
#define B53_GLOBAL_VLAN_CTRL1			0x01
#define   VLAN_MCST_UNTAG_CHECK_EN		BIT(6)
#define   VLAN_MCST_FWD_CHECK_EN		BIT(5)
#define   VLAN_RES_MCST_FWD_CHECK_EN	BIT(3)
#define   VLAN_RES_MCST_UNTAG_CHECK_EN	BIT(2)

/* VLAN Control 2 (8 bit) */
#define B53_GLOBAL_VLAN_CTRL2			0x02
#define   VLAN_GMVRP_UNTAG_CHECK_EN		BIT(6)
#define   VLAN_GMVRP_FWD_CHECK_EN		BIT(5)
#define   VLAN_IMP_FWD_BYPASS       	BIT(2)

/* VLAN Control 3 (8 bit when BCM5325, 16 bit else) */
#define B53_GLOBAL_VLAN_CTRL3			0x03
#define   VLAN_DROP_NOVID(n)		    BIT(n) /* When enabled, any frame without an IEEE 802.1Q tag is dropped by this port  */
//Bit 8 = IMP port.

/* VLAN Control 4 (8 bit) */
#define B53_GLOBAL_VLAN_CTRL4			0x05
#define   VLAN_ING_VID_CHECK_S		6
#define   VLAN_ING_VID_CHECK_MASK	(0x3 << VLAN_ING_VID_CHECK_S)

#define   VLAN_ING_VID_VIO_FWD		0 /* forward, but do not learn */
#define   VLAN_ING_VID_VIO_DROP		1 /* drop VID violations */
#define   VLAN_NO_ING_VID_CHK		2 /* do not check */
#define   VLAN_ING_VID_VIO_TO_IMP	3 /* redirect to MII port */

#define   VLAN_GVRP_FWD_IMP_EN		BIT(5)
#define   VLAN_GMRP_FWD_IMP_EN		BIT(4)
#define   VLAN_DOUBLE_TAG_EN		BIT(2)
#define   VLAN_RSV_MCAST_FLOOD		BIT(1)


/* VLAN Control 5 (8 bit) */
#define B53_GLOBAL_VLAN_CTRL5			0x06
/*Tag Status Preserve:1 = Regardless of untag map in VLAN table, non-1Q
frames (including 802.1p frames) will not be changed
at TX (egress).
This field has no effect in double-tagging mode
(DT_Mode).*/
#define   VLAN_TAG_STATUS_PRESERVE		BIT(6) 
/*
1 = Egress directed frames issued from the IMP port
bypass trunk checking.
0 = Egress directed frames issued from the IMP port
are subject to trunk checking and redirection.
*/
#define   VLAN_TRUNK_CHECK_BYPASS		BIT(5) 
#define   VLAN_DROP_VID_FFF_EN		BIT(2)
#define   VLAN_DROP_VID_INVALID		BIT(3)

/* VLAN Multiport Address Control Register (16 bit) */
#define B53_VLAN_MULTIPORT			0x0A


/* VLAN Port Default Tag (16 bit) */
#define B53_VLAN_PORT_DEF_TAG(i)	(0x10 + 2 * (i))

/* Double Tagging TPID Register */
#define B53_VLAN_ISP_TPID	(0x30)

/* ISP Port Selection Portmap Register */
#define B53_VLAN_ISP_PORT	(0x32)



 
#ifdef __cplusplus
}
#endif


#endif /* __B53_VLAN_REGS_H__ */
