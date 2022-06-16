#ifndef __B53_PORT_REGS_H__
#define __B53_PORT_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* MIB registers */
#define B53_MIB_PAGE(i)			(0x20 + (i))

#define B53_PORT_MII_PAGE(i)		(0x10 + (i)) /* Port i MII Registers */

/* Port Control Register (8 bit) */
#define B53_PORT_CTRL(i)		(0x00 + (i))
#define   PORT_CTRL_RX_DISABLE		BIT(0)
#define   PORT_CTRL_TX_DISABLE		BIT(1)
#define   PORT_CTRL_RX_BCST_EN		BIT(2) /* Broadcast RX (P8 only) */
#define   PORT_CTRL_RX_MCST_EN		BIT(3) /* Multicast RX (P8 only) */
#define   PORT_CTRL_RX_UCST_EN		BIT(4) /* Unicast RX (P8 only) */
#define	  PORT_CTRL_STP_STATE_S		5
#define   PORT_CTRL_NO_STP		(0 << PORT_CTRL_STP_STATE_S)
#define   PORT_CTRL_DIS_STATE		(1 << PORT_CTRL_STP_STATE_S)
#define   PORT_CTRL_BLOCK_STATE		(2 << PORT_CTRL_STP_STATE_S)
#define   PORT_CTRL_LISTEN_STATE	(3 << PORT_CTRL_STP_STATE_S)
#define   PORT_CTRL_LEARN_STATE		(4 << PORT_CTRL_STP_STATE_S)
#define   PORT_CTRL_FWD_STATE		(5 << PORT_CTRL_STP_STATE_S)
#define   PORT_CTRL_STP_STATE_MASK	(0x7 << PORT_CTRL_STP_STATE_S)



/*
 * Override Ports 0-7 State on devices with xMII interfaces (8 bit)
 *
 * For port 8 still use B53_PORT_OVERRIDE_CTRL
 * Please note that not all ports are available on every hardware, e.g. BCM5301X
 * don't include overriding port 6, BCM63xx also have some limitations.
 */
#define B53_GMII_PORT_OVERRIDE_CTRL(i)	(0x58 + (i))
#define   GMII_PO_LINK			BIT(0)
#define   GMII_PO_FULL_DUPLEX		BIT(1) /* 0 = Half Duplex */
#define   GMII_PO_SPEED_S		2
#define   GMII_PO_SPEED_10M		(0 << GMII_PO_SPEED_S)
#define   GMII_PO_SPEED_100M		(1 << GMII_PO_SPEED_S)
#define   GMII_PO_SPEED_1000M		(2 << GMII_PO_SPEED_S)
#define   GMII_PO_RX_FLOW		BIT(4)
#define   GMII_PO_TX_FLOW		BIT(5)
#define   GMII_PO_EN			BIT(6) /* Use the register contents */
#define   GMII_PO_SPEED_2000M		BIT(7) /* BCM5301X only, requires setting 1000M */


/* Pause Frame Detection Control Register (8 bit) */
#define B53_PAUSE_FRAME_DETECTION		0x80
#define   PAUSE_IGNORE_DA		BIT(0)


#define B53_PORT_OVERRIDE_CTRL		0x0e
#define   PORT_OVERRIDE_LINK		BIT(0)
#define   PORT_OVERRIDE_FULL_DUPLEX	BIT(1) /* 0 = Half Duplex */
#define   PORT_OVERRIDE_SPEED_S		2
#define   PORT_OVERRIDE_SPEED_10M	(0 << PORT_OVERRIDE_SPEED_S)
#define   PORT_OVERRIDE_SPEED_100M	(1 << PORT_OVERRIDE_SPEED_S)
#define   PORT_OVERRIDE_SPEED_1000M	(2 << PORT_OVERRIDE_SPEED_S)
#define   PORT_OVERRIDE_RV_MII_25	BIT(4) /* BCM5325 only */
#define   PORT_OVERRIDE_RX_FLOW		BIT(4)
#define   PORT_OVERRIDE_TX_FLOW		BIT(5)
#define   PORT_OVERRIDE_SPEED_2000M	BIT(6) /* BCM5301X only, requires setting 1000M */
#define   PORT_OVERRIDE_EN		BIT(7) /* Use the register contents */


/*************************************************************************
 * Status Page registers
 *************************************************************************/

/* Link Status Summary Register (16bit) */
#define B53_LINK_STAT			0x00

/* Link Status Change Register (16 bit) */
#define B53_LINK_STAT_CHANGE		0x02

/* Port Speed Summary Register (16 bit for FE, 32 bit for GE) */
#define B53_SPEED_STAT			0x04
#define  SPEED_PORT_FE(reg, port)	(((reg) >> (port)) & 1)
#define  SPEED_PORT_GE(reg, port)	(((reg) >> 2 * (port)) & 3)
#define  SPEED_STAT_10M			0
#define  SPEED_STAT_100M		1
#define  SPEED_STAT_1000M		2

/* Duplex Status Summary (16 bit) */
#define B53_DUPLEX_STAT_FE		0x06
#define B53_DUPLEX_STAT_GE		0x08
#define B53_DUPLEX_STAT_63XX		0x0c

/* Pause Capability Register */
#define B53_PAUSE_CAP			0x28
#define  	B53_OVERIDE_EN			BIT(23)
#define  	EN_RX_PAUSE_CAP			9
#define  	EN_TX_PAUSE_CAP			0
#define  	EN_PAUSE_CAP_MASK		0XFF

/* IP Multicast control (8 bit) */
#define B53_IP_MULTICAST_CTRL		0x21
#define  B53_INRANGE_ERR_DIS	BIT(2)
#define  B53_OUTRANGE_ERR_DIS	BIT(1)
#define  B53_UC_FWD_EN			BIT(6)
#define  B53_MC_FWD_EN			BIT(7)


/* Protected Port Selection Register*/
#define B53_PROTECTED_CTRL			0x24

/* WAN Port Select Register */
#define B53_WAN_CTRL			0x26



/* Pause Pass Through for TX Register */
#define B53_PAUSE_PASS_TX			0x3a
/* Pause Pass Through for RX Register */
#define B53_PAUSE_PASS_RX			0x3c


#define B53_DIS_LEARNING		0x3c
#define B53_SOFTWARE_LEARNING		0x3e


/* MII Control Registers */
#define B53_MII_CTL			0x00
#define B53_MII_STAT		0x02
#define 	B53_INTERNAL_LOOPBACK		BIT(14)
#define 	B53_AUTO_NEGOTIATION		BIT(12)
#define 	B53_POWERDOWN				BIT(11)
#define 	B53_DUPLEX_MODE				BIT(8)




#ifdef __cplusplus
}
#endif


#endif /* __B53_PORT_REGS_H__ */
