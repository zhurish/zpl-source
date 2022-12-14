#ifndef __ONVIF_IMPL_H__
#define __ONVIF_IMPL_H__

#include <time.h>

#define DBG_LINE printf("enter: %s\n", __FUNCTION__);
#define DBG_TAG(l) printf("dbg:%s %s\n", __FUNCTION__, l);

#define TRUE 1
#define FALSE 0


#define CHECK_FIELD(em) \
if (em == NULL) {\
	size_t size = sizeof(*em);\
	em = soap_malloc(soap, size);\
	memset(em, 0, size);\
}\


#define MEM_CHECK_FIELD(em, n) \
if (em == NULL) {\
	size_t size = sizeof(*em)*n;\
	em = soap_malloc(soap, size);\
	memset(em, 0, size);\
}\

#define LOAD_FIELD(obj1, name1, obj2, name2) \
	(obj1).(name1) = (obj2).(name2);

#define STORE_FIELD(obj1, name1, obj2, name2) \
	 (obj2).(name2) = (obj1).(name1);



#define LOAD(val1, val2) \
	val1 = val2;

#define STORE(val1, val2) \
	 val2 = val1;


void soap_set_field_string(struct soap* soap, char ** p_field, const char* val);


const char * _hw_onvif_get_ipv4_address();
const char * _hw_onvif_get_wsdl_url();
const char * _hw_onvif_get_service_url();
const char * _hw_onvif_get_service_namespace();
void _hw_onvif_get_service_version(int* major, int * minor);

const char* _hw_onvif_get_manufacturer();
const char* _hw_onvif_get_model();
const char* _hw_onvif_get_firmware_version();
const char* _hw_onvif_get_serial_number();
const char* _hw_onvif_get_hardware_id();


void _hw_onvif_set_system_date_time(int is_ntp, struct tm* now);
void _hw_onvif_get_system_date_time(int* is_ntp, struct tm* now);

void _hw_onvif_set_system_factory_default(int type);
void _hw_onvif_reboot_system();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int _hw_onvif_media_get_video_frame_rate();
int _hw_onvif_media_get_video_width();
int _hw_onvif_media_get_video_height();
char* _hw_onvif_media_get_video_token();

float _hw_onvif_media_get_video_brightness();
float _hw_onvif_media_get_video_color_saturation();
float _hw_onvif_media_get_video_contrast();
int _hw_onvif_media_get_video_ir_cut_filter();
float _hw_onvif_media_get_video_sharpness();
int _hw_onvif_media_get_video_backlight_compensation_mode();
int _hw_onvif_media_get_video_backlight_compensation_level();
int _hw_onvif_media_get_video_wide_dynamic_range_mode();
int _hw_onvif_media_get_video_wide_dynamic_range_level();
int _hw_onvif_media_get_video_white_balance_mode();
int _hw_onvif_media_get_video_white_balance_cr_gain();
int _hw_onvif_media_get_video_white_balance_cb_gain();

#endif