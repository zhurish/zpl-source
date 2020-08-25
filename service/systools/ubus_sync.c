/*
 * ubus_sync.c
 *
 *  Created on: 2019年2月11日
 *      Author: DELL
 */
#include "zebra.h"
#include "log.h"
#include "eloop.h"
#include "network.h"
#include "eloop.h"

#ifdef PL_UBUS_MODULE

#include "ubus_sync.h"

static ubus_sync_t ubus_sync_ctx;

int ubus_sync_debug(BOOL enable)
{
	if(enable)
		ubus_sync_ctx.debug |= UBUS_SYNC_DEBUG;
	else
		ubus_sync_ctx.debug &= ~UBUS_SYNC_DEBUG;
	return OK;
}

int ubus_sync_hook_install(ubus_sync_cb *cb, void *p)
{
	int i = 0;
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
	int i = 0;
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
	if(uci->sock)
		write(uci->sock, buf, 8);
	return OK;
}

static int ubus_sync_handle(ubus_sync_t *uci)
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
		if(uci->debug & UBUS_SYNC_DEBUG)
			zlog_debug(ZLOG_UTILS, "UCI UBUS read %d byte:'%s'",uci->len, uci->buf);
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
				zlog_debug(ZLOG_UTILS, "UCI UBUS respone OK");
			ubus_sync_respone(uci, 0);
			return OK;
		}
		else
		{
			if(uci->debug & UBUS_SYNC_DEBUG)
				zlog_debug(ZLOG_UTILS, "UCI UBUS respone ERROR");
			ubus_sync_respone(uci, 1);
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
			//zlog_debug(ZLOG_DEFAULT, "--------%s: reset udp socket", __func__);
			if(ubus->debug & UBUS_SYNC_DEBUG)
				zlog_debug(ZLOG_UTILS, "UCI UBUS socket close");
			//ubus_sync_reset();
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

static int ubus_sync_accept_eloop(struct eloop *thread)
{
	int sock = 0;
	int accept = ELOOP_FD(thread);
	ubus_sync_t *ubus = ELOOP_ARG(thread);
	//zlog_debug(ZLOG_DEFAULT, "--------%s:", __func__);
	ubus->t_accept = NULL;//eloop_add_read(ubus_sync_ctx.master, ubus_sync_accept_eloop, ubus, accept);
	if(ubus)
	{
		sock = unix_sock_accept(accept, NULL);
		if(sock > 0)
		{
			//zlog_debug(ZLOG_DEFAULT, "--------%s:%d", __func__, sock);
			if(ubus->sock > 0)
			{
				//ubus->t_accept = eloop_add_read(ubus->master, ubus_sync_accept_eloop, ubus, accept);
				//close(sock);
				//return ERROR;
				close(ubus->sock);
			}
			os_set_nonblocking(sock);
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
	int i = 0;
	int accept = 0;

	os_uci_init();

	memset(&ubus_sync_ctx, 0, sizeof(ubus_sync_ctx));
	ubus_sync_ctx.sock = 0;
	ubus_sync_ctx.master = NULL;
	ubus_sync_ctx.len = 0;
	memset(ubus_sync_ctx.buf, 0, sizeof(ubus_sync_ctx.buf));
	for(i = 0; i < UBUS_SYNC_CB_MAX; i++)
	{
		ubus_sync_ctx.cb[i] = NULL;
	}
	accept = unix_sock_server_create(TRUE, "lualuci");
	if(accept <= 0)
		return ERROR;
	os_set_nonblocking(accept);
	ubus_sync_ctx.accept = accept;
	ubus_sync_ctx.master = m;
	ubus_sync_ctx.t_accept = eloop_add_read(ubus_sync_ctx.master, ubus_sync_accept_eloop, &ubus_sync_ctx, accept);
	//ubus_sync_debug(TRUE);
	return OK;
}

int ubus_sync_reset(void)
{
	//int i = 0;
	int accept = 0;
	if(ubus_sync_ctx.t_read)
	{
		eloop_cancel(ubus_sync_ctx.t_read);
		ubus_sync_ctx.t_read = NULL;
	}
	if(ubus_sync_ctx.sock > 0)
		close(ubus_sync_ctx.sock);
	ubus_sync_ctx.sock = 0;

	if(ubus_sync_ctx.accept > 0)
		close(ubus_sync_ctx.accept);
	ubus_sync_ctx.accept = 0;

	//ubus_sync_ctx.master = NULL;
	ubus_sync_ctx.len = 0;
	memset(ubus_sync_ctx.buf, 0, sizeof(ubus_sync_ctx.buf));
	accept = unix_sock_server_create(TRUE, "lualuci");
	if(accept <= 0)
		return ERROR;
	//os_set_nonblocking(sock);
	ubus_sync_ctx.accept = accept;
	//ubus_sync_ctx.master = m;
	ubus_sync_ctx.t_accept = eloop_add_read(ubus_sync_ctx.master, ubus_sync_accept_eloop, &ubus_sync_ctx, accept);
	return OK;
}

int ubus_sync_exit(void)
{
	int i = 0;
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
	if(ubus_sync_ctx.sock > 0)
		close(ubus_sync_ctx.sock);
	ubus_sync_ctx.sock = 0;

	if(ubus_sync_ctx.accept > 0)
		close(ubus_sync_ctx.accept);
	ubus_sync_ctx.accept = 0;

	ubus_sync_ctx.master = NULL;
	ubus_sync_ctx.len = 0;
	memset(ubus_sync_ctx.buf, 0, sizeof(ubus_sync_ctx.buf));
	for(i = 0; i < UBUS_SYNC_CB_MAX; i++)
	{
		ubus_sync_ctx.cb[i] = NULL;
	}
	return OK;
}
#endif /* PL_UBUS_MODULE */

