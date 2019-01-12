/*
 * voip_volume.h
 *
 *  Created on: 2018年12月29日
 *      Author: DELL
 */

#ifndef __VOIP_VOLUME_H__
#define __VOIP_VOLUME_H__



#define VOIP_VOLUME_USE_SHELL

enum
{
	VOIP_OUT_VOLUME			= 0,
	VOIP_OUT_STEREO_VOLUME	= 60,
	VOIP_OUT_MONO_VOLUME	= 0,

	VOIP_IN_VOLUME			= 0,
	VOIP_IN_ADC_VOLUME		= 60,
	VOIP_IN_MONO_VOLUME		= 0,
	VOIP_IN_BOOST_VOLUME	= 0,
	VOIP_IN_BOOST_GAIN_VOLUME	= 0,
};

typedef struct voip_volume_s
{
	//Playback
	BOOL		power;
	u_int8		out_volume;
	u_int8		dac_volume;
	u_int8		mono_volume;
	//Capture
	BOOL		record ;
	u_int8		in_volume;
	u_int8		adc_volume;
	u_int8		in_mono_volume;
	u_int8		in_boost;
	u_int8		in_boost_gain;

	BOOL		isconfig;
} voip_volume_t;


extern int voip_volume_module_init();
extern int voip_volume_module_exit();

extern int voip_volume_apply();

/*
 * open power amplifier
 */
extern int voip_volume_power(BOOL enable);
extern int voip_volume_ispower(void);

/*
 * Playback
 */
extern int voip_playback_volume_out_set_api(u_int8 value);
extern int voip_playback_volume_out_get_api(u_int8 *value);
extern int voip_playback_volume_dac_set_api(u_int8 value);
extern int voip_playback_volume_dac_get_api(u_int8 *value);
extern int voip_playback_volume_mono_set_api(u_int8 value);
extern int voip_playback_volume_mono_get_api(u_int8 *value);

extern int voip_playback_open_api(BOOL enable);
/*
 * Capture
 */
extern int voip_capture_volume_in_set_api(u_int8 value);
extern int voip_capture_volume_in_get_api(u_int8 *value);
extern int voip_capture_volume_adc_set_api(u_int8 value);
extern int voip_capture_volume_adc_get_api(u_int8 *value);
extern int voip_capture_volume_mono_set_api(u_int8 value);
extern int voip_capture_volume_mono_get_api(u_int8 *value);

extern int voip_volume_boost_set_api(u_int8 value);
extern int voip_volume_boost_get_api(u_int8 *value);
extern int voip_volume_boost_gain_set_api(u_int8 value);
extern int voip_volume_boost_gain_get_api(u_int8 *value);

extern int voip_capture_open_api(BOOL enable);


extern int voip_volume_show_config(struct vty *vty, int detail);
extern int voip_volume_write_config(struct vty *vty);


#endif /* __VOIP_VOLUME_H__ */
