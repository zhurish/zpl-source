/*
 * b53_qos.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */



#include <zplos_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_rate.h"
#include "b53_qos.h"

/*************************************************************************/
/*************************************************************************/
/* Broadcast Storm Suppression Register */
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
//禁止使能流量IPG
static int b53125_qos_ingress_ipg(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_INGRESS_RATE_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_IPG_XLENEN_EN;
	else
		port_ctrl &= ~B53_IPG_XLENEN_EN;	
	port_ctrl &= ~(B53_BUCK0_BRM_SEL_EN);
	port_ctrl &= ~(B53_BUCK1_BRM_SEL_EN);

	ret |= b53125_write32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_INGRESS_RATE_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
/*************************************************************************/
static int b53125_qos_buck_type(sdk_driver_t *dev, int id, zpl_uint32 type)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_INGRESS_RATE_CTL, &port_ctrl);
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
	ret |= b53125_write32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_INGRESS_RATE_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
/* 
* bucket : 0-7 4kB,8KB, 16KB,32KB,64KB,500KB,500KB,500KB
* cnt 0-0XFF
*/
static int b53125_qos_ingress_rate(sdk_driver_t *dev, zpl_phyport_t port, zpl_uint32 index, int bucket, int cnt)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), &port_ctrl);
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
	ret |= b53125_write32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
static int port_rate_tbl(int rate, int *value)//KB 1024KB=1MB
{
	//int minrate1 = 0x0fffffff, minrate2 = 0x0fffffff;
	//zpl_uint32 i = 0;
	//4kB,8KB, 16KB,32KB,64KB,500KB,500KB,500KB
	//4kb,8Kb, 16Kb,32Kb,64Kb,500Kb,500Kb,500Kb
	//int rate_tbl[] = { 4*8, 8*8, 16*8, 32*8, 64*8, 500*8, 500*8, 500*8};
	if(rate <= (1024*8*1.792))
	{
		*value = rate/(64*8);
		return 4;
	}
	else if(rate <= (1024*8*100))
	{
		*value = rate/(500*8*2);
		return 5;
	}
	else	
	{
		*value = rate/(500*8*8);
		return 6;
	}
}
/*************************************************************************/
static int cpu_rate_tbl(int rate)//pps
{
	int minrate1 = 0x0fffffff, minrate2 = 0x0fffffff;
	zpl_uint32 i = 0;
	int rate_tbl[] = { 384, 512, 639, 786, 1024, 1280, 1536, 1791, 2048, 2303, 2559, 2815,
		3328, 3840, 4352, 4863, 5376, 5887, 6400, 6911, 7936, 8960, 9984, 11008, 12030,
	    13054, 14076, 15105, 17146, 19201, 21240, 23299, 25354, 27382, 29446, 31486, 25561,
		39682, 42589, 56818, 71023, 85324, 99602, 113636, 127551, 142045, 312675, 284091,
		357143, 423929, 500000, 568182, 641026, 714286, 781250, 862069, 925069, 1000000,
		1086957, 1136364, 1190476, 1250000, 1315789, 1388889
	};
	for(i = 0; i < sizeof(rate_tbl)/sizeof(rate_tbl[0]); i++)//50=500000/57=1000000/61=1250000
	{
		if(rate <= rate_tbl[i])
		{
			minrate1 = abs(rate_tbl[i] - rate);
			if(i>=1)
				minrate2 = abs(rate_tbl[i-1] - rate);
			if(minrate1 > minrate2)
			{
				return (i-1);
			}
			return i;
		}			
	}
	return i;
}
/*************************************************************************/
//CPU接口限速
int b53125_qos_cpu_rate(sdk_driver_t *dev, int rate)
{
	int ret = 0;
	u8 port_ctrl = 0;
	port_ctrl = cpu_rate_tbl(rate) & B53_RATE_INDEX_MASK;
	ret |= b53125_write8(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_IMP_PORT_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
/*************************************************************************/
int b53125_rate_default(sdk_driver_t *dev)
{
	b53125_qos_ingress_ipg(dev, 1);
	b53125_qos_buck_type(dev, 0, BIT(0)|BIT(1));
	b53125_qos_buck_type(dev, 1, BIT(2)|BIT(3)|BIT(4)|BIT(5));//广播，多播，未知单播
/*
	b53125_qos_ingress_rate_mode(dev, 4,  1, 1);
	b53125_qos_ingress_rate_mode(dev, 4,  2, 1);
	b53125_qos_ingress_rate_mode(dev, 4,  3, 1);
	b53125_qos_ingress_rate_mode(dev, 4,  4, 1);
	b53125_qos_ingress_rate_mode(dev, 4,  5, 1);*/
	return OK;
}

static int b53125_broadcast_rate(sdk_driver_t *dev, zpl_phyport_t port, int pp, int cnt)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), &port_ctrl);
	if(cnt <= 0)
	{
		//port_ctrl &= ~(B53_STRM_SUPP_EN);
		//port_ctrl &= ~(B53_RSVMC_SUPP_EN);
		//port_ctrl &= ~(B53_MC_SUPP_EN);
		port_ctrl &= ~(B53_BC_SUPP_EN);
		//port_ctrl &= ~(B53_DLF_SUPP_EN);
		port_ctrl &= ~(B53_BUCKET1_EN);
	}
	else
	{
		port_ctrl |= (B53_STRM_SUPP_EN);
		//port_ctrl |= (B53_RSVMC_SUPP_EN);
		//port_ctrl |= (B53_MC_SUPP_EN);
		port_ctrl |= (B53_BC_SUPP_EN);
		//port_ctrl |= (B53_DLF_SUPP_EN);
		port_ctrl |= B53_BUCKET1_EN;
	}
	ret |= b53125_write32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), port_ctrl);
	ret |= b53125_qos_ingress_rate(dev,  port, 1, pp, cnt);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_multicast_rate(sdk_driver_t *dev, zpl_phyport_t port, int pp, int cnt)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), &port_ctrl);
	if(cnt <= 0)
	{
		port_ctrl &= ~(B53_STRM_SUPP_EN);
		port_ctrl &= ~(B53_RSVMC_SUPP_EN);
		port_ctrl &= ~(B53_MC_SUPP_EN);
		//port_ctrl &= ~(B53_BC_SUPP_EN);
		port_ctrl &= ~(B53_DLF_SUPP_EN);
		port_ctrl &= ~(B53_BUCKET1_EN);
	}
	else
	{
		port_ctrl |= (B53_STRM_SUPP_EN);
		port_ctrl |= (B53_RSVMC_SUPP_EN);
		port_ctrl |= (B53_MC_SUPP_EN);
		//port_ctrl |= (B53_BC_SUPP_EN);
		port_ctrl |= (B53_DLF_SUPP_EN);
		port_ctrl |= B53_BUCKET1_EN;
	}
	ret |= b53125_write32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), port_ctrl);
	ret |= b53125_qos_ingress_rate(dev,  port, 1, pp, cnt);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;	
}

int b53125_strom_rate(sdk_driver_t *dev, zpl_phyport_t port, zpl_uint32 mode, zpl_uint32 cnt, zpl_uint32 type)
{
	int value = 1;
	int bucket = port_rate_tbl(cnt, &value);
	if(mode == 1)
		return b53125_multicast_rate(dev,  port,  bucket, value);
	if(mode == 2)
		return b53125_broadcast_rate(dev,  port,  bucket, value);
	return 0;	
}

int b53125_ingress_rate(sdk_driver_t *dev, zpl_phyport_t port, int cnt)
{
	int ret = 0;
	u32 port_ctrl = 0;
	int value = 1;
	int bucket = port_rate_tbl(cnt, &value);
	ret |= b53125_qos_buck_type(dev, 0, BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(5));

	ret |= b53125_read32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), &port_ctrl);
	if(cnt <= 0)
	{
		port_ctrl &= ~(B53_BUCKET0_EN);
	}
	else
	{
		port_ctrl |= B53_BUCKET0_EN;
	}
	ret |= b53125_write32(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_PORT_RECEIVE_RATE_CTL(port), port_ctrl);
	ret |= b53125_qos_ingress_rate(dev,  port, 0, bucket, value);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;		
}
/*************************************************************************/
int b53125_egress_rate(sdk_driver_t *dev, zpl_phyport_t port, int rate)//64kB
{
	int ret = 0;
	u16 port_ctrl = 0;
	int value = 1;
	int bucket = port_rate_tbl(rate, &value);
	port_ctrl |= B53_ERC_EN;
	port_ctrl |= (bucket & B53_BKT_SZE_MASK)<<B53_BKT_SZE_S;
	port_ctrl |= (value & B53_RATE_CNT_MASK);
	ret |= b53125_write16(dev->sdk_device, B53_BROADCAST_STROM_PAGE, B53_PORT_EGRESS_RATE_CTL(port), port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}