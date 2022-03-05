/*
 * b53_qos.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */



#include <zpl_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"


/*************************************************************************/
int b53125_qos_aggreation_mode(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_AGG_MODE;
	else
		port_ctrl &= 0x7f;//~B53_AGG_MODE;

	ret |= b53125_write8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
//设置使能基于端口的优先级(基于端口默认VLAN优先级)
int b53125_qos_base_port(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_PORT_QOS_EN;
	else
		port_ctrl &= ~B53_PORT_QOS_EN;

	ret |= b53125_write8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
//设置优先级映射规则
int b53125_qos_layer_sel(sdk_driver_t *dev, int sel)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, &port_ctrl);
	port_ctrl &= ~(3<<B53_QOS_LAYER_SEL_S);
	port_ctrl |= (sel<<B53_QOS_LAYER_SEL_S);
/*
	if(enable)
		port_ctrl |= (sel<<B53_QOS_LAYER_SEL_S);
	else
		port_ctrl &= ~(sel<<B53_QOS_LAYER_SEL_S);
*/

	ret |= b53125_write8(dev->sdk_device, B53_QOS_PAGE, B53_QOS_GLOBAL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
//禁止使能8021p优先级
int b53125_qos_8021p(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev->sdk_device, B53_QOS_PAGE, B53_802_1P_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= BIT(port);
	else
		port_ctrl &= ~BIT(port);

	ret |= b53125_write16(dev->sdk_device, B53_QOS_PAGE, B53_802_1P_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
//禁止使能差分服务优先级
int b53125_qos_diffserv(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev->sdk_device, B53_QOS_PAGE, B53_QOS_DIFFSERV_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= BIT(port);
	else
		port_ctrl &= ~BIT(port);

	ret |= b53125_write16(dev->sdk_device, B53_QOS_PAGE, B53_QOS_DIFFSERV_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
//设置端口到队列的映射（8021P优先级到COS优先级的映射）
int b53125_8021p_map_priority(sdk_driver_t *dev, zpl_phyport_t port, int p, int priority)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_QOS_PAGE, B53_PCP_TO_TC_PORT_CTL(port), &port_ctrl);
	port_ctrl &= ~(B53_TC_MASK<<B53_PCP_TO_TC(p));
	port_ctrl |= (priority<<B53_PCP_TO_TC(p));
	ret |= b53125_write32(dev->sdk_device, B53_QOS_PAGE, B53_PCP_TO_TC_PORT_CTL(port), port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
//设置差分优先级到队列的映射
int b53125_diffserv_map_priority(sdk_driver_t *dev, int diffserv, int priority)
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

	ret |= b53125_read48(dev->sdk_device, B53_QOS_PAGE, reg, &port_ctrl);
	port_ctrl &= ~(B53_TC_MASK<<B53_DIFFSERV_TO_TC(diffserv));
	port_ctrl |= (priority<<B53_DIFFSERV_TO_TC(diffserv));
	ret |= b53125_write48(dev->sdk_device, B53_QOS_PAGE, reg, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
//设置内部优先级到队列的映射
int b53125_tc_map_queue(sdk_driver_t *dev, int priority, int queue)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev->sdk_device, B53_QOS_PAGE, B53_TC_TO_QUEUE_CTL, &port_ctrl);
	port_ctrl &= ~(B53_TC_QUEUE_MASK<<B53_TC_TO_QUEUE(priority));
	port_ctrl |= (queue<<B53_TC_TO_QUEUE(priority));

	ret |= b53125_write16(dev->sdk_device, B53_QOS_PAGE, B53_TC_TO_QUEUE_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
//设置进入CPU报文到队列的映射
int b53125_qos_cpu_map_queue(sdk_driver_t *dev, int traffic, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_QOS_PAGE, B53_CPU_TO_QUEUE_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= (traffic);
	else
		port_ctrl &= ~(traffic);
	ret |= b53125_write32(dev->sdk_device, B53_QOS_PAGE, B53_CPU_TO_QUEUE_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
//设置queue调度方式
int b53125_queue_scheduling(sdk_driver_t *dev, int mode)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_QOS_PAGE, B53_TX_QUEUE_CTL, &port_ctrl);
	port_ctrl &= ~(B53_TC_QUEUE_MASK);
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
	ret |= b53125_write8(dev->sdk_device, B53_QOS_PAGE, B53_TX_QUEUE_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
//设置queue调度权限
int b53125_queue_weight(sdk_driver_t *dev, int queue, int weight)
{
	int ret = 0;
	u8 port_ctrl = 0;
	port_ctrl |= (weight);
	ret |= b53125_write8(dev->sdk_device, B53_QOS_PAGE, B53_TX_QUEUE_WEIGHT(queue), port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
int b53125_qos_class4_weight(sdk_driver_t *dev, zpl_bool strict, int weight)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev->sdk_device, B53_QOS_PAGE, B53_COS4_SERVICE_WEIGHT, &port_ctrl);
	port_ctrl &= ~(1<<8);
	if(weight >= 0)
		port_ctrl |= (weight);
	port_ctrl |= (strict<<8);
	ret |= b53125_write16(dev->sdk_device, B53_QOS_PAGE, B53_COS4_SERVICE_WEIGHT, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/* Traffic Remarking Register */
int b53125_qos_cfi_remarking(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= (B53_CFI_REMARK_EN(port));
	else
		port_ctrl &= ~B53_CFI_REMARK_EN(port);
	ret |= b53125_write32(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

int b53125_qos_pcp_remarking(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= (B53_PCP_REMARK_EN(port));
	else
		port_ctrl &= ~B53_PCP_REMARK_EN(port);
	ret |= b53125_write32(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

int b53125_priority_remarking(sdk_driver_t *dev, zpl_phyport_t port, zpl_uint32 tc, zpl_uint32 priority)
{
	int ret = 0;
	u64 port_ctrl = 0;
	ret |= b53125_read64(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TX_TO_PCP_CTL(port), &port_ctrl);

	port_ctrl &= ~(B53_TC_PCP_MASK<<B53_TC_PCP_PRI(tc));
	port_ctrl |= (priority<<B53_TC_PCP_PRI(tc));

	ret |= b53125_write64(dev->sdk_device, B53_TC_REMARK_PAGE, B53_TC_REMARK_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

int b53125_qos_init(void)
{
	//禁止使能qos
	sdk_qos.sdk_qos_enable_cb = b53125_qos_aggreation_mode;
	//sdk_qos.sdk_qos_ipg_cb) (void *, zpl_bool tx, zpl_bool);
	//端口优先级模式
	//sdk_qos.sdk_qos_base_cb) (void *, zpl_phyport_t, nsm_qos_trust_e);
	sdk_qos.sdk_qos_8021q_enable_cb = b53125_qos_8021p;
	sdk_qos.sdk_qos_diffserv_enable_cb = b53125_qos_diffserv;
#ifdef NSM_QOS_CLASS_PRIORITY
	//端口优先级到queue映射
	sdk_qos.sdk_qos_port_map_queue_cb = b53125_qos_port_map_queue;
#endif
	//差分服务到queue映射
	sdk_qos.sdk_qos_diffserv_map_queue_cb = NULL;

#ifdef NSM_QOS_CLASS_PRIORITY
	//队列到class的映射
	sdk_qos.sdk_qos_queue_map_class_cb = b53125_qos_queue_map_class;
	sdk_qos.sdk_qos_class_scheduling_cb = b53125_qos_class_scheduling;
	sdk_qos.sdk_qos_class_weight_cb = b53125_qos_class_weight;
#endif
	//风暴
	sdk_qos.sdk_qos_storm_rate_cb = b53125_strom_rate;

	//端口限速
	sdk_qos.sdk_qos_port_egress_rate_cb = b53125_egress_rate;
	sdk_qos.sdk_qos_port_ingress_rate_cb = b53125_ingress_rate;
	//CPU
	sdk_qos.sdk_qos_cpu_rate_cb = b53125_qos_cpu_map_queue;
	//remarking
	sdk_qos.sdk_qos_cfi_remarking_cb = NULL;
	sdk_qos.sdk_qos_pcp_remarking_cb = NULL;
	return OK;
}