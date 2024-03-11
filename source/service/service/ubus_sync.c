/*
 * ubus_sync.c
 *
 *  Created on: 2019年2月11日
 *      Author: DELL
 */
#include "service.h"
#include "vty.h"
#include "zmemory.h"
#include "command.h"
#ifdef ZPL_SERVICE_UBUS_SYNC

#include "ubus_sync.h"

static ubus_sync_t ubus_sync_ctx;

int ubus_sync_debug(zpl_bool enable)
{
	if(enable)
		ubus_sync_ctx.debug |= UBUS_SYNC_DEBUG;
	else
		ubus_sync_ctx.debug &= ~UBUS_SYNC_DEBUG;
	return OK;
}

int ubus_sync_hook_install(ubus_sync_cb *cb, void *p)
{
	zpl_uint32 i = 0;
	for(i = 0; i < UBUS_SYNC_CB_MAX; i++)
	{
		if(ubus_sync_ctx.cb[i] == NULL)
		{
			ubus_sync_ctx.cb[i] = cb;
			ubus_sync_ctx.cb_argvs[i] = p;
			return OK;
		}
	}
	return ERROR;
}

int ubus_sync_hook_uninstall(ubus_sync_cb *cb, void *p)
{
	zpl_uint32 i = 0;
	for(i = 0; i < UBUS_SYNC_CB_MAX; i++)
	{
		if(ubus_sync_ctx.cb[i] == cb)
		{
			ubus_sync_ctx.cb[i] = NULL;
			ubus_sync_ctx.cb_argvs[i] = NULL;
			return OK;
		}
	}
	return ERROR;
}

static int ubus_sync_respone(ubus_sync_t *uci, int res)
{
	char buf[64];
	int	*head = (int*)buf;
	int	*respone = (int*)(buf + 4);
	if(!uci)
		return ERROR;
	memset(buf, 0, sizeof(buf));
	*head = htonl(0XAABBCCDD);
	*respone = htonl(res);
	if(!ipstack_invalid(uci->sock))
		ipstack_write(uci->sock, buf, 8);
	return OK;
}

static int ubus_sync_handle(ubus_sync_t *uci)
{
	int ret = 0;
	if(!uci)
	{
		//zlog_debug(MODULE_SERVICE, "---------------------------");
		return ERROR;
	}
	memset(uci->buf, 0, sizeof(uci->buf));
	if(ipstack_invalid(uci->sock))
	{
		//zlog_debug(MODULE_SERVICE, "------------ sock ----------");
		return ERROR;
	}
	uci->len = ret = ipstack_read(uci->sock, uci->buf, sizeof(uci->buf));
	if(uci->len > 0)
	{
		zpl_uint32 i = 0;
		if(uci->debug & UBUS_SYNC_DEBUG)
			zlog_debug(MODULE_SERVICE, "UCI UBUS read %d byte:'%s'",uci->len, uci->buf);
		ret = 0;
		for(i = 0; i < UBUS_SYNC_CB_MAX; i++)
		{
			if(uci->cb[i])
			{
				ret |= uci->cb[i](uci->cb_argvs, uci->buf, uci->len);
			}
		}
		if(ret == OK)
		{
			if(uci->debug & UBUS_SYNC_DEBUG)
				zlog_debug(MODULE_SERVICE, "UCI UBUS respone OK");
			ubus_sync_respone(uci, 0);
			return OK;
		}
		else
		{
			if(uci->debug & UBUS_SYNC_DEBUG)
				zlog_debug(MODULE_SERVICE, "UCI UBUS respone ERROR");
			ubus_sync_respone(uci, 1);
			return OK;
		}
	}
	else
	{
		//zlog_debug(MODULE_SERVICE, "------------ read %s ----------", strerror(ipstack_errno));
		if(ret < 0)
		{
			if (IPSTACK_ERRNO_RETRY(ipstack_errno))
				return OK;
		}
		return ERROR;
	}
	return OK;
}


static int ubus_sync_read_eloop(struct eloop *thread)
{
	//int sock = ELOOP_FD(thread);
	ubus_sync_t *ubus = ELOOP_ARG(thread);
	ubus->t_read = NULL;
	if(ubus)
	{
		if(ubus_sync_handle(ubus) == OK)
		{
			if(ubus->master)
				ubus->t_read = eloop_add_read(ubus->master, ubus_sync_read_eloop, ubus, ubus->sock);
			return OK;
		}
		else
		{
			//zlog_debug(MODULE_SERVICE, "--------%s: reset udp ipstack_socket", __func__);
			if(ubus->debug & UBUS_SYNC_DEBUG)
				zlog_debug(MODULE_SERVICE, "UCI UBUS ipstack_socket close");
			//ubus_sync_reset();
			ipstack_close(ubus->sock);
			//ubus->sock = 0;
			return ERROR;
		}
	}
	//zlog_debug(MODULE_SERVICE, "-------------UCI UBUS ipstack_socket close");
	ipstack_close(ubus->sock);
	//ubus->sock = 0;
	return ERROR;
}

static int ubus_sync_accept_eloop(struct eloop *thread)
{
	zpl_socket_t sock;
	zpl_socket_t accept = ELOOP_FD(thread);
	ubus_sync_t *ubus = ELOOP_ARG(thread);
	//zlog_debug(MODULE_SERVICE, "--------%s:", __func__);
	ubus->t_accept = NULL;//eloop_add_read(ubus_sync_ctx.master, ubus_sync_accept_eloop, ubus, ipstack_accept);
	if(ubus)
	{
		ipstack_fd(sock) = os_sock_unix_accept(ipstack_fd(accept), NULL);
		if(ipstack_fd(sock) > 0)
		{
			//zlog_debug(MODULE_SERVICE, "--------%s:%d", __func__, sock);
			if(ipstack_fd(ubus->sock) > 0)
			{
				//ubus->t_accept = eloop_add_read(ubus->master, ubus_sync_accept_eloop, ubus, ipstack_accept);
				//close(sock);
				//return ERROR;
				ipstack_close(ubus->sock);
			}
			os_set_nonblocking(ipstack_fd(sock));
			ubus->sock = sock;
			if(ubus->master)
			{
				ubus->t_accept = eloop_add_read(ubus->master, ubus_sync_accept_eloop, ubus, accept);
				ubus->t_read = eloop_add_read(ubus->master, ubus_sync_read_eloop, ubus, sock);
			}
		}
		else
			ubus->t_accept = eloop_add_read(ubus->master, ubus_sync_accept_eloop, ubus, accept);
	}
	return OK;
}

int ubus_sync_init(void *m)
{
	zpl_uint32 i = 0;
	int iaccept = 0;

	os_uci_init();

	memset(&ubus_sync_ctx, 0, sizeof(ubus_sync_ctx));
	ubus_sync_ctx.sock = ipstack_create(IPSTACK_OS);
	//ipstack_fd(ubus_sync_ctx.sock) = 0;
	//ipstack_type(ubus_sync_ctx.sock) = IPSTACK_OS;
	ubus_sync_ctx.master = NULL;
	ubus_sync_ctx.len = 0;
	memset(ubus_sync_ctx.buf, 0, sizeof(ubus_sync_ctx.buf));
	for(i = 0; i < UBUS_SYNC_CB_MAX; i++)
	{
		ubus_sync_ctx.cb[i] = NULL;
	}
	iaccept = os_sock_unix_server_create(zpl_true, "lualuci");
	if(iaccept <= 0)
		return ERROR;
	os_set_nonblocking(iaccept);
	ubus_sync_ctx.accept = ipstack_create(IPSTACK_OS);

	ipstack_fd(ubus_sync_ctx.accept) = iaccept;
	ipstack_type(ubus_sync_ctx.accept) = IPSTACK_OS;
	ubus_sync_ctx.master = m;
	ubus_sync_ctx.t_accept = eloop_add_read(ubus_sync_ctx.master, ubus_sync_accept_eloop, &ubus_sync_ctx, ubus_sync_ctx.accept);
	//ubus_sync_debug(zpl_true);
	return OK;
}

int ubus_sync_reset(void)
{
	//zpl_uint32 i = 0;
	int iaccept = 0;
	if(ubus_sync_ctx.t_read)
	{
		eloop_cancel(ubus_sync_ctx.t_read);
		ubus_sync_ctx.t_read = NULL;
	}
	if(ipstack_fd(ubus_sync_ctx.sock) > 0)
		close(ipstack_fd(ubus_sync_ctx.sock));
	ipstack_fd(ubus_sync_ctx.sock) = 0;

	if(ipstack_fd(ubus_sync_ctx.accept) > 0)
		close(ipstack_fd(ubus_sync_ctx.accept));
	ipstack_fd(ubus_sync_ctx.accept) = 0;

	//ubus_sync_ctx.master = NULL;
	ubus_sync_ctx.len = 0;
	memset(ubus_sync_ctx.buf, 0, sizeof(ubus_sync_ctx.buf));
	iaccept = os_sock_unix_server_create(zpl_true, "lualuci");
	if(iaccept <= 0)
		return ERROR;
	//os_set_nonblocking(sock);
	ipstack_fd(ubus_sync_ctx.accept) = iaccept;
	//ubus_sync_ctx.master = m;
	ubus_sync_ctx.t_accept = eloop_add_read(ubus_sync_ctx.master, ubus_sync_accept_eloop, &ubus_sync_ctx, ubus_sync_ctx.accept);
	return OK;
}

int ubus_sync_exit(void)
{
	zpl_uint32 i = 0;
	if(ubus_sync_ctx.t_read)
	{
		eloop_cancel(ubus_sync_ctx.t_read);
		ubus_sync_ctx.t_read = NULL;
	}
	if(ubus_sync_ctx.t_accept)
	{
		eloop_cancel(ubus_sync_ctx.t_accept);
		ubus_sync_ctx.t_accept = NULL;
	}
	if(ipstack_fd(ubus_sync_ctx.sock) > 0)
		close(ipstack_fd(ubus_sync_ctx.sock));
	ipstack_fd(ubus_sync_ctx.sock) = 0;

	if(ipstack_fd(ubus_sync_ctx.accept) > 0)
		close(ipstack_fd(ubus_sync_ctx.accept));
	ipstack_fd(ubus_sync_ctx.accept) = 0;

	ubus_sync_ctx.master = NULL;
	ubus_sync_ctx.len = 0;
	memset(ubus_sync_ctx.buf, 0, sizeof(ubus_sync_ctx.buf));
	for(i = 0; i < UBUS_SYNC_CB_MAX; i++)
	{
		ubus_sync_ctx.cb[i] = NULL;
	}
	return OK;
}
#endif /* ZPL_SERVICE_UBUS_SYNC */

