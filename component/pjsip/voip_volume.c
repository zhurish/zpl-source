/*
 * voip_volume.c
 *
 *  Created on: 2018年12月29日
 *      Author: DELL
 */

#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "vty.h"


#include "voip_volume.h"


#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
//#include <asm/page.h>


static voip_volume_t *voip_volume = NULL;


#ifdef VOIP_VOLUME_USE_SHELL
#define VOIP_NOHUP 	" >/dev/null 2>&1"

/*
 * playback
 */
static int voip_volume_playback_init()
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
	super_system("amixer cset numid=92 1" VOIP_NOHUP);		//Stereo DAC MIXR DAC L1 Switch
	super_system("amixer cset numid=97 1" VOIP_NOHUP);		//Stereo DAC MIXL DAC L1 Switch
	super_system("amixer cset numid=117 1" VOIP_NOHUP);		//OUT MIXR DAC R1 Switch
	super_system("amixer cset numid=113 1" VOIP_NOHUP);		//OUT MIXL DAC L1 Switch

	super_system("amixer cset numid=126 1" VOIP_NOHUP);		//LOUT MIX OUTMIX L Switch
	super_system("amixer cset numid=127 1" VOIP_NOHUP);		//LOUT MIX OUTMIX R Switch
	super_system("amixer cset numid=85 1" VOIP_NOHUP);		//DAC1 MIXL DAC1 Switch
	super_system("amixer cset numid=87 1" VOIP_NOHUP);		//DAC1 MIXR DAC1 Switch
	super_system("amixer cset numid=7 1 1" VOIP_NOHUP);		//OUT Playback Switch
#else
	super_system("amixer cset numid=82 1" VOIP_NOHUP);		//Stereo DAC MIXR DAC L1 Switch
	super_system("amixer cset numid=83 1" VOIP_NOHUP);		//Stereo DAC MIXL DAC L1 Switch
	super_system("amixer cset numid=63 1" VOIP_NOHUP);		//OUT MIXR DAC R1 Switch
	super_system("amixer cset numid=67 1" VOIP_NOHUP);		//OUT MIXL DAC L1 Switch

	super_system("amixer cset numid=52 1" VOIP_NOHUP);		//LOUT MIX OUTMIX L Switch
	super_system("amixer cset numid=53 1" VOIP_NOHUP);		//LOUT MIX OUTMIX R Switch
	super_system("amixer cset numid=93 1" VOIP_NOHUP);		//DAC1 MIXL DAC1 Switch
	super_system("amixer cset numid=91 1" VOIP_NOHUP);		//DAC1 MIXR DAC1 Switch

	super_system("amixer cset numid=7 1 1" VOIP_NOHUP);	//OUT Playback Switch
#endif
	return OK;
}

static int voip_volume_playback_exit()
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
	super_system("amixer cset numid=92 0" VOIP_NOHUP);		//Stereo DAC MIXR DAC L1 Switch
	super_system("amixer cset numid=97 0" VOIP_NOHUP);		//Stereo DAC MIXL DAC L1 Switch
	super_system("amixer cset numid=117 0" VOIP_NOHUP);		//OUT MIXR DAC R1 Switch
	super_system("amixer cset numid=113 0" VOIP_NOHUP);		//OUT MIXL DAC L1 Switch

	super_system("amixer cset numid=126 0" VOIP_NOHUP);		//LOUT MIX OUTMIX L Switch
	super_system("amixer cset numid=127 0" VOIP_NOHUP);		//LOUT MIX OUTMIX R Switch
	super_system("amixer cset numid=85 0" VOIP_NOHUP);		//DAC1 MIXL DAC1 Switch
	super_system("amixer cset numid=87 0" VOIP_NOHUP);		//DAC1 MIXR DAC1 Switch
	super_system("amixer cset numid=7 0 0" VOIP_NOHUP);		//OUT Playback Switch
#else
	super_system("amixer cset numid=82 0" VOIP_NOHUP);		//Stereo DAC MIXR DAC L1 Switch
	super_system("amixer cset numid=83 0" VOIP_NOHUP);		//Stereo DAC MIXL DAC L1 Switch
	super_system("amixer cset numid=63 0" VOIP_NOHUP);		//OUT MIXR DAC R1 Switch
	super_system("amixer cset numid=67 0" VOIP_NOHUP);		//OUT MIXL DAC L1 Switch

	super_system("amixer cset numid=52 0" VOIP_NOHUP);		//LOUT MIX OUTMIX L Switch
	super_system("amixer cset numid=53 0" VOIP_NOHUP);		//LOUT MIX OUTMIX R Switch
	super_system("amixer cset numid=93 0" VOIP_NOHUP);		//DAC1 MIXL DAC1 Switch
	super_system("amixer cset numid=91 0" VOIP_NOHUP);		//DAC1 MIXR DAC1 Switch

	super_system("amixer cset numid=7 0 0" VOIP_NOHUP);	//OUT Playback Switch
#endif
	return OK;
}


static int voip_volume_playback_volume_init(int out, int dac, int mono)
{
	char cmd[128];
	float v = 0.0;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
	memset(cmd, 0, sizeof(cmd));

	if(out >= 0)
	{
		v = (float)((float)out)/100 * 39;
		_VOIP_VOLUME_DEBUG("=======%s:out=%.2f", __func__,v);
		//v=30.0;
		sprintf(cmd, "amixer cset numid=8 %d %d" VOIP_NOHUP, (int)v, (int)v);//OUT Playback Volume
		super_system(cmd);
	}
	if(dac >= 0)
	{
		v = (float)((float)dac)/100 * 175;
		_VOIP_VOLUME_DEBUG("=======%s:dac=%.2f", __func__,v);
		//v=150.0;
		sprintf(cmd, "amixer cset numid=10 %d %d" VOIP_NOHUP, (int)v, (int)v);//DAC1 Playback Volume
		super_system(cmd);
	}
	if(mono >= 0)
	{
		v = (float)((float)mono)/100 * 175;
		_VOIP_VOLUME_DEBUG("=======%s:mono=%.2f", __func__,v);
		//v=150.0;
		sprintf(cmd, "amixer cset numid=11 %d %d" VOIP_NOHUP, (int)v, (int)v);//Mono DAC Playback Volume
		super_system(cmd);
	}
#else
	memset(cmd, 0, sizeof(cmd));

	if(out >= 0)
	{
		v = (float)((float)out)/100 * 39;
		_VOIP_VOLUME_DEBUG("=======%s:out=%.2f", __func__,v);
		//v=30.0;
		sprintf(cmd, "amixer cset numid=8 %d %d" VOIP_NOHUP, (int)v, (int)v);//OUT Playback Volume
		super_system(cmd);
	}
	if(dac >= 0)
	{
		v = (float)((float)dac)/100 * 175;
		_VOIP_VOLUME_DEBUG("=======%s:dac=%.2f", __func__,v);
		//v=150.0;
		sprintf(cmd, "amixer cset numid=10 %d %d" VOIP_NOHUP, (int)v, (int)v);//DAC1 Playback Volume
		super_system(cmd);
	}
	if(mono >= 0)
	{
		v = (float)((float)mono)/100 * 175;
		_VOIP_VOLUME_DEBUG("=======%s:mono=%.2f", __func__,v);
		//v=150.0;
		sprintf(cmd, "amixer cset numid=11 %d %d" VOIP_NOHUP, (int)v, (int)v);//Mono DAC Playback Volume
		super_system(cmd);
	}
#endif
	//super_system("amixer cset numid=8 1 1");	//OUT Playback Volume 0-39
	//super_system("amixer cset numid=10 175 175");	//DAC1 Playback Volume 0-175
	//super_system("amixer cset numid=11 175 175");	//Mono DAC Playback Volume 0-175
	return OK;
}

static int voip_volume_dsp_init()
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
	super_system("amixer cset numid=132 1" VOIP_NOHUP);//IF1 ADC1 IN2 Mux
	super_system("amixer cset numid=3 2" VOIP_NOHUP);//DSP Function Switch
	super_system("amixer cset numid=76 0" VOIP_NOHUP);//DSP UL Mux
#else
	//if(access("/app/etc/dsp", F_OK) == 0)
	{
		super_system("amixer cset numid=45 1" VOIP_NOHUP);//IF1 ADC1 IN2 Mux
		super_system("amixer cset numid=3 2" VOIP_NOHUP);//DSP Function Switch
		super_system("amixer cset numid=101 0" VOIP_NOHUP);//DSP UL Mux
		//super_system("amixer cset numid=100 0");
	}
#endif
	return OK;
}

static int voip_volume_dsp_exit()
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
	super_system("amixer cset numid=132 0" VOIP_NOHUP);//IF1 ADC1 IN2 Mux
	super_system("amixer cset numid=3 0" VOIP_NOHUP);//DSP Function Switch
#else
	//if(access("/app/etc/dsp", F_OK) == 0)
	{
		super_system("amixer cset numid=45 0" VOIP_NOHUP);
		super_system("amixer cset numid=3 0" VOIP_NOHUP);
	}
#endif
	return OK;
}

/*
 * Capture
 */
static int voip_volume_capture_init()
{
/*
	super_system("amixer cset numid=45 1");		//IF1 ADC1 IN2 Mux
	super_system("amixer cset numid=3 2");		//AES
	super_system("amixer cset numid=101 0");		//DSP UL Mux
	super_system("amixer cset numid=100 0");		//DSP DL Mux
*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
	super_system("amixer cset numid=64 1" VOIP_NOHUP);		//Sto1 ADC MIXL ADC1 Switch
	super_system("amixer cset numid=66 1" VOIP_NOHUP);		//Sto1 ADC MIXR ADC1 Switch
	super_system("amixer cset numid=49 1" VOIP_NOHUP);		//RECMIXR BST2 Switch
	super_system("amixer cset numid=46 1" VOIP_NOHUP);		//RECMIXL BST2 Switch

	super_system("amixer cset numid=17 1 1" VOIP_NOHUP);	//ADC Capture Switch
	super_system("amixer cset numid=18 75 75" VOIP_NOHUP);	//ADC Capture Volume
	super_system("amixer cset numid=20 18 18" VOIP_NOHUP);	//TxDP Capture Volume
	super_system("amixer cset numid=19 30 30" VOIP_NOHUP);	//Mono ADC Capture Volume
	super_system("amixer cset numid=16 18 18" VOIP_NOHUP);	//IN Capture Volume
#else
	super_system("amixer cset numid=112 1" VOIP_NOHUP);		//Sto1 ADC MIXL ADC1 Switch
	super_system("amixer cset numid=110 1" VOIP_NOHUP);		//Sto1 ADC MIXR ADC1 Switch
	super_system("amixer cset numid=128 1" VOIP_NOHUP);		//RECMIXR BST2 Switch
	super_system("amixer cset numid=131 1" VOIP_NOHUP);		//RECMIXL BST2 Switch

	super_system("amixer cset numid=17 1 1" VOIP_NOHUP);	//ADC Capture Switch
	super_system("amixer cset numid=18 100 100" VOIP_NOHUP);	//ADC Capture Volume
#endif
	return OK;
}

static int voip_volume_capture_exit()
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
	super_system("amixer cset numid=64 0" VOIP_NOHUP);		//Sto1 ADC MIXL ADC1 Switch
	super_system("amixer cset numid=66 0" VOIP_NOHUP);		//Sto1 ADC MIXR ADC1 Switch
	super_system("amixer cset numid=49 0" VOIP_NOHUP);		//RECMIXR BST2 Switch
	super_system("amixer cset numid=46 0" VOIP_NOHUP);		//RECMIXL BST2 Switch

	super_system("amixer cset numid=17 1 1" VOIP_NOHUP);	//ADC Capture Switch
#else
	super_system("amixer cset numid=112 0" VOIP_NOHUP);		//Sto1 ADC MIXL ADC1 Switch
	super_system("amixer cset numid=110 0" VOIP_NOHUP);		//Sto1 ADC MIXR ADC1 Switch
	super_system("amixer cset numid=128 0" VOIP_NOHUP);		//RECMIXR BST2 Switch
	super_system("amixer cset numid=131 0" VOIP_NOHUP);		//RECMIXL BST2 Switch

	super_system("amixer cset numid=17 0 0" VOIP_NOHUP);	//ADC Capture Switch
#endif
	return OK;
}

static int voip_volume_capture_boost_init(int value)
{
	if(value)
	{
		char cmd[128];
		memset(cmd, 0, sizeof(cmd));
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
		sprintf(cmd, "amixer cset numid=14 %d" VOIP_NOHUP, value);
		super_system(cmd);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "amixer cset numid=15 %d" VOIP_NOHUP, value);
		super_system(cmd);
#else
		sprintf(cmd, "amixer cset numid=14 %d" VOIP_NOHUP, value);
		super_system(cmd);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "amixer cset numid=15 %d" VOIP_NOHUP, value);
		super_system(cmd);
#endif
	}
	//super_system("amixer cset numid=14 7");		//IN1 Boost Volume 0-8
	//super_system("amixer cset numid=15 7");		//IN2 Boost Volume 0-8
	return OK;
}

static int voip_volume_capture_volume_init(int inv, int adc, int mono)
{
	char cmd[128];
	float v = 0.0;
	memset(cmd, 0, sizeof(cmd));
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
	if(inv >= 0)
	{
		v = (float)((float)inv)/100 * 31;
		_VOIP_VOLUME_DEBUG("=======%s:inv=%.2f", __func__,v);
		//v=30.0;
		sprintf(cmd, "amixer cset numid=16 %d %d" VOIP_NOHUP, (int)v, (int)v); //IN Capture Volume
		super_system(cmd);
	}
	if(adc >= 0)
	{
		v = (float)((float)adc)/100 * 127;
		_VOIP_VOLUME_DEBUG("=======%s:adc=%.2f", __func__,v);
		//v=120.0;
		sprintf(cmd, "amixer cset numid=18 %d %d" VOIP_NOHUP, (int)v, (int)v); //ADC Capture Volume
		super_system(cmd);
	}
	if(mono >= 0)
	{
		v = (float)((float)mono)/100 * 127;
		_VOIP_VOLUME_DEBUG("=======%s:mono=%.2f", __func__,v);
		//v=120.0;
		sprintf(cmd, "amixer cset numid=19 %d %d" VOIP_NOHUP, (int)v, (int)v); //Mono ADC Capture Volume
		super_system(cmd);
	}
#else
	if(inv >= 0)
	{
		v = (float)((float)inv)/100 * 31;
		_VOIP_VOLUME_DEBUG("=======%s:inv=%.2f", __func__,v);
		//v=30.0;
		sprintf(cmd, "amixer cset numid=16 %d %d" VOIP_NOHUP, (int)v, (int)v);
		super_system(cmd);
	}
	if(adc >= 0)
	{
		v = (float)((float)adc)/100 * 127;
		_VOIP_VOLUME_DEBUG("=======%s:adc=%.2f", __func__,v);
		//v=120.0;
		sprintf(cmd, "amixer cset numid=18 %d %d" VOIP_NOHUP, (int)v, (int)v);
		super_system(cmd);
	}
	if(mono >= 0)
	{
		v = (float)((float)mono)/100 * 127;
		_VOIP_VOLUME_DEBUG("=======%s:mono=%.2f", __func__,v);
		//v=120.0;
		sprintf(cmd, "amixer cset numid=19 %d %d" VOIP_NOHUP, (int)v, (int)v);
		super_system(cmd);
	}
#endif
	//super_system("amixer cset numid=16 30");			//IN Capture Volume 0-31
	//super_system("amixer cset numid=18 175 175");		//ADC Capture Volume 0-127
	//super_system("amixer cset numid=19 175 175");		//Mono ADC Capture Volume 0-127
	return OK;
}

static int voip_volume_capture_boost_gain_init(int value)
{
	if(value)
	{
		char cmd[128];
		memset(cmd, 0, sizeof(cmd));
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
		sprintf(cmd, "amixer cset numid=21 %d" VOIP_NOHUP, value);//STO1 ADC Boost Gain Volume
		super_system(cmd);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "amixer cset numid=22 %d" VOIP_NOHUP, value);//STO2 ADC Boost Gain Volume
		super_system(cmd);
#else
		sprintf(cmd, "amixer cset numid=21 %d" VOIP_NOHUP, value);
		super_system(cmd);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "amixer cset numid=22 %d" VOIP_NOHUP, value);
		super_system(cmd);
#endif
	}
	//super_system("amixer cset numid=21 3");		//STO1 ADC Boost Gain Volume 0-3
	//super_system("amixer cset numid=22 3");		//STO2 ADC Boost Gain Volume 0-3
	return OK;
}

#endif



int voip_volume_module_init()
{
	//zassert(voip_app != NULL);
	voip_volume = XMALLOC(MTYPE_VOIP_VOLUME, sizeof(voip_volume_t));
	zassert(voip_volume != NULL);
	memset(voip_volume, 0, sizeof(voip_volume_t));
	voip_volume->power		= FALSE;
	voip_volume->out_volume	= VOIP_OUT_VOLUME;
	voip_volume->dac_volume  = VOIP_OUT_STEREO_VOLUME;
	voip_volume->mono_volume = VOIP_OUT_MONO_VOLUME;
	//Capture
	voip_volume->record      = FALSE;
	voip_volume->in_volume	= VOIP_IN_VOLUME;
	voip_volume->adc_volume  = VOIP_IN_ADC_VOLUME;
	voip_volume->in_mono_volume = VOIP_IN_MONO_VOLUME;
	voip_volume->in_boost	= VOIP_IN_BOOST_VOLUME;
	voip_volume->in_boost_gain = VOIP_IN_BOOST_GAIN_VOLUME;

	//voip_capture_volume_adc_set_api(voip_volume->adc_volume);
#ifdef VOIP_VOLUME_USE_SHELL
	voip_volume_capture_volume_init(voip_volume->out_volume, voip_volume->dac_volume, voip_volume->mono_volume);
	voip_volume_capture_volume_init(voip_volume->in_volume, voip_volume->adc_volume, voip_volume->in_mono_volume);
#endif
	return OK;
}

int voip_volume_module_exit()
{
	zassert(voip_volume != NULL);
	memset(voip_volume, 0, sizeof(voip_volume_t));
	XFREE(MTYPE_VOIP_VOLUME,voip_volume);
	voip_volume = NULL;
	return OK;
}


int voip_playback_volume_out_set_api(u_int8 value)
{
	zassert(voip_volume != NULL);
#ifdef VOIP_VOLUME_USE_SHELL
	voip_volume_playback_volume_init(value ? value:VOIP_OUT_VOLUME, -1, -1);
#endif
	voip_volume->out_volume = value ? value:VOIP_OUT_VOLUME;
	return OK;
}

int voip_playback_volume_out_get_api(u_int8 *value)
{
	zassert(voip_volume != NULL);
	if(value)
		*value = voip_volume->out_volume;
	return OK;
}

int voip_playback_volume_dac_set_api(u_int8 value)
{
	zassert(voip_volume != NULL);
#ifdef VOIP_VOLUME_USE_SHELL
	voip_volume_playback_volume_init(-1, value ? value:VOIP_OUT_STEREO_VOLUME, -1);
#endif
	voip_volume->dac_volume = value ? value:VOIP_OUT_STEREO_VOLUME;
	return OK;
}

int voip_playback_volume_dac_get_api(u_int8 *value)
{
	zassert(voip_volume != NULL);
	if(value)
		*value = voip_volume->dac_volume;
	return OK;
}

int voip_playback_volume_mono_set_api(u_int8 value)
{
	zassert(voip_volume != NULL);
#ifdef VOIP_VOLUME_USE_SHELL
	voip_volume_playback_volume_init(-1, -1, value ? value:VOIP_OUT_MONO_VOLUME);
#endif
	voip_volume->mono_volume = value ? value:VOIP_OUT_MONO_VOLUME;
	return OK;
}

int voip_playback_volume_mono_get_api(u_int8 *value)
{
	zassert(voip_volume != NULL);
	if(value)
		*value = voip_volume->mono_volume;
	return OK;
}

int voip_playback_open_api(BOOL enable)
{
	zassert(voip_volume != NULL);
	if(enable)
	{
		voip_volume_playback_init();
		voip_playback_volume_out_set_api(voip_volume->out_volume);
/*		voip_volume->isconfig = FALSE;
		voip_volume_apply();*/
		//voip_volume_playback_volume_init(voip_volume->out_volume,
		//		voip_volume->dac_volume, voip_volume->mono_volume);
		//voip_volume->isopen = TRUE;
	}
	else
	{
		voip_volume_playback_exit();
	}
	//voip_volume_power(enable);
	return OK;
}
/*
 * Capture
 */
int voip_capture_volume_in_set_api(u_int8 value)
{
	zassert(voip_volume != NULL);
#ifdef VOIP_VOLUME_USE_SHELL
	voip_volume_capture_volume_init(value ? value:VOIP_IN_VOLUME, -1, -1);
#endif
	voip_volume->in_volume = value ? value:VOIP_IN_VOLUME;
	return OK;
}

int voip_capture_volume_in_get_api(u_int8 *value)
{
	zassert(voip_volume != NULL);
	if(value)
		*value = voip_volume->in_volume;
	return OK;
}

int voip_capture_volume_adc_set_api(u_int8 value)
{
	zassert(voip_volume != NULL);
#ifdef VOIP_VOLUME_USE_SHELL
	voip_volume_capture_volume_init(-1, value ? value:VOIP_IN_ADC_VOLUME, -1);
#endif
	voip_volume->adc_volume = value ? value:VOIP_IN_ADC_VOLUME;
	return OK;
}

int voip_capture_volume_adc_get_api(u_int8 *value)
{
	zassert(voip_volume != NULL);
	if(value)
		*value = voip_volume->adc_volume;
	return OK;
}

int voip_capture_volume_mono_set_api(u_int8 value)
{
	zassert(voip_volume != NULL);
#ifdef VOIP_VOLUME_USE_SHELL
	voip_volume_capture_volume_init(-1, -1, value ? value:VOIP_IN_MONO_VOLUME);
#endif
	voip_volume->in_mono_volume = value ? value:VOIP_IN_MONO_VOLUME;
	return OK;
}

int voip_capture_volume_mono_get_api(u_int8 *value)
{
	zassert(voip_volume != NULL);
	if(value)
		*value = voip_volume->in_mono_volume;
	return OK;
}



int voip_volume_boost_set_api(u_int8 value)
{
	zassert(voip_volume != NULL);
#ifdef VOIP_VOLUME_USE_SHELL
	voip_volume_capture_boost_init(value ? value:VOIP_IN_BOOST_VOLUME);
#endif
	//if(input)
	{
		voip_volume->in_boost = value ? value:VOIP_IN_BOOST_VOLUME;
	}
	return OK;
}

int voip_volume_boost_get_api(u_int8 *value)
{
	zassert(voip_volume != NULL);
	//if(input)
	{
		if(value)
			*value = voip_volume->in_boost;
	}
	return OK;
}

int voip_volume_boost_gain_set_api(u_int8 value)
{
	zassert(voip_volume != NULL);
#ifdef VOIP_VOLUME_USE_SHELL
	voip_volume_capture_boost_gain_init(value ? value:VOIP_IN_BOOST_GAIN_VOLUME);
#endif
	//if(input)
	{
		voip_volume->in_boost_gain = value ? value:VOIP_IN_BOOST_GAIN_VOLUME;
	}
	return OK;
}

int voip_volume_boost_gain_get_api(u_int8 *value)
{
	zassert(voip_volume != NULL);
	//if(input)
	{
		if(value)
			*value = voip_volume->in_boost_gain;
	}
	return OK;
}

int voip_capture_open_api(BOOL enable)
{
	if(enable)
	{
		voip_volume_capture_init();
		voip_capture_volume_adc_set_api(voip_volume->adc_volume);
/*		voip_volume->isconfig = FALSE;
		voip_volume_apply();*/
		//voip_volume_capture_volume_init(voip_volume->in_volume,
		//		voip_volume->adc_volume, voip_volume->in_mono_volume);
		//voip_volume_capture_boost_init(voip_volume->in_boost);
		//voip_volume_capture_boost_gain_init(voip_volume->in_boost_gain);
	}
	else
	{
		voip_volume_capture_exit();
	}
	return OK;
}

int voip_volume_open_api(voip_volume_mode mode)
{
	zassert(voip_volume != NULL);
	if(VOIP_VOLUME_ALL == mode)
	{
		if(!voip_volume->c_isopen)
		{
			_VOIP_VOLUME_DEBUG( "======%s capture open", __func__);
			voip_volume_dsp_init();
			voip_capture_open_api(TRUE);
			voip_volume->c_isopen = TRUE;
		}
		if(!voip_volume->p_isopen)
		{
			_VOIP_VOLUME_DEBUG( "======%s playback open", __func__);
#ifndef VOIP_AMP_DEV_ENABLE
			voip_volume_power(TRUE);
#endif
			voip_volume->p_isopen = TRUE;
			voip_playback_open_api(TRUE);

		}
	}
	if(VOIP_VOLUME_PLAYBACK == mode)
	{
		if(!voip_volume->p_isopen)
		{
			_VOIP_VOLUME_DEBUG( "======%s capture open", __func__);
#ifndef VOIP_AMP_DEV_ENABLE
			voip_volume_power(TRUE);
#endif
			voip_volume->p_isopen = TRUE;
			voip_playback_open_api(TRUE);

		}
	}
	if(VOIP_VOLUME_CAPTURE == mode)
	{
		if(!voip_volume->c_isopen)
		{
			_VOIP_VOLUME_DEBUG( "======%s capture open", __func__);
			voip_volume_dsp_init();
			voip_capture_open_api(TRUE);
			voip_volume->c_isopen = TRUE;
		}
	}
	return OK;
}

int voip_volume_close_api(voip_volume_mode mode)
{
	zassert(voip_volume != NULL);
	if(VOIP_VOLUME_ALL == mode)
	{
		if(voip_volume->c_isopen)
		{
			_VOIP_VOLUME_DEBUG( "======%s capture close", __func__);
			voip_volume_dsp_exit();
			voip_capture_open_api(FALSE);
			voip_volume->c_isopen = FALSE;
		}
		if(voip_volume->p_isopen)
		{
			_VOIP_VOLUME_DEBUG( "======%s playback close", __func__);
#ifndef VOIP_AMP_DEV_ENABLE
			voip_volume_power(FALSE);
#endif
			voip_volume->p_isopen = FALSE;
			voip_playback_open_api(FALSE);

		}
	}
	if(VOIP_VOLUME_PLAYBACK == mode)
	{
		if(voip_volume->p_isopen)
		{
			_VOIP_VOLUME_DEBUG( "======%s playback close", __func__);
#ifndef VOIP_AMP_DEV_ENABLE
			voip_volume_power(FALSE);
#endif
			voip_volume->p_isopen = FALSE;
			voip_playback_open_api(FALSE);

		}
	}
	if(VOIP_VOLUME_CAPTURE == mode)
	{
		if(voip_volume->c_isopen)
		{
			_VOIP_VOLUME_DEBUG( "======%s capture close", __func__);
			voip_volume_dsp_exit();
			voip_capture_open_api(FALSE);
			voip_volume->c_isopen = FALSE;
		}
	}
	return OK;
}
#if 0//LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
static int voip_volume_open_new_api(voip_volume_mode mode)
{
	super_system("/app/etc/amixer-setup.sh" VOIP_NOHUP);		//
	super_system("/app/etc/amixer-setup.sh" VOIP_NOHUP);		//
	super_system("/app/etc/amixer-setup.sh" VOIP_NOHUP);		//
	super_system("/app/etc/amixer-setup.sh" VOIP_NOHUP);		//

	/*
	 * numid=20,iface=MIXER,name='TxDP Capture Volume'
	 * numid=21,iface=MIXER,name='STO1 ADC Boost Gain Volume'
	 * numid=15,iface=MIXER,name='IN2 Boost Volume'
	 * numid=14,iface=MIXER,name='IN1 Boost Volume'
	 * numid=16,iface=MIXER,name='IN Capture Volume'
	 * numid=18,iface=MIXER,name='ADC Capture Volume'
	 * amixer-setup.sh
	 */

	return OK;
}
static int voip_volume_close_new_api(voip_volume_mode mode)
{
	//ADC Capture Switch
	super_system("amixer cset numid=17 0 0" VOIP_NOHUP);

	super_system("amixer cset numid=3 0" VOIP_NOHUP);
	//OUT Playback Switch
	super_system("amixer cset numid=7 0 0" VOIP_NOHUP);
	return OK;
}
#endif

int voip_volume_control_api(BOOL enable)
{
#if 0//LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
	if(enable)
		return voip_volume_open_new_api(VOIP_VOLUME_ALL);
	else
		return voip_volume_close_new_api(VOIP_VOLUME_ALL);
#else
	if(enable)
		voip_volume_open_api(VOIP_VOLUME_ALL);
	else
		voip_volume_close_api(VOIP_VOLUME_ALL);

	if(enable)
		return voip_volume_open_api(VOIP_VOLUME_ALL);
	else
		return voip_volume_close_api(VOIP_VOLUME_ALL);
#endif
}

int voip_volume_show_config(struct vty *vty, int detail)
{
	zassert(voip_volume != NULL);
	u_int8 value = 0;
	//if(voip_config.enable)
	{
		/*
		 * Playback
		 */
		voip_playback_volume_out_get_api(&value);
		vty_out(vty, "  playback volume                    : %d%s", value, VTY_NEWLINE);
		voip_playback_volume_dac_get_api(&value);
		vty_out(vty, "  playback stereo volume             : %d%s", value, VTY_NEWLINE);
		voip_playback_volume_mono_get_api(&value);
		vty_out(vty, "  playback mono volume               : %d%s", value, VTY_NEWLINE);

		/*
		* Capture
		*/
		voip_capture_volume_in_get_api(&value);
		vty_out(vty, "  capture volume                     : %d%s", value, VTY_NEWLINE);
		voip_capture_volume_adc_get_api(&value);
		vty_out(vty, "  capture volume                     : %d%s", value, VTY_NEWLINE);
		voip_capture_volume_mono_get_api(&value);
		vty_out(vty, "  capture volume                     : %d%s", value, VTY_NEWLINE);
		voip_volume_boost_get_api(&value);
		vty_out(vty, "  capture boost                      : %d%s", value, VTY_NEWLINE);

		voip_volume_boost_gain_get_api(&value);
		vty_out(vty, "  capture boost-gain                 : %d%s", value, VTY_NEWLINE);
	}
	return OK;
}





int voip_volume_write_config(struct vty *vty)
{
	zassert(voip_volume != NULL);
	u_int8 value = 0;
//	if(voip_config.enable)
	{
		/*
		 * Playback
		 */
		voip_playback_volume_out_get_api(&value);
		if(value != VOIP_OUT_VOLUME)
			vty_out(vty, " voip playback volume %d%s", value, VTY_NEWLINE);

		voip_playback_volume_dac_get_api(&value);
		if(value != VOIP_OUT_STEREO_VOLUME)
			vty_out(vty, " voip playback stereo volume %d%s", value, VTY_NEWLINE);

		voip_playback_volume_mono_get_api(&value);
		if(value != VOIP_OUT_MONO_VOLUME)
			vty_out(vty, " voip playback mono volume %d%s", value, VTY_NEWLINE);

		/*
		* Capture
		*/
		voip_capture_volume_in_get_api(&value);
		if(value != VOIP_IN_VOLUME)
			vty_out(vty, " voip capture volume %d%s", value, VTY_NEWLINE);

		voip_capture_volume_adc_get_api(&value);
		if(value != VOIP_IN_ADC_VOLUME)
			vty_out(vty, " voip capture volume %d%s", value, VTY_NEWLINE);

		voip_capture_volume_mono_get_api(&value);
		if(value != VOIP_IN_MONO_VOLUME)
			vty_out(vty, " voip capture volume %d%s", value, VTY_NEWLINE);

		voip_volume_boost_get_api(&value);
		if(value != VOIP_IN_BOOST_VOLUME)
			vty_out(vty, " voip capture boost %d%s", value, VTY_NEWLINE);

		voip_volume_boost_gain_get_api(&value);
		if(value != VOIP_IN_BOOST_GAIN_VOLUME)
			vty_out(vty, " voip capture boost-gain %d%s", value, VTY_NEWLINE);

	}
	return OK;
}
