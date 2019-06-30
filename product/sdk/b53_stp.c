/*
 * b53_stp.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */

#include <zebra.h>


#include "b53_mdio.h"
#include "b53_regs.h"
#include "b53_driver.h"


/****************************************************************************************/
int b53125_set_stp_state(struct b53125_device *dev, int port, u8 state)
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

	ret |= b53_read8(dev, B53_CTRL_PAGE, B53_PORT_CTRL(port), &reg);
	reg &= ~PORT_CTRL_STP_STATE_MASK;
	reg |= hw_state;
	ret |= b53_write8(dev, B53_CTRL_PAGE, B53_PORT_CTRL(port), reg);
	return ret;
}
/****************************************************************************************/
int b53125_mstp_enable(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u8 reg = 0;
	if(enable)
		reg |= B53_MSTP_EN;
	else
		reg &= ~B53_MSTP_EN;
	ret |= b53_write8(dev, B53_MSTP_PAGE, B53_MSTP_CTL, reg);
	return ret;
}
/****************************************************************************************/
int b53125_mstp_aging_time(struct b53125_device *dev, int aging)
{
	int ret = 0;
	u32 reg = 0;
	reg |= aging & B53_MSTP_AGE_MASK;
	ret |= b53_write32(dev, B53_MSTP_PAGE, B53_MSTP_AGE_CTL, reg);
	return ret;
}
/****************************************************************************************/
int b53125_mstp_state(struct b53125_device *dev, int id, int port, int state)
{
	int ret = 0;
	u32 reg = 0;
	ret |= b53_read32(dev, B53_MSTP_PAGE, B53_MSTP_TBL_CTL(id), &reg);
	reg &= ~B53_MSTP_TBL_PORT_MASK(port);
	reg |= B53_MSTP_TBL_PORT(port, state);
	ret |= b53_write32(dev, B53_MSTP_PAGE, B53_MSTP_TBL_CTL(id), reg);
	return ret;
}

int b53125_mstp_bypass(struct b53125_device *dev, int id, BOOL enable)
{
	int ret = 0;
	u32 reg = 0;
	ret |= b53_read16(dev, B53_MSTP_PAGE, B53_MSTP_BYPASS_CTL, &reg);
	if(enable)
		reg |= B53_MSTP_BYPASS_EN(id);
	else
		reg &= ~B53_MSTP_BYPASS_EN(id);
	ret |= b53_write16(dev, B53_MSTP_PAGE, B53_MSTP_BYPASS_CTL, reg);
	return ret;
}
