/*
 * b53_qos.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */




#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_qos.h"

/*************************************************************************/
static int b53125_qos_aggreation_mode(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 regval = 0;
	ret |= b53125_read8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, regval);
	if(enable)
		regval |= B53_AGG_MODE;
	else
		regval &= 0x7f;//~B53_AGG_MODE;

	ret |= b53125_write8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, regval);
	sdk_handle_return(ret);
	ret |= b53125_rate_default(dev);
	return ret;
}

/*************************************************************************/
//设置使能基于端口的优先级(基于端口默认VLAN优先级)
static int b53125_qos_base_port(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 regval = 0;
	ret |= b53125_read8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, regval);
	if(enable)
		regval |= B53_PORT_QOS_EN;
	else
		regval &= ~B53_PORT_QOS_EN;

	ret |= b53125_write8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}
/*************************************************************************/
//设置优先级映射规则
static int b53125_qos_layer_sel(sdk_driver_t *dev, int sel)
{
	int ret = 0;
	u8 regval = 0;
	ret |= b53125_read8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, regval);
	regval &= ~(3<<B53_QOS_LAYER_SEL_S);
	regval |= (sel<<B53_QOS_LAYER_SEL_S);
/*
	if(enable)
		regval |= (sel<<B53_QOS_LAYER_SEL_S);
	else
		regval &= ~(sel<<B53_QOS_LAYER_SEL_S);
*/

	ret |= b53125_write8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}
/*************************************************************************/
//禁止使能8021p优先级
static int b53125_qos_8021p(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 regval = 0;
	ret |= b53125_read16(dev->sdk_device, B53_QOS_PAGE, B53_802_1P_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_802_1P_CTL, regval);
	if(enable)
		regval |= BIT(port);
	else
		regval &= ~BIT(port);

	ret |= b53125_write16(dev->sdk_device, B53_QOS_PAGE, B53_802_1P_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_802_1P_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}
/*************************************************************************/
//禁止使能差分服务优先级
static int b53125_qos_diffserv(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 regval = 0;
	ret |= b53125_read16(dev->sdk_device, B53_QOS_PAGE, B53_QOS_DIFFSERV_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_QOS_DIFFSERV_CTL, regval);
	if(enable)
		regval |= BIT(port);
	else
		regval &= ~BIT(port);

	ret |= b53125_write16(dev->sdk_device, B53_QOS_PAGE, B53_QOS_DIFFSERV_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_QOS_DIFFSERV_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_qos_trust_enable_type(sdk_driver_t *dev, zpl_phyport_t port, nsm_qos_trust_e type, zpl_bool enable)
{
	if(type == NSM_QOS_TRUST_COS)
		return b53125_qos_8021p(dev,  port,  enable);
	if(type == NSM_QOS_TRUST_DSCP)
		return b53125_qos_diffserv(dev,  port,  enable);
	return NO_SDK;
}

/*************************************************************************/
//设置端口到队列的映射（8021P优先级到COS优先级的映射）
static int b53125_8021p_map_priority(sdk_driver_t *dev, zpl_phyport_t port, int p, int priority)
{
	int ret = 0;
	u32 regval = 0;
	ret |= b53125_read32(dev->sdk_device, B53_QOS_PAGE, B53_PCP_TO_TC_PORT_CTL(port), &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_PCP_TO_TC_PORT_CTL(port), regval);
	regval &= ~(B53_TC_MASK<<B53_PCP_TO_TC(p));
	regval |= (priority<<B53_PCP_TO_TC(p));
	ret |= b53125_write32(dev->sdk_device, B53_QOS_PAGE, B53_PCP_TO_TC_PORT_CTL(port), regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_PCP_TO_TC_PORT_CTL(port), regval);
	sdk_handle_return(ret);
	return ret;
}

/*************************************************************************/
//设置差分优先级到队列的映射
static int b53125_diffserv_map_priority(sdk_driver_t *dev, zpl_phyport_t port, int diffserv, int priority)
{
	int ret = 0;
	int reg = 0;
	u64 regval = 0;

	if(diffserv <= 0x0f)
		reg = B53_QOS_DIFFSERV_MAP_CTL0;
	else if(diffserv <= 0x1f)
		reg = B53_QOS_DIFFSERV_MAP_CTL1;
	else if(diffserv <= 0x2f)
		reg = B53_QOS_DIFFSERV_MAP_CTL2;
	else if(diffserv <= 0x3f)
		reg = B53_QOS_DIFFSERV_MAP_CTL3;

	ret |= b53125_read48(dev->sdk_device, B53_QOS_PAGE, reg, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, reg, regval);
	regval &= ~(B53_TC_MASK<<B53_DIFFSERV_TO_TC(diffserv));
	regval |= (priority<<B53_DIFFSERV_TO_TC(diffserv));
	ret |= b53125_write48(dev->sdk_device, B53_QOS_PAGE, reg, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, reg, regval);
	sdk_handle_return(ret);
	return ret;
}
/*************************************************************************/
//设置内部优先级到队列的映射
static int b53125_tc_map_queue(sdk_driver_t *dev, zpl_phyport_t port, int priority, int queue)
{
	int ret = 0;
	u16 regval = 0;
	ret |= b53125_read16(dev->sdk_device, B53_QOS_PAGE, B53_TC_TO_QUEUE_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_TC_TO_QUEUE_CTL, regval);
	regval &= ~(B53_TC_QUEUE_MASK<<B53_TC_TO_QUEUE(priority));
	regval |= (queue<<B53_TC_TO_QUEUE(priority));

	ret |= b53125_write16(dev->sdk_device, B53_QOS_PAGE, B53_TC_TO_QUEUE_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_TC_TO_QUEUE_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}
/*************************************************************************/
/*************************************************************************/
//设置queue调度方式
static int b53125_queue_scheduling(sdk_driver_t *dev, zpl_phyport_t port, int mode)
{
	int ret = 0;
	u8 regval = 0;
	ret |= b53125_read8(dev->sdk_device, B53_QOS_PAGE, B53_TX_QUEUE_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_TX_QUEUE_CTL, regval);
	regval &= ~(B53_TC_QUEUE_MASK);
	//if(enable)
		regval |= (mode);
/*
00 = all queues are weighted round robin
01 = COS3 is strict priority, COS2-COS0 are weighted round
robin.
10 = COS3 and COS2 is strict priority, COS1-COS0 are weighted
round robin.
11 = COS3, COS2, COS1 and COS0 are in strict priority.
 */
	ret |= b53125_write8(dev->sdk_device, B53_QOS_PAGE, B53_TX_QUEUE_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_TX_QUEUE_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}
/*************************************************************************/
//设置queue调度权限
static int b53125_queue_weight(sdk_driver_t *dev, zpl_phyport_t port, int queue, int weight)
{
	int ret = 0;
	u8 regval = 0;
	regval |= (weight);
	ret |= b53125_write8(dev->sdk_device, B53_QOS_PAGE, B53_TX_QUEUE_WEIGHT(queue), regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_TX_QUEUE_WEIGHT(queue), regval);
	sdk_handle_return(ret);
	return ret;
}
/*************************************************************************/
int b53125_qos_class4_weight(sdk_driver_t *dev, zpl_bool strict, int weight)
{
	int ret = 0;
	u16 regval = 0;
	ret |= b53125_read16(dev->sdk_device, B53_QOS_PAGE, B53_COS4_SERVICE_WEIGHT, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_COS4_SERVICE_WEIGHT, regval);
	regval &= ~(1<<8);
	if(weight >= 0)
		regval |= (weight);
	regval |= (strict<<8);
	ret |= b53125_write16(dev->sdk_device, B53_QOS_PAGE, B53_COS4_SERVICE_WEIGHT, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_COS4_SERVICE_WEIGHT, regval);
	sdk_handle_return(ret);
	return ret;
}

/*************************************************************************/
/*************************************************************************/
//设置进入CPU报文到队列的映射
static int b53125_qos_cpu_queue_init(sdk_driver_t *dev)
{
	int ret = 0;
	u32 regval = 0;

	regval &= (B53_CPU_TC_QUEUE_MASK<<B53_CPU_TC_PROTO_FLOOD);
	regval |= (B53_CPU_TC_TO_QUEUE(B53_CPU_PROTO_FLOOD_QUEUE)<<B53_CPU_TC_PROTO_FLOOD);

	regval &= (B53_CPU_TC_QUEUE_MASK<<B53_CPU_TC_PROTO_SNOOP);
	regval |= (B53_CPU_TC_TO_QUEUE(B53_CPU_PROTO_SNOOP_QUEUE)<<B53_CPU_TC_PROTO_SNOOP);

	regval &= (B53_CPU_TC_QUEUE_MASK<<B53_CPU_TC_PROTO_TERM);
	regval |= (B53_CPU_TC_TO_QUEUE(B53_CPU_PROTO_TERM_QUEUE)<<B53_CPU_TC_PROTO_TERM);

	regval &= (B53_CPU_TC_QUEUE_MASK<<B53_CPU_TC_SWITCH);
	regval |= (B53_CPU_TC_TO_QUEUE(B53_CPU_SWITCH_QUEUE)<<B53_CPU_TC_SWITCH);

	regval &= (B53_CPU_TC_QUEUE_MASK<<B53_CPU_TC_SALEARN);
	regval |= (B53_CPU_TC_TO_QUEUE(B53_CPU_SALEARN_QUEUE)<<B53_CPU_TC_SALEARN);

	regval &= (B53_CPU_TC_QUEUE_MASK<<B53_CPU_TC_MIRROR);
	regval |= (B53_CPU_TC_TO_QUEUE(B53_CPU_MIRROR_QUEUE)<<B53_CPU_TC_MIRROR);

	ret |= b53125_write32(dev->sdk_device, B53_QOS_PAGE, B53_CPU_TO_QUEUE_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_CPU_TO_QUEUE_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}


/*************************************************************************/
/*************************************************************************/
/* Traffic Remarking Register */
static int b53125_qos_cfi_remarking(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u32 regval = 0;
	ret |= b53125_read32(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, regval);
	if(enable)
		regval |= (B53_CFI_REMARK_EN(port));
	else
		regval &= ~B53_CFI_REMARK_EN(port);
	ret |= b53125_write32(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_qos_pcp_remarking(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u32 regval = 0;
	ret |= b53125_read32(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, regval);
	if(enable)
		regval |= (B53_PCP_REMARK_EN(port));
	else
		regval &= ~B53_PCP_REMARK_EN(port);
	ret |= b53125_write32(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_priority_remarking(sdk_driver_t *dev, zpl_phyport_t port, zpl_uint32 tc, zpl_uint32 priority)
{
	int ret = 0;
	u64 regval = 0;
	ret |= b53125_read64(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TX_TO_PCP_CTL(port), &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TC_REMARK_PAGE, B53_TX_TO_PCP_CTL(port), regval);
	if(regval & 0xffffffff)
	{
		b53125_qos_cfi_remarking(dev,  port, zpl_true);
		b53125_qos_pcp_remarking(dev,  port, zpl_true);
	}

	regval &= ~(B53_TC_PCP_MASK<<B53_TC_PCP_PRI(tc));
	regval |= (priority<<B53_TC_PCP_PRI(tc));

	ret |= b53125_write64(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TX_TO_PCP_CTL(port), regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TC_REMARK_PAGE, B53_TX_TO_PCP_CTL(port), regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_priority_remarking_default(sdk_driver_t *dev, zpl_phyport_t port)
{
	int ret = 0;
	u64 regval = 0;
	zpl_uint32 tc = 0;
	zpl_uint32 priority;
	for(priority = 0; priority < 8; priority++)
	{
		tc = priority;
		regval &= ~(B53_TC_PCP_MASK<<B53_TC_PCP_PRI(tc));
		regval |= (priority<<B53_TC_PCP_PRI(tc));
	}
	ret |= b53125_write64(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TX_TO_PCP_CTL(port), regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TC_REMARK_PAGE, B53_TX_TO_PCP_CTL(port), regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_qos_pcp_map_default(sdk_driver_t *dev, zpl_phyport_t port)
{
	int ret = 0;
	u32 regval = 0;
	regval |= (0<<B53_PCP_TO_TC(0));
	regval |= (1<<B53_PCP_TO_TC(1));
	regval |= (2<<B53_PCP_TO_TC(2));
	regval |= (3<<B53_PCP_TO_TC(3));
	regval |= (4<<B53_PCP_TO_TC(4));
	regval |= (5<<B53_PCP_TO_TC(5));
	regval |= (6<<B53_PCP_TO_TC(6));
	regval |= (7<<B53_PCP_TO_TC(7));
	ret |= b53125_write32(dev->sdk_device, B53_QOS_PAGE, B53_PCP_TO_TC_PORT_CTL(port), regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_PCP_TO_TC_PORT_CTL(port), regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_qos_diffserv_map_default(sdk_driver_t *dev)
{
	int ret = 0, diffserv = 0, priority = 0;
	int reg = 0;
	u64 regval = 0;
	for(diffserv = 0; diffserv < 64; diffserv++)
	{
		if(diffserv >= 56 && diffserv <= 63)
			priority = 7;
		else if(diffserv >= 48 && diffserv <= 55)
			priority = 6;
		else if(diffserv >= 40 && diffserv <= 47)
			priority = 5;
		else if(diffserv >= 32 && diffserv <= 39)
			priority = 4;
		else if(diffserv >= 24 && diffserv <= 31)
			priority = 3;
		else if(diffserv >= 16 && diffserv <= 23)
			priority = 2;
		else if(diffserv >= 8 && diffserv <= 15)
			priority = 1;
		else if(diffserv >= 0 && diffserv <= 7)
			priority = 0;

		regval &= ~(B53_TC_MASK<<B53_DIFFSERV_TO_TC(diffserv));
		regval |= (priority<<B53_DIFFSERV_TO_TC(diffserv));

		if(diffserv == 0x0f)
		{
			reg = B53_QOS_DIFFSERV_MAP_CTL0;
			ret |= b53125_write48(dev->sdk_device, B53_QOS_PAGE, reg, regval);
		}
		else if(diffserv == 0x1f)
		{
			reg = B53_QOS_DIFFSERV_MAP_CTL1;
			ret |= b53125_write48(dev->sdk_device, B53_QOS_PAGE, reg, regval);
		}
		else if(diffserv == 0x2f)
		{
			reg = B53_QOS_DIFFSERV_MAP_CTL2;
			ret |= b53125_write48(dev->sdk_device, B53_QOS_PAGE, reg, regval);
		}
		else if(diffserv == 0x3f)
		{
			reg = B53_QOS_DIFFSERV_MAP_CTL3;
			ret |= b53125_write48(dev->sdk_device, B53_QOS_PAGE, reg, regval);
		}	
		sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, reg, regval);
	}
	sdk_handle_return(ret);
	return ret;
}

static int b53125_qos_tc_map_default(sdk_driver_t *dev)
{
	int ret = 0;
	u16 regval = 0;
	int priority = 0;
	int queue = 0;
	for(priority = 0; priority < 8; priority++)
	{
		if(priority>=0 && priority<2)
			queue = 0;
		if(priority>=2 && priority<4)
			queue = 1;
		if(priority>=4 && priority<6)
			queue = 2;
		if(priority>=6 && priority<8)
			queue = 3;
		regval &= ~(B53_TC_QUEUE_MASK<<B53_TC_TO_QUEUE(priority));
		regval |= (queue<<B53_TC_TO_QUEUE(priority));
	}
	ret |= b53125_write16(dev->sdk_device, B53_QOS_PAGE, B53_TC_TO_QUEUE_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_QOS_PAGE, B53_TC_TO_QUEUE_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_qos_map_default(sdk_driver_t *dev)
{
	int ret = 0;
	ret |= b53125_qos_pcp_map_default(dev, 0);
	ret |= b53125_qos_pcp_map_default(dev, 1);
	ret |= b53125_qos_pcp_map_default(dev, 2);
	ret |= b53125_qos_pcp_map_default(dev, 3);
	ret |= b53125_qos_pcp_map_default(dev, 4);
	ret |= b53125_qos_pcp_map_default(dev, 5);
	ret |= b53125_qos_pcp_map_default(dev, 6);//IMP Port
	ret |= b53125_qos_diffserv_map_default(dev);
	ret |= b53125_qos_tc_map_default(dev);
	ret |= b53125_priority_remarking_default(dev, 0);
	ret |= b53125_priority_remarking_default(dev, 1);
	ret |= b53125_priority_remarking_default(dev, 2);
	ret |= b53125_priority_remarking_default(dev, 3);
	ret |= b53125_priority_remarking_default(dev, 4);
	ret |= b53125_priority_remarking_default(dev, 5);
	//ret |= b53125_priority_remarking_default(dev, 6);
	return ret;	
}

static int b53125_qos_map_default_cb (sdk_driver_t *dev, zpl_phyport_t port, zpl_uint32 dif)
{
	int ret = 0;
	if(dif)
		ret |= b53125_qos_diffserv_map_default(dev);
	else
		ret |= b53125_qos_pcp_map_default(dev, port);
	return ret;
}

static int b53125_qos_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	ret |= b53125_qos_aggreation_mode(dev, zpl_true);
	ret |= b53125_qos_base_port(dev, zpl_false);
	ret |= b53125_qos_layer_sel(dev, 3);
	ret |= b53125_qos_8021p(dev, 1, zpl_true);
	ret |= b53125_qos_8021p(dev, 2, zpl_true);
	ret |= b53125_qos_8021p(dev, 3, zpl_true);
	ret |= b53125_qos_8021p(dev, 4, zpl_true);
	ret |= b53125_qos_8021p(dev, 5, zpl_true);
	ret |= b53125_qos_8021p(dev, 6, zpl_true);
	ret |= b53125_qos_8021p(dev, 7, zpl_true);
	ret |= b53125_qos_8021p(dev, 8, zpl_true);

	ret |= b53125_qos_map_default(dev);

	sdk_handle_return(ret);
	return ret;	
}


int b53125_qos_init(sdk_driver_t *dev)
{
	b53125_qos_cpu_queue_init(dev);
	//禁止使能qos
	sdk_qos.sdk_qos_enable_cb = b53125_qos_enable;




	//端口限速
	sdk_qos.sdk_qos_port_egress_rate_cb = b53125_egress_rate;
	sdk_qos.sdk_qos_port_ingress_rate_cb = b53125_ingress_rate;
	//CPU
	//sdk_qos.sdk_qos_cpu_rate_cb = b53125_qos_cpu_map_queue;
/*******************************************************************/
	//端口信任那类优先级
	sdk_qos.sdk_qos_trust_enable_cb = b53125_qos_trust_enable_type;//b53125_qos_diffserv

	//8021P优先级到内部优先级映射
	sdk_qos.sdk_qos_8021p_map_priority_cb = b53125_8021p_map_priority;
	//diffserv优先级到内部优先级映射
	sdk_qos.sdk_qos_diffserv_map_priority_cb = b53125_diffserv_map_priority;
	//内部优先级到queue队列的映射
	sdk_qos.sdk_qos_priority_map_queue_cb = b53125_tc_map_queue;
	//设置queue调度方式
	sdk_qos.sdk_qos_queue_scheduling_cb = b53125_queue_scheduling;
	//设置queue调度权限
	sdk_qos.sdk_qos_queue_weight_cb = b53125_queue_weight;

	sdk_qos.sdk_qos_map_default_cb = b53125_qos_map_default_cb;

	sdk_qos.sdk_qos_remarking_map_cb = b53125_priority_remarking;
	sdk_qos.sdk_qos_remarking_map_default_cb = b53125_priority_remarking_default;	
	
	return OK;
}
