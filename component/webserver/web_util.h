/*
 * web_util.h
 *
 *  Created on: 2019年8月26日
 *      Author: DELL
 */

#ifndef __WEB_UTIL_H__
#define __WEB_UTIL_H__

#include "zebra.h"
#include "if.h"
#include "vrf.h"
#include "nexthop.h"
#include "rib.h"


#define _WEB_DEBUG_ENABLE 1

#if defined(_WEB_DEBUG_ENABLE)
#define _WEB_DBG_ERR(format, ...) 		zlog_err (ZLOG_WEB, format, ##__VA_ARGS__)
#define _WEB_DBG_WARN(format, ...) 		zlog_warn (ZLOG_WEB, format, ##__VA_ARGS__)
#define _WEB_DBG_INFO(format, ...) 		zlog_info (ZLOG_WEB, format, ##__VA_ARGS__)
#define _WEB_DBG_DEBUG(format, ...) 	zlog_debug (ZLOG_WEB, format, ##__VA_ARGS__)
#define _WEB_DBG_TRAP(format, ...) 		zlog_trap (ZLOG_WEB, format, ##__VA_ARGS__)
#else
#define _WEB_DBG_ERR(format, ...)
#define _WEB_DBG_WARN(format, ...)
#define _WEB_DBG_INFO(format, ...)
#define _WEB_DBG_DEBUG(format, ...)
#define _WEB_DBG_TRAP(format, ...)
#endif

#if 0
#define WEBS_CRIT		0
#define WEBS_ALERT		1
#define WEBS_EMERG		2
#define WEBS_ERROR      3           /**< Hard error trace level */
#define WEBS_WARN       4           /**< Soft warning trace level */
#define WEBS_NOTICE		5
#define WEBS_INFO		6
#define WEBS_DEBUG		7
#define WEBS_TRAP		(WEBS_DEBUG+1)
#define WEBS_CONFIG     2           /**< Configuration settings trace level. */
#define WEBS_VERBOSE    9           /**< Highest level of trace */
/*
    Log message flags
 */
#define WEBS_ASSERT_MSG     0x10        /**< Originated from assert */
#define WEBS_ERROR_MSG      0x20        /**< Originated from error */
#define WEBS_LOG_MSG        0x100       /**< Originated from logmsg */
#define WEBS_RAW_MSG        0x200       /**< Raw message output */
#define WEBS_TRACE_MSG      0x400       /**< Originated from trace */
#define WEBS_EVENT_MSG      0x800       /**< Originated from trace */

#endif


#define _WEB_DEBUG_MSG		0X01
#define _WEB_DEBUG_DETAIL	0X200
#define _WEB_DEBUG_EVENT	0X800
#define _WEB_DEBUG_TRACE	0X400
#define _WEB_DEBUG_RAW		0X200
#define _WEB_DEBUG_HEADER   0x1000      /**< trace HTTP header */


#define WEB_IS_DEBUG(n)		(_WEB_DEBUG_ ## n & _web_app_debug)
#define WEB_DEBUG_ON(n)		{ _web_app_debug |= (_WEB_DEBUG_ ## n ); __websLogLevel |= (_WEB_DEBUG_ ## n );}
#define WEB_DEBUG_OFF(n)	{ _web_app_debug &= ~(_WEB_DEBUG_ ## n ); __websLogLevel &= ~(_WEB_DEBUG_ ## n );}


/*
#ifndef ME_GOAHEAD_UPLOAD_DIR
#define ME_GOAHEAD_UPLOAD_DIR SYSTFTPBOOTDIR
#endif
*/

/*
#ifndef ME_GOAHEAD_UPLOAD_DIR
    #define ME_GOAHEAD_UPLOAD_DIR "/home/zhurish/Downloads/tftpboot"
#endif
*/

#define HAS_BOOL 1
#include "src/goahead.h"

#include "web_api.h"

/*
#if ME_GOAHEAD_UPLOAD
#define WEB_UPLOAD_BASE ME_GOAHEAD_UPLOAD_DIR
#endif
*/



#define WEB_BTN_CB_MAX	128
#define WEB_ACTION_NAME_MAX	64


struct web_button_cb
{
	char action[WEB_ACTION_NAME_MAX];
	char ID[WEB_ACTION_NAME_MAX];
	int (*btn_cb)(Webs *, void *);
	void *pVoid;
};

#if ME_GOAHEAD_UPLOAD

#define WEB_UPLOAD_CB_MAX	32
/*
 * Upload
 */
struct web_upload_cb
{
	char form[WEB_ACTION_NAME_MAX];
	char ID[WEB_ACTION_NAME_MAX];
	int (*upload_cb)(Webs *, WebsUpload *up, void *);
	void *pVoid;
};

struct web_download_cb
{
	char form[WEB_ACTION_NAME_MAX];
	char ID[WEB_ACTION_NAME_MAX];
	int (*download_cb)(Webs *, char **);
	//void *pVoid;
};
#endif


extern const char * web_type_string(web_app_t *wp);
extern const char * web_os_type_string(web_app_t *wp);
extern web_type web_type_get();
extern web_os web_os_get();

extern int _web_app_debug;
/*
 * Webs Util
 */
extern const char *webs_get_var(Webs *wp, const char *var, const char *defaultGetValue);

extern int webs_json_parse(Webs *wp);
extern int webs_jsonarray_parse(Webs *wp);
extern int webs_jsonarray_next(Webs *wp);
extern int webs_json_finsh(Webs *wp);
extern int webs_json_get_var(Webs *wp, const char *var,
		char *valuestring, int *valueint, double *valuedouble);

/*
 * Password Encoded
 */
extern char * web_encoded_password(char *username, char *password, char *realm, char *cipher);

extern int web_gopass(const char *username, const char *password, const char *cipher,
						const char *realm,  char *encodedPassword);
extern int web_gopass_roles(const char *authFile, const char *username, const char *roles[]);
extern int web_gopass_save(const char *authFile, const char *username,
		const char *roles[], char *encodedPassword);
extern int web_auth_save(const char *authFile);

extern int webs_username_password_update(void *pwp, char *username, char *password);

/*
 * Return
 */
/*
 * 返回成功失败字串
 */
extern int web_return_text_plain(Webs *wp, int ret);

/*
 * 返回json数据
 */
extern int web_return_application_json(Webs *wp, char * json);
/*
 * 返回成功失败和原因的json
 */
extern int web_return_application_json_result(Webs *wp, int ret, char * result);
/*
 * 返回json列表
 */
extern int web_return_application_json_array(Webs *wp, int state, char * msg, char * json);

/*
 * Button
 */
extern int web_button_init(void);
extern int web_button_cb_init(void);
extern int web_button_add_hook(char *action, char *btnid, int(*cb)(void *, void *), void *);
extern int web_button_del_hook(char *action, char *btnid);

#define web_progress_view_add_hook(v,b,c,p) 	web_button_add_hook(v,b,c,p)
#define web_progress_view_del_hook(v,b) 		web_button_del_hook(v,b)
/*
 * Upload
 */
extern int web_updownload_cb_init(void);
extern int web_upload_add_hook(char *form, char *id, int (*cb)(void *, void *, void *),
		void *p);
extern int web_upload_del_hook(char *form, char *id);
extern int web_upload_call_hook(char *form, char *id, Webs *wp, WebsUpload *up);

extern int web_download_add_hook(char *form, char *id, int (*cb)(void *, char **));
extern int web_download_del_hook(char *form, char *id);
extern int web_download_setup(Webs *, char *, char *, char *, char **);
extern int web_download_call_hook(char *form, char *id, Webs *wp, char **filename);





/*
 * Route
 */
extern int web_route_lookup_default(ifindex_t ifindex, u_int32 *local_gateway);
extern int web_static_ipv4_safi (u_int8_t safi, int add_cmd,
			const char *dest_str, const char *mask_str,
			const char *gate_str, const char *flag_str,
			const char *tag_str, const char *distance_str,
			const char *vrf_id_str);

#ifdef WEB_OPENWRT_PROCESS
extern int web_kernel_route_lookup_default(ifindex_t ifindex, u_int32 *local_gateway );
extern int web_kernel_dns_lookup_default(ifindex_t *ifindex, u_int32 *dns1, u_int32 *dns2);
#endif


#endif /* __WEB_UTIL_H__ */
