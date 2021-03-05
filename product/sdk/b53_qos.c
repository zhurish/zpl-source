/*
 * b53_qos.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */



#include <zebra.h>
#include "b53_mdio.h"
#include "b53_regs.h"
#include "sdk_driver.h"


/*************************************************************************/
int b53125_qos_aggreation_mode(struct b53125_device *dev, ospl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_AGG_MODE;
	else
		port_ctrl &= 0x7f;//~B53_AGG_MODE;

	ret |= b53125_write8(dev, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, port_ctrl);
	return ret;
}

/*************************************************************************/
//设置使能基于端口的优先级(基于端口默认VLAN优先级)
int b53125_qos_base_port(struct b53125_device *dev, ospl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_PORT_QOS_EN;
	else
		port_ctrl &= ~B53_PORT_QOS_EN;

	ret |= b53125_write8(dev, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
//设置优先级映射规则
int b53125_qos_layer_sel(struct b53125_device *dev, int sel)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, &port_ctrl);
	port_ctrl &= ~(3<<B53_QOS_LAYER_SEL_S);
	port_ctrl |= (sel<<B53_QOS_LAYER_SEL_S);
/*
	if(enable)
		port_ctrl |= (sel<<B53_QOS_LAYER_SEL_S);
	else
		port_ctrl &= ~(sel<<B53_QOS_LAYER_SEL_S);
*/

	ret |= b53125_write8(dev, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
//禁止使能8021p优先级
int b53125_qos_8021p(struct b53125_device *dev, int port, ospl_bool enable)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev, B53_QOS_PAGE, B53_802_1P_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= BIT(port);
	else
		port_ctrl &= ~BIT(port);

	ret |= b53125_write16(dev, B53_QOS_PAGE, B53_802_1P_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
//禁止使能差分服务优先级
int b53125_qos_diffserv(struct b53125_device *dev, int port, ospl_bool enable)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev, B53_QOS_PAGE, B53_QOS_DIFFSERV_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= BIT(port);
	else
		port_ctrl &= ~BIT(port);

	ret |= b53125_write16(dev, B53_QOS_PAGE, B53_QOS_DIFFSERV_CTL, port_ctrl);
	return ret;
}

/*************************************************************************/
//设置端口到队列的映射（8021P优先级到COS优先级的映射）
int b53125_qos_port_map_queue(struct b53125_device *dev, int port, int p, int queue)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_QOS_PAGE, B53_PCP_TO_TC_PORT_CTL(port), &port_ctrl);
	port_ctrl &= ~(B53_TC_QUEUE_MASK<<B53_TC_TO_COS(p));
	//if(enable)
		port_ctrl |= (queue<<B53_TC_TO_COS(p));
	//else
	//	port_ctrl &= ~(p<<B53_TC_TO_COS(p));

	ret |= b53125_write32(dev, B53_QOS_PAGE, B53_PCP_TO_TC_PORT_CTL(port), port_ctrl);
	return ret;
}

/*************************************************************************/
//设置差分优先级到队列的映射
int b53125_qos_diffserv_map_queue(struct b53125_device *dev, int diffserv, int queue)
{
	int ret = 0;
	int reg = 0;
	u64 port_ctrl = 0;

	if(reg <= 0x0f)
		reg = B53_QOS_DIFFSERV_MAP_CTL0;
	else if(reg <= 0x1f)
		reg = B53_QOS_DIFFSERV_MAP_CTL1;
	else if(reg <= 0x2f)
		reg = B53_QOS_DIFFSERV_MAP_CTL2;
	else if(reg <= 0x3f)
		reg = B53_QOS_DIFFSERV_MAP_CTL3;

	ret |= b53125_read48(dev, B53_QOS_PAGE, reg, &port_ctrl);
	port_ctrl &= ~(B53_TC_QUEUE_MASK<<B53_DIFFSERV_TO_COS(diffserv));
	//if(enable)
		port_ctrl |= (queue<<B53_DIFFSERV_TO_COS(diffserv));
	//else
	//	port_ctrl &= ~(diffserv<<B53_DIFFSERV_TO_COS(diffserv));

	ret |= b53125_write48(dev, B53_QOS_PAGE, reg, port_ctrl);
	return ret;
}
/*************************************************************************/
//设置队列到class的映射
int b53125_qos_queue_map_class(struct b53125_device *dev, int queue, int class)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev, B53_QOS_PAGE, B53_QUEUE_TO_CLASS_CTL, &port_ctrl);
	port_ctrl &= ~(B53_TC_CLASS_MASK<<B53_QUEUE_TO_CLASS(queue));
	//if(enable)
		port_ctrl |= (class<<B53_QUEUE_TO_CLASS(queue));

	ret |= b53125_write16(dev, B53_QOS_PAGE, B53_QUEUE_TO_CLASS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
//设置class调度方式
int b53125_qos_class_scheduling(struct b53125_device *dev, int mode)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev, B53_QOS_PAGE, B53_TX_QUEUE_CTL, &port_ctrl);
	port_ctrl &= ~(B53_TC_CLASS_MASK);
	//if(enable)
		port_ctrl |= (mode);
/*
00 = all queues are weighted round robin
01 = COS3 is strict priority, COS2-COS0 are weighted round
robin.
10 = COS3 and COS2 is strict priority, COS1-COS0 are weighted
round robin.
11 = COS3, COS2, COS1 and COS0 are in strict priority.
 */
	ret |= b53125_write8(dev, B53_QOS_PAGE, B53_TX_QUEUE_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
//设置进入CPU报文到队列的映射
int b53125_qos_cpu_map_queue(struct b53125_device *dev, int traffic, ospl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_QOS_PAGE, B53_CPU_TO_QUEUE_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= (traffic);
	else
		port_ctrl &= ~(traffic);
	ret |= b53125_write32(dev, B53_QOS_PAGE, B53_CPU_TO_QUEUE_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
//设置class调度权限
int b53125_qos_class_weight(struct b53125_device *dev, int class, int weight)
{
	int ret = 0;
	u8 port_ctrl = 0;
	port_ctrl |= (weight);
	ret |= b53125_write8(dev, B53_QOS_PAGE, B53_TX_QUEUE_WEIGHT(class), port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_qos_class4_weight(struct b53125_device *dev, ospl_bool strict, int weight)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev, B53_QOS_PAGE, B53_COS4_SERVICE_WEIGHT, &port_ctrl);
	port_ctrl &= ~(1<<8);
	if(weight >= 0)
		port_ctrl |= (weight);
	port_ctrl |= (strict<<8);
	ret |= b53125_write16(dev, B53_QOS_PAGE, B53_COS4_SERVICE_WEIGHT, port_ctrl);
	return ret;
}
/*************************************************************************/
/*************************************************************************/
/* Broadcast Storm Suppression Register */
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
//禁止使能流量IPG
int b53125_qos_ingress_ipg(struct b53125_device *dev, ospl_bool tx, ospl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_BROADCAST_STROM_PAGE, B53_INGRESS_RATE_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= (tx ? B53_IPG_XLENEN_EG_EN:B53_IPG_XLENEN_EN);
	else
		port_ctrl &= ~(tx ? B53_IPG_XLENEN_EG_EN:B53_IPG_XLENEN_EN);

	ret |= b53125_write32(dev, B53_BROADCAST_STROM_PAGE, B53_INGRESS_RATE_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_qos_buck_mode(struct b53125_device *dev, int id, ospl_bool mode)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_BROADCAST_STROM_PAGE, B53_INGRESS_RATE_CTL, &port_ctrl);
	if(mode)
		port_ctrl |= (id ? B53_BUCK0_BRM_SEL_EN:B53_BUCK1_BRM_SEL_EN);
	else
		port_ctrl &= ~(id ? B53_BUCK0_BRM_SEL_EN:B53_BUCK1_BRM_SEL_EN);

	ret |= b53125_write32(dev, B53_BROADCAST_STROM_PAGE, B53_INGRESS_RATE_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_qos_buck_type(struct b53125_device *dev, int id, ospl_uint32 type)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_BROADCAST_STROM_PAGE, B53_INGRESS_RATE_CTL, &port_ctrl);
	if(id == 1)
	{
		port_ctrl &= ~(0x3f<<B53_BUCK1_PACKET_TYPE);
		port_ctrl |= (type<<B53_BUCK1_PACKET_TYPE);
	}
	else
	{
		port_ctrl &= ~(0x3f<<B53_BUCK0_PACKET_TYPE);
		port_ctrl |= (type<<B53_BUCK0_PACKET_TYPE);
	}
	ret |= b53125_write32(dev, B53_BROADCAST_STROM_PAGE, B53_INGRESS_RATE_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
//端口限速（网络风暴）
int b53125_qos_ingress_rate_mode(struct b53125_device *dev, int port,  ospl_uint32 type, ospl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), &port_ctrl);
	switch(type)
	{
	case 1:
		if(enable)
			port_ctrl |= (B53_STRM_SUPP_EN);
		else
			port_ctrl &= ~(B53_STRM_SUPP_EN);
		break;
	case 2:
		if(enable)
			port_ctrl |= (B53_RSVMC_SUPP_EN);
		else
			port_ctrl &= ~(B53_RSVMC_SUPP_EN);
		break;
	case 3:
		if(enable)
			port_ctrl |= (B53_BC_SUPP_EN);
		else
			port_ctrl &= ~(B53_BC_SUPP_EN);
		break;
	case 4:
		if(enable)
			port_ctrl |= (B53_MC_SUPP_EN);
		else
			port_ctrl &= ~(B53_MC_SUPP_EN);
		break;
	case 5:
		if(enable)
			port_ctrl |= (B53_DLF_SUPP_EN);
		else
			port_ctrl &= ~(B53_DLF_SUPP_EN);
		break;
	default:
		return ERROR;
	}
	ret |= b53125_write32(dev, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), port_ctrl);
	return ret;
}

int b53125_qos_ingress_rate(struct b53125_device *dev, int port, ospl_uint32 index, int bucket, int cnt)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), &port_ctrl);
	if(index == 0)
	{
		port_ctrl &= ~(B53_BUCKET_SIZE_MASK<<B53_BUCKET0_SIZE_S |
				B53_BUCKET_SIZE_MASK<<B53_BUCKET1_SIZE_S |
				B53_BUCKET_RATE_MASK<<B53_BUCKET1_RATE_CNT |
				B53_BUCKET_RATE_MASK);

		port_ctrl |= B53_BUCKET0_EN;
		port_ctrl |= (bucket & B53_BUCKET_SIZE_MASK)<<B53_BUCKET0_SIZE_S;
		port_ctrl |= (cnt & B53_BUCKET_RATE_MASK);
	}
	else if(index == 1)
	{
		port_ctrl &= ~(B53_BUCKET_SIZE_MASK<<B53_BUCKET0_SIZE_S |
				B53_BUCKET_SIZE_MASK<<B53_BUCKET1_SIZE_S |
				B53_BUCKET_RATE_MASK<<B53_BUCKET1_RATE_CNT |
				B53_BUCKET_RATE_MASK);

		port_ctrl |= B53_BUCKET1_EN;
		port_ctrl |= (bucket & B53_BUCKET_SIZE_MASK)<<B53_BUCKET1_SIZE_S;
		port_ctrl |= (cnt & B53_BUCKET_RATE_MASK)<<B53_BUCKET1_RATE_CNT;
	}
	ret |= b53125_write32(dev, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_qos_egress_rate(struct b53125_device *dev, int port, int bucket, int cnt)
{
	int ret = 0;
	u16 port_ctrl = 0;
	port_ctrl |= B53_ERC_EN;
	port_ctrl |= (bucket & B53_BKT_SZE_MASK)<<B53_BKT_SZE_S;
	port_ctrl |= (cnt & B53_RATE_CNT_MASK);
	ret |= b53125_write16(dev, B53_BROADCAST_STROM_PAGE, B53_PORT_EGRESS_RATE_CTL(port), port_ctrl);
	return ret;
}
/*************************************************************************/
/*************************************************************************/
static int cpu_rate_tbl(int rate)
{
	ospl_uint32 i = 0;
	int rate_tbl[] = { 384, 512, 639, 786, 1024, 1280, 1536, 1791, 2048, 2303, 2559, 2815,
		3328, 3840, 4352, 4863, 5376, 5887, 6400, 6911, 7936, 8960, 9984, 11008, 12030,
	    13054, 14076, 15105, 17146, 19201, 21240, 23299, 25354, 27382, 29446, 31486, 25561,
		39682, 42589, 56818, 71023, 85324, 99602, 113636, 127551, 142045, 312675, 284091,
		357143, 423929, 500000, 568182, 641026, 714286, 781250, 862069, 925069, 1000000,
		1086957, 1136364, 1190476, 1250000, 1315789, 1388889
	};
	for(i = 0; i < sizeof(rate_tbl)/sizeof(rate_tbl[0]); i++)
	{
		if(rate <= rate_tbl[i])
			break;
	}
	return i;
}
/*************************************************************************/
//CPU接口限速
int b53125_qos_cpu_rate(struct b53125_device *dev, int rate)
{
	int ret = 0;
	u8 port_ctrl = 0;
	port_ctrl = cpu_rate_tbl(rate) & B53_RATE_INDEX_MASK;
	ret |= b53125_write8(dev, B53_BROADCAST_STROM_PAGE, B53_IMP_PORT_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/* Traffic Remarking Register */
int b53125_qos_cfi_remarking(struct b53125_device *dev, int port, ospl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= (B53_CFI_REMARK_EN(port));
	else
		port_ctrl &= ~B53_CFI_REMARK_EN(port);
	ret |= b53125_write32(dev, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, port_ctrl);
	return ret;
}

int b53125_qos_pcp_remarking(struct b53125_device *dev, int port, ospl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= (B53_PCP_REMARK_EN(port));
	else
		port_ctrl &= ~B53_PCP_REMARK_EN(port);
	ret |= b53125_write32(dev, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, port_ctrl);
	return ret;
}

/* Jumbo Frame Control Register */
int b53125_jumbo_enable(struct b53125_device *dev, int port, ospl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_JUMBO_PAGE, dev->jumbo_pm_reg, &port_ctrl);
	if(port == dev->cpu_port)
	{
		if(enable)
			port_ctrl |= BIT(8);
		else
			port_ctrl &= ~BIT(8);
	}
	else
	{
		if(enable)
			port_ctrl |= BIT(port);
		else
			port_ctrl &= ~BIT(port);
	}
	ret |= b53125_write32(dev, B53_JUMBO_PAGE, dev->jumbo_pm_reg, port_ctrl);
	return ret;
}

int b53125_jumbo_size(struct b53125_device *dev, int size)
{
	u16 max_size = size;
	int ret = 0;
	ret |= b53125_write16(dev, B53_JUMBO_PAGE, dev->jumbo_size_reg, max_size);
	return ret;
}
