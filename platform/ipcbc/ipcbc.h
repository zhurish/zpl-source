/*
 * ipcbc.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_IPCBC_H__
#define __ZPL_IPCBC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ZPL_IPCBC_TIMEOUT	2

enum ipcbc_cmd
{
	ZPL_IPCBC_REGISTER,
	ZPL_IPCBC_HELLO,
	ZPL_IPCBC_ACK,	
	ZPL_IPCBC_SET,
	ZPL_IPCBC_GET,
};


struct ipcbc_result
{
	int result;
	int msglen;
};

struct ipcbc_callback
{
    int (*ipcbc_callback)(zpl_uint32, zpl_uint8 *, zpl_uint32, void *);
    void  *pVoid;
};

struct ipcbc_reskey
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

struct ipcbc
{
	zpl_uint32 slot;
	zpl_void *master; 
	zpl_taskid_t taskid;
	zpl_uint32  state;
	zpl_uint32  start_state;

};


extern void ipcbc_create_header(struct stream *s, zpl_uint16 cmd);

extern int ipcbc_sendto_msg(char *msg, int len);
extern int ipcbc_request_res(int type, struct ipcbc_reskey *reskey);


extern void ipcbc_cmd_init(void);
extern void ipcbc_init(void *m, zpl_uint32 slot);
extern void ipcbc_exit(void);


#ifdef __cplusplus
}
#endif

#endif /* __ZPL_IPCBC_H__ */
