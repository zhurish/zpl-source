/*
 * ipcstandby.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_IPCSTANBY_H__
#define __ZPL_IPCSTANBY_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ZPL_IPCSTANBY_TIMEOUT	2

enum ipcstanby_cmd
{
	ZPL_IPCSTANBY_REGISTER,
	ZPL_IPCSTANBY_HELLO,
	ZPL_IPCSTANBY_ACK,	
	ZPL_IPCSTANBY_CLI,
	ZPL_IPCSTANBY_MSG,
	ZPL_IPCSTANBY_RES,
};


struct ipcstanby_result
{
	int precoss;
	int result;
	int msglen;
};

struct ipcstandby_callback
{
    int (*ipcstandby_callback)(zpl_uint8 *, zpl_uint32, void *);
    void  *pVoid;
};

struct ipcstanby_reskey
{
	int key1;
	int key2;
	int key3;
	int key4;
	int key5;
	int key6;
	zpl_uint8 keystr1[32];
	zpl_uint8 keystr2[32];
	zpl_uint8 keystr3[32];
	zpl_uint8 keystr4[32];
	zpl_uint8 keystr5[32];
	zpl_uint8 keystr6[32];
};

struct ipcstandby
{
	zpl_uint32 slot;
	zpl_void *master; 
	zpl_taskid_t taskid;
	zpl_uint32    state;
	zpl_uint32    start_state;

	struct ipcstandby_server_t *ipcstandby_server;
	struct ipcstandby_client  *ipcstandby_client;

	/*主备倒换回调函数*/
    int (*ipcstandby_switch_callback)(zpl_bool);
};

extern struct ipcstandby  _host_standby;

extern void ipcstandby_create_header(struct stream *s, zpl_uint16 command);

extern int ipcstandby_execue_clicmd(char *cmd, int len);
extern int ipcstandby_sendto_msg(char *msg, int len);
extern int ipcstandby_request_res(int type, struct ipcstanby_reskey *reskey);

extern zpl_bool ipcstandby_done(int waitime);
extern int ipcstandby_switch_master(zpl_bool master);

extern void ipcstandby_cmd_init(void);
extern void ipcstandby_init(void *m, zpl_uint32 slot);
extern void ipcstandby_exit(void);


#ifdef __cplusplus
}
#endif

#endif /* __ZPL_IPCSTANBY_H__ */
