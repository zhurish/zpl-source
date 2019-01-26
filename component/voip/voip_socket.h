/*
 * voip_socket.h
 *
 *  Created on: 2019年1月3日
 *      Author: DELL
 */

#ifndef __VOIP_SOCKET_H__
#define __VOIP_SOCKET_H__

#pragma pack(1)
typedef struct _voip_socket_hdr
{
	u_int8 		type;
	u_int8 		magic;
	u_int16 	len;
	u_int8		data[256];
} voip_socket_hdr;
#pragma pack(0)


typedef struct voip_socket_s
{
	void		*master;
	int 		taskid;
	BOOL		enable;
	//int 		fd;
	int			accept;
	int			sock;
	void		*t_accept;
	void		*t_read;
	void		*t_write;
	void		*t_event;
	void		*t_time;
	u_int8		buf[1024];
	u_int8		sbuf[1024];
	u_int16		slen;

	int			mdctl;		//mediastream control socket
}voip_socket_t;

extern voip_socket_t voip_socket;

extern int voip_socket_module_init(void);
extern int voip_socket_module_exit(void);
extern int voip_socket_task_init();
extern int voip_socket_task_exit();


extern int voip_socket_sync_cmd();
/*
 * control mediastream
 */
extern int voip_socket_quit(voip_socket_t *socket);
extern int voip_socket_equalizer(voip_socket_t *socket, BOOL enable, float frequency, float gain, float width);
extern int voip_socket_lossrate(voip_socket_t *socket, int lossrate);
extern int voip_socket_bandwidth(voip_socket_t *socket, int bandwidth);


#endif /* __VOIP_SOCKET_H__ */
