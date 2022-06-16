/*
 * b53_stp.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */


#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_stp.h"
#include "b53_port.h"
/****************************************************************************************/
static int b53125_set_stp_state(sdk_driver_t *dev, zpl_index_t index, zpl_phyport_t port, zpl_uint32 state)
{
	int ret = 0;
	u8 hw_state;
	u8 reg;

	switch (state) {
	case BR_STATE_DISABLED:
		hw_state = PORT_CTRL_DIS_STATE;
		break;
	case BR_STATE_LISTENING:
		hw_state = PORT_CTRL_LISTEN_STATE;
		break;
	case BR_STATE_LEARNING:
		hw_state = PORT_CTRL_LEARN_STATE;
		break;
	case BR_STATE_FORWARDING:
		hw_state = PORT_CTRL_FWD_STATE;
		break;
	case BR_STATE_BLOCKING:
		hw_state = PORT_CTRL_BLOCK_STATE;
		break;
	default:
		//dev_err(ds->dev, "invalid STP state: %d\n", state);
		return ERROR;
	}

	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_CTRL(port), &reg);
	reg &= ~PORT_CTRL_STP_STATE_MASK;
	reg |= hw_state;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_CTRL(port), reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int b53125_set_mstp_state(sdk_driver_t *dev, zpl_index_t index, zpl_phyport_t port, zpl_uint32 state)
{
	int ret = 0;
	u8 hw_state = 0;
	u8 reg;

	switch (state) {
	case STP_DISABLE:
		hw_state = 1;
		break;
	case STP_LISTENING:
		hw_state = 3;
		break;
	case STP_LEARNING:
		hw_state = 4;
		break;
	case STP_FORWARDING:
		hw_state = 5;
		break;
	case STP_BLOCKING:
		hw_state = 2;
		break;
	default:
		//dev_err(ds->dev, "invalid STP state: %d\n", state);
		return ERROR;
	}

	ret |= b53125_read8(dev->sdk_device, B53_MSTP_PAGE, B53_MSTP_TBL_CTL(index), &reg);
	reg &= ~B53_MSTP_TBL_PORT_MASK(port);
	reg |= B53_MSTP_TBL_PORT(port, hw_state);
	ret |= b53125_write8(dev->sdk_device, B53_MSTP_PAGE, B53_MSTP_TBL_CTL(index), reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/****************************************************************************************/
static int b53125_mstp_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	if(enable)
		reg |= B53_MSTP_EN;
	else
		reg &= ~B53_MSTP_EN;
	ret |= b53125_write8(dev->sdk_device, B53_MSTP_PAGE, B53_MSTP_CTL, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/****************************************************************************************/
static int b53125_mstp_aging_time(sdk_driver_t *dev, zpl_index_t aging)
{
	int ret = 0;
	u32 reg = 0;
	reg |= aging & B53_MSTP_AGE_MASK;
	ret |= b53125_write32(dev->sdk_device, B53_MSTP_PAGE, B53_MSTP_AGE_CTL, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
#if 0
static int b53125_mstp_bypass(sdk_driver_t *dev, zpl_bool enable, int id)
{
	int ret = 0;
	u32 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MSTP_PAGE, B53_MSTP_BYPASS_CTL, &reg);
	if(enable)
		reg |= B53_MSTP_BYPASS_EN(id);
	else
		reg &= ~B53_MSTP_BYPASS_EN(id);
	ret |= b53125_write16(dev->sdk_device, B53_MSTP_PAGE, B53_MSTP_BYPASS_CTL, reg);
	return ret;
}
#endif

static int sdk_mstp_create (sdk_driver_t *dev,  zpl_index_t id)
{
	return OK;
}
int b53125_mstp_init(sdk_driver_t *dev)
{

	sdk_mstp.sdk_stp_state_cb = b53125_set_stp_state;

	sdk_mstp.sdk_mstp_enable_cb = b53125_mstp_enable;
	sdk_mstp.sdk_mstp_aging_cb = b53125_mstp_aging_time;
	sdk_mstp.sdk_mstp_state = b53125_set_mstp_state;
	sdk_mstp.sdk_mstp_create = sdk_mstp_create;

	return OK;
}

