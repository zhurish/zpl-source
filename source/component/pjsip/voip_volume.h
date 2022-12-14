/*
 * voip_volume.h
 *
 *  Created on: 2018年12月29日
 *      Author: DELL
 */

#ifndef __VOIP_VOLUME_H__
#define __VOIP_VOLUME_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VOIP_AMP_DEV_ENABLE
//#define _VOIP_VOLUME_DEBUG(fmt,...)		zlog_debug(MODULE_VOIP, fmt, ##__VA_ARGS__)
#define _VOIP_VOLUME_DEBUG(fmt,...)

#define VOIP_VOLUME_USE_SHELL

enum
{
	VOIP_OUT_VOLUME			= 80,
	VOIP_OUT_STEREO_VOLUME	= 0,
	VOIP_OUT_MONO_VOLUME	= 0,

	VOIP_IN_VOLUME			= 0,//80,
#ifdef ZPL_PJSIP_MODULE
	VOIP_IN_ADC_VOLUME		= 60,
#else
	VOIP_IN_ADC_VOLUME		= 0,
#endif
	VOIP_IN_MONO_VOLUME		= 0,
	VOIP_IN_BOOST_VOLUME	= 0,
	VOIP_IN_BOOST_GAIN_VOLUME	= 0,
};

typedef enum
{
	VOIP_VOLUME_ALL,
	VOIP_VOLUME_PLAYBACK,
	VOIP_VOLUME_CAPTURE,
}voip_volume_mode;

typedef struct voip_volume_s
{
	//Playback
	zpl_bool		power;
	zpl_uint8		out_volume;
	zpl_uint8		dac_volume;
	zpl_uint8		mono_volume;
	//Capture
	zpl_bool		record ;
	zpl_uint8		in_volume;
	zpl_uint8		adc_volume;
	zpl_uint8		in_mono_volume;
	zpl_uint8		in_boost;
	zpl_uint8		in_boost_gain;

	zpl_bool		isconfig;
	zpl_bool		p_isopen;
	zpl_bool		c_isopen;
} voip_volume_t;


extern int voip_volume_module_init();
extern int voip_volume_module_exit();

//extern int voip_volume_apply();

extern int voip_volume_open_api(voip_volume_mode mode);
extern int voip_volume_close_api(voip_volume_mode mode);
extern int voip_volume_control_api(zpl_bool enable);
#ifndef VOIP_AMP_DEV_ENABLE
/*
 * open power amplifier
 */
extern int voip_volume_power(zpl_bool enable);
extern int voip_volume_ispower(void);
#endif

/*
 * Playback
 */
extern int voip_playback_volume_out_set_api(zpl_uint8 value);
extern int voip_playback_volume_out_get_api(zpl_uint8 *value);
extern int voip_playback_volume_dac_set_api(zpl_uint8 value);
extern int voip_playback_volume_dac_get_api(zpl_uint8 *value);
extern int voip_playback_volume_mono_set_api(zpl_uint8 value);
extern int voip_playback_volume_mono_get_api(zpl_uint8 *value);

extern int voip_playback_open_api(zpl_bool enable);
/*
 * Capture
 */
extern int voip_capture_volume_in_set_api(zpl_uint8 value);
extern int voip_capture_volume_in_get_api(zpl_uint8 *value);
extern int voip_capture_volume_adc_set_api(zpl_uint8 value);
extern int voip_capture_volume_adc_get_api(zpl_uint8 *value);
extern int voip_capture_volume_mono_set_api(zpl_uint8 value);
extern int voip_capture_volume_mono_get_api(zpl_uint8 *value);

extern int voip_volume_boost_set_api(zpl_uint8 value);
extern int voip_volume_boost_get_api(zpl_uint8 *value);
extern int voip_volume_boost_gain_set_api(zpl_uint8 value);
extern int voip_volume_boost_gain_get_api(zpl_uint8 *value);

extern int voip_capture_open_api(zpl_bool enable);


extern int voip_volume_show_config(struct vty *vty, int detail);
extern int voip_volume_write_config(struct vty *vty);

#ifdef __cplusplus
}
#endif


#endif /* __VOIP_VOLUME_H__ */
