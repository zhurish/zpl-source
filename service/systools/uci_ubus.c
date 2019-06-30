/*
 * uci_ubus.c
 *
 *  Created on: 2019年2月11日
 *      Author: DELL
 */
#include "zebra.h"
#include "log.h"
#include "eloop.h"
#include "network.h"
#include "eloop.h"

#ifdef PL_OPENWRT_UCI
#if 1//def BUILD_OPENWRT
/*#include <libubus.h>
#include <libubox/blobmsg.h>


int uci_ubus_cb_install(uci_ubus_cb *cb)
{
	return OK;
}

int uci_ubus_cb_uninstall(uci_ubus_cb *cb)
{
	return OK;
}

int uci_ubus_debug(BOOL enable)
{
	return OK;
}

int uci_ubus_init(void *m)
{
	return OK;
}

int uci_ubus_reset(void)
{
	return OK;
}

int uci_ubus_exit(void)
{
	return OK;
}*/

//#else /* BUILD_OPENWRT */

#include "os_uci.h"
#include "uci_ubus.h"

static uci_ubus_t uci_ubus_ctx;

int uci_ubus_debug(BOOL enable)
{
	if(enable)
		uci_ubus_ctx.debug |= UCI_UBUS_DEBUG;
	else
		uci_ubus_ctx.debug &= ~UCI_UBUS_DEBUG;
	return OK;
}

int uci_ubus_cb_install(uci_ubus_cb *cb)
{
	int i = 0;
	for(i = 0; i < UCI_UBUS_CB_MAX; i++)
	{
		if(uci_ubus_ctx.cb[i] == NULL)
		{
			uci_ubus_ctx.cb[i] = cb;
			return OK;
		}
	}
	return ERROR;
}

int uci_ubus_cb_uninstall(uci_ubus_cb *cb)
{
	int i = 0;
	for(i = 0; i < UCI_UBUS_CB_MAX; i++)
	{
		if(uci_ubus_ctx.cb[i] == cb)
		{
			uci_ubus_ctx.cb[i] = NULL;
			return OK;
		}
	}
	return ERROR;
}

static int uci_ubus_respone(uci_ubus_t *uci, int res)
{
	char buf[64];
	int	*head = (int*)buf;
	int	*respone = (int*)(buf + 4);
	if(!uci)
		return ERROR;
	memset(buf, 0, sizeof(buf));
	*head = htonl(0XAABBCCDD);
	*respone = htonl(res);
	if(uci->sock)
		write(uci->sock, buf, 8);
	return OK;
}

static int uci_ubus_handle(uci_ubus_t *uci)
{
	int ret = 0;
	if(!uci)
	{
		//zlog_debug(ZLOG_UTILS, "---------------------------");
		return ERROR;
	}
	memset(uci->buf, 0, sizeof(uci->buf));
	if(uci->sock <= 0)
	{
		//zlog_debug(ZLOG_UTILS, "------------ sock ----------");
		return ERROR;
	}
	uci->len = ret = read(uci->sock, uci->buf, sizeof(uci->buf));
	if(uci->len > 0)
	{
		int i = 0;
		if(uci->debug & UCI_UBUS_DEBUG)
			zlog_debug(ZLOG_UTILS, "UCI UBUS read %d byte:'%s'",uci->len, uci->buf);
		ret = 0;
		for(i = 0; i < UCI_UBUS_CB_MAX; i++)
		{
			if(uci->cb[i])
			{
				ret |= uci->cb[i](uci->buf, uci->len);
			}
		}
		if(ret == OK)
		{
			if(uci->debug & UCI_UBUS_DEBUG)
				zlog_debug(ZLOG_UTILS, "UCI UBUS respone OK");
			uci_ubus_respone(uci, 0);
			return OK;
		}
		else
		{
			if(uci->debug & UCI_UBUS_DEBUG)
				zlog_debug(ZLOG_UTILS, "UCI UBUS respone ERROR");
			uci_ubus_respone(uci, 1);
			return OK;
		}
	}
	else
	{
		//zlog_debug(ZLOG_UTILS, "------------ read %s ----------", strerror(errno));
		if(ret < 0)
		{
			if (ERRNO_IO_RETRY(errno))
				return OK;
		}
		return ERROR;
	}
	return OK;
}


static int uci_ubus_read_eloop(struct eloop *thread)
{
	//int sock = ELOOP_FD(thread);
	uci_ubus_t *ubus = ELOOP_ARG(thread);
	ubus->t_read = NULL;
	if(ubus)
	{
		if(uci_ubus_handle(ubus) == OK)
		{
			if(ubus->master)
				ubus->t_read = eloop_add_read(ubus->master, uci_ubus_read_eloop, ubus, ubus->sock);
			return OK;
		}
		else
		{
			//zlog_debug(ZLOG_DEFAULT, "--------%s: reset udp socket", __func__);
			if(ubus->debug & UCI_UBUS_DEBUG)
				zlog_debug(ZLOG_UTILS, "UCI UBUS socket close");
			//uci_ubus_reset();
			close(ubus->sock);
			ubus->sock = 0;
			return ERROR;
		}
	}
	//zlog_debug(ZLOG_UTILS, "-------------UCI UBUS socket close");
	close(ubus->sock);
	ubus->sock = 0;
	return ERROR;
}

static int uci_ubus_accept_eloop(struct eloop *thread)
{
	int sock = 0;
	int accept = ELOOP_FD(thread);
	uci_ubus_t *ubus = ELOOP_ARG(thread);
	//zlog_debug(ZLOG_DEFAULT, "--------%s:", __func__);
	ubus->t_accept = NULL;//eloop_add_read(uci_ubus_ctx.master, uci_ubus_accept_eloop, ubus, accept);
	if(ubus)
	{
		sock = unix_sock_accept(accept, NULL);
		if(sock > 0)
		{
			//zlog_debug(ZLOG_DEFAULT, "--------%s:%d", __func__, sock);
			if(ubus->sock > 0)
			{
				//ubus->t_accept = eloop_add_read(ubus->master, uci_ubus_accept_eloop, ubus, accept);
				//close(sock);
				//return ERROR;
				close(ubus->sock);
			}
			os_set_nonblocking(sock);
			ubus->sock = sock;
			if(ubus->master)
			{
				ubus->t_accept = eloop_add_read(ubus->master, uci_ubus_accept_eloop, ubus, accept);
				ubus->t_read = eloop_add_read(ubus->master, uci_ubus_read_eloop, ubus, sock);
			}
		}
		else
			ubus->t_accept = eloop_add_read(ubus->master, uci_ubus_accept_eloop, ubus, accept);
	}
	return OK;
}

int uci_ubus_init(void *m)
{
	int i = 0;
	int accept = 0;

	os_uci_init();

	memset(&uci_ubus_ctx, 0, sizeof(uci_ubus_ctx));
	uci_ubus_ctx.sock = 0;
	uci_ubus_ctx.master = NULL;
	uci_ubus_ctx.len = 0;
	memset(uci_ubus_ctx.buf, 0, sizeof(uci_ubus_ctx.buf));
	for(i = 0; i < UCI_UBUS_CB_MAX; i++)
	{
		uci_ubus_ctx.cb[i] = NULL;
	}
	accept = unix_sock_server_create(TRUE, "lualuci");
	if(accept <= 0)
		return ERROR;
	os_set_nonblocking(accept);
	uci_ubus_ctx.accept = accept;
	uci_ubus_ctx.master = m;
	uci_ubus_ctx.t_accept = eloop_add_read(uci_ubus_ctx.master, uci_ubus_accept_eloop, &uci_ubus_ctx, accept);
	uci_ubus_debug(TRUE);
	return OK;
}

int uci_ubus_reset(void)
{
	//int i = 0;
	int accept = 0;
	if(uci_ubus_ctx.t_read)
	{
		eloop_cancel(uci_ubus_ctx.t_read);
		uci_ubus_ctx.t_read = NULL;
	}
	if(uci_ubus_ctx.sock > 0)
		close(uci_ubus_ctx.sock);
	uci_ubus_ctx.sock = 0;

	if(uci_ubus_ctx.accept > 0)
		close(uci_ubus_ctx.accept);
	uci_ubus_ctx.accept = 0;

	//uci_ubus_ctx.master = NULL;
	uci_ubus_ctx.len = 0;
	memset(uci_ubus_ctx.buf, 0, sizeof(uci_ubus_ctx.buf));
	accept = unix_sock_server_create(TRUE, "lualuci");
	if(accept <= 0)
		return ERROR;
	//os_set_nonblocking(sock);
	uci_ubus_ctx.accept = accept;
	//uci_ubus_ctx.master = m;
	uci_ubus_ctx.t_accept = eloop_add_read(uci_ubus_ctx.master, uci_ubus_accept_eloop, &uci_ubus_ctx, accept);
	return OK;
}

int uci_ubus_exit(void)
{
	int i = 0;
	if(uci_ubus_ctx.t_read)
	{
		eloop_cancel(uci_ubus_ctx.t_read);
		uci_ubus_ctx.t_read = NULL;
	}
	if(uci_ubus_ctx.t_accept)
	{
		eloop_cancel(uci_ubus_ctx.t_accept);
		uci_ubus_ctx.t_accept = NULL;
	}
	if(uci_ubus_ctx.sock > 0)
		close(uci_ubus_ctx.sock);
	uci_ubus_ctx.sock = 0;

	if(uci_ubus_ctx.accept > 0)
		close(uci_ubus_ctx.accept);
	uci_ubus_ctx.accept = 0;

	uci_ubus_ctx.master = NULL;
	uci_ubus_ctx.len = 0;
	memset(uci_ubus_ctx.buf, 0, sizeof(uci_ubus_ctx.buf));
	for(i = 0; i < UCI_UBUS_CB_MAX; i++)
	{
		uci_ubus_ctx.cb[i] = NULL;
	}
	return OK;
}
#endif /* BUILD_OPENWRT */
#endif
