/*
 * webutil.h
 *
 *  Created on: 2019年8月26日
 *      Author: DELL
 */

#ifndef _goahead_WEBUTIL_H__
#define _goahead_WEBUTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define T(n)	(n)

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
};
#endif


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
extern int web_auth_save(const char *authFile);


/*
 * Return
 */
/*
 * 返回成功失败字串
 */
extern int web_return_text_plain(Webs *wp, int ret, char *msg);
extern int web_return_text_fmt(Webs *wp, int ret, char *fmt,...);

/*
 * 返回json数据
 */
#if ME_GOAHEAD_JSON
extern int web_return_application_json(Webs *wp,  int ret, cJSON* json);
extern int web_return_application_json_fmt(Webs *wp, int ret, char *fmt,...);
#endif
/*
 * Button
 按钮操作：按钮事件需要按钮ID，按钮表单action和回调函数组成；前端提交按钮事件的时候提交
 按钮表单 action/button_onclick,数据封装支持application/json和application/x-www-form-urlencoded
 */
extern int web_button_init(void);
extern int web_button_onclick_init(void);
extern int web_button_add_hook(char *action, char *btnid, int(*cb)(void *, void *), void *);
extern int web_button_del_hook(char *action, char *btnid);

#define web_progress_view_add_hook(v,b,c,p) 	web_button_add_hook(v,b,c,p)
#define web_progress_view_del_hook(v,b) 		web_button_del_hook(v,b)
/*
 * Upload
 */

extern int web_upload_add_hook(char *form, char *id, int (*cb)(void *, void *, void *),
		void *p);
extern int web_upload_del_hook(char *form, char *id);
extern int web_upload_call_hook(char *form, char *id, Webs *wp, WebsUpload *up);

extern int web_download_add_hook(char *form, char *id, int (*cb)(void *, char **));
extern int web_download_del_hook(char *form, char *id);
extern int web_download_setup(Webs *, char *, char *, char *, char **);
extern int web_download_call_hook(char *form, char *id, Webs *wp, char **filename);


 
#ifdef __cplusplus
}
#endif
 
#endif /* _goahead_WEBUTIL_H__ */
