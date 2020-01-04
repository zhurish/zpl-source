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

#undef ME_GOAHEAD_UPLOAD_DIR
#ifndef ME_GOAHEAD_UPLOAD_DIR
#define ME_GOAHEAD_UPLOAD_DIR SYSUPLOADDIR
#define WEB_UPLOAD_BASE ME_GOAHEAD_UPLOAD_DIR
#endif

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
#include "goahead.h"

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
/*
 * Webs Util
 */
extern const char *webs_get_var(Webs *wp, const char *var, const char *defaultGetValue);


/*
 * Password Encoded
 */
extern char * web_encoded_password(char *username, char *password, char *realm, char *cipher);

extern int web_gopass(const char *username, const char *password, const char *cipher,
						const char *realm,  char *encodedPassword);
extern int web_gopass_roles(const char *authFile, const char *username, const char *roles[]);
extern int web_gopass_save(const char *authFile, const char *username,
		const char *roles[], char *encodedPassword);
/*
 * Return
 */
extern int web_return_text_plain(Webs *wp, int ret);
extern int web_return_application_json(Webs *wp, char * json);
extern int web_return_text_plain_json(Webs *wp, char * json);
/*
 * Button
 */
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

extern int web_download_add_hook(char *form, char *id, int (*cb)(void *, char **));
extern int web_download_del_hook(char *form, char *id);
extern int web_download_setup(Webs *, char *, char *, char *, char **);





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
