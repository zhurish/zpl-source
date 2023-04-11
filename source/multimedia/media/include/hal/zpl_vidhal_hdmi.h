/*
 * zpl_vidhal_hdmi.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_HDMI_H__
#define __ZPL_VIDHAL_HDMI_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef ZPL_HISIMPP_MODULE

int zpl_vidhal_hdmi_getwh(VO_INTF_SYNC_E enIntfSync, zpl_uint32* pu32W, zpl_uint32* pu32H, zpl_uint32* pu32Frm);
int zpl_vidhal_hdmi_dev_start(zpl_int32 VoDev, void* pstPubAttr);
int zpl_vidhal_hdmi_dev_stop(zpl_int32 VoDev);
int zpl_vidhal_hdmi_layer_start(zpl_int32 VoLayer, const void* pstLayerAttr);
int zpl_vidhal_hdmi_layer_stop(zpl_int32 VoLayer);
int zpl_vidhal_hdmi_channel_start(zpl_int32 VoLayer, ZPL_HDMI_VO_MODE_E enMode);
int zpl_vidhal_hdmi_channel_stop(zpl_int32 VoLayer, ZPL_HDMI_VO_MODE_E enMode);
int zpl_vidhal_hdmi_HdmiConvertSync(VO_INTF_SYNC_E enIntfSync,
    HI_HDMI_VIDEO_FMT_E *penVideoFmt);
int zpl_vidhal_hdmi_HdmiStart(VO_INTF_SYNC_E enIntfSync);
int zpl_vidhal_hdmi_HdmiStartByDyRg(VO_INTF_SYNC_E enIntfSync, DYNAMIC_RANGE_E enDyRg);
int zpl_vidhal_hdmi_HdmiStop(HI_VOID);

#endif


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_HDMI_H__ */
