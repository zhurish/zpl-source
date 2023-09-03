/**
 * @file     pjapp_app_util.h
 * @brief     : Description
 * @author   zhurish (zhurish@163.com)
 * @version  1.0
 * @date     2023-08-26
 * 
 * @copyright   Copyright (c) 2023 {author}({email}).Co.Ltd. All rights reserved.
 * 
 */
#ifndef __PJAPP_APP_UTIL_H__
#define __PJAPP_APP_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif


enum
{
	PJAPP_APP_SET_CMD,
	PJAPP_APP_ACK_CMD,
	PJAPP_APP_GET_CMD,
};

struct pjapp_sock_msg_hdr
{
	pj_uint32_t version;
	pj_uint32_t mask;
	pj_uint32_t cmd;
	pj_uint32_t len;
};


struct pjapp_msg_result
{
	pj_int32_t result;
	pj_uint32_t len;
};



int pjapp_sock_cfg_default(struct pjapp_sock_cfg *cfg);

int pjapp_sock_read_cmd(struct pjapp_sock_cfg *cfg);
int pjapp_sock_write_result(struct pjapp_sock_cfg *cfg, int ret, char *result);

int pjapp_sock_write_cmd(struct pjapp_sock_cfg *cfg, int cmd, char *data, int len);
int pjapp_sock_read_result(struct pjapp_sock_cfg *cfg, char *result);

#ifdef __cplusplus
}
#endif

#endif /* __PJAPP_APP_UTIL_H__ */
