/*
 * zpl_ipcmsg.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_IPCMSG_H__
#define __ZPL_IPCMSG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ZPL_IPCMSG_MAX_PACKET_SIZ 4096
#define ZPL_IPCMSG_VERSION 1
#define ZPL_IPCMSG_HEADER_MARKER 255
#define ZPL_IPCMSG_HEADER_SIZE 6

enum ipcmsg_event
{
	ZPL_IPCMSG_NONE,
	ZPL_IPCMSG_ACCEPT,
	ZPL_IPCMSG_READ,
	ZPL_IPCMSG_WRITE,
	ZPL_IPCMSG_CONNECT,
	ZPL_IPCMSG_TIMEOUT,
	ZPL_IPCMSG_EVENT,
	ZPL_IPCMSG_SCHEDULE
};



/* Debug flags. */
#define ZPL_IPCMSG_DEBUG_EVENT 0x01
#define ZPL_IPCMSG_DEBUG_PACKET 0x01
#define ZPL_IPCMSG_DEBUG_SEND 0x20
#define ZPL_IPCMSG_DEBUG_RECV 0x40
#define ZPL_IPCMSG_DEBUG_DETAIL 0x80

/* Debug related macro. */
#define IS_ZPL_IPCMSG_DEBUG_EVENT(f) (f & ZPL_IPCMSG_DEBUG_EVENT)
#define IS_ZPL_IPCMSG_DEBUG_PACKET(f) (f & ZPL_IPCMSG_DEBUG_PACKET)
#define IS_ZPL_IPCMSG_DEBUG_SEND(f) (f & ZPL_IPCMSG_DEBUG_SEND)
#define IS_ZPL_IPCMSG_DEBUG_RECV(f) (f & ZPL_IPCMSG_DEBUG_RECV)
#define IS_ZPL_IPCMSG_DEBUG_DETAIL(f) (f & ZPL_IPCMSG_DEBUG_DETAIL)



#ifdef __cplusplus
}
#endif

#endif /* __ZPL_IPCMSG_H__ */
