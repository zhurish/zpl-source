/* $Id$ */
/*
 * Copyright (C) 2010 Teluu Inc. (http://www.teluu.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <unistd.h>
#include <fcntl.h>
#include <pjlib-util/cli_imp.h>
#include <pjlib-util/cli_socket.h>
#include <pj/sock_select.h>
#include <pj/activesock.h>
#include <pj/assert.h>
#include <pj/errno.h>
#include <pj/log.h>
#include <pj/os.h>
#include <pj/pool.h>
#include <pj/string.h>
#include <pj/except.h>
#include <pjlib-util/errno.h>
#include <pjlib-util/scanner.h>
#include <pj/addr_resolv.h>
#include <pj/compat/socket.h>


#define CLI_SOCKET_BUF_SIZE 256


typedef struct cli_socket_fe
{
    pj_cli_front_end        base;
    pj_pool_t              *pool;
    pj_cli_socket_cfg       *cfg;
    pj_thread_t        		*input_thread;
    pj_bool_t           	thread_quit;
    pj_mutex_t             *mutex;

    pj_cli_sess        *sess;
    unsigned char	    buf[CLI_SOCKET_BUF_SIZE];
    unsigned		    buf_len;
} cli_socket_fe;


static int socket_worker_thread(void *p);


static int cli_set_nonblocking(int fd)
{
	int flags = 0;

	/* According to the Single UNIX Spec, the return value for F_GETFL should
	 never be negative. */
	if ((flags = fcntl(fd, F_GETFL)) < 0)
	{
		fprintf(stdout, "fcntl(F_GETFL) failed for fd %d set-nonblocking: %s", fd,
				strerror(errno));
		return -1;
	}
	if (fcntl(fd, F_SETFL, (flags | O_NONBLOCK)) < 0)
	{
		fprintf(stdout, "fcntl failed setting fd %d set-nonblocking: %s", fd,
				strerror(errno));
		return -1;
	}
	return 0;
}

static void cli_socket_quit(pj_cli_front_end *fe, pj_cli_sess *req)
{
	cli_socket_fe * cfe = (cli_socket_fe *)fe;

    PJ_UNUSED_ARG(req);

    pj_assert(cfe);
    if (cfe->input_thread) {
        cfe->thread_quit = PJ_TRUE;
    }
}

static void cli_socket_front_destroy(pj_cli_front_end *fe)
{
	cli_socket_fe * cfe = (cli_socket_fe *)fe;

    pj_assert(cfe);
    if(cfe->mutex)
    	pj_mutex_lock (cfe->mutex);
    if (cfe->input_thread)
        cfe->thread_quit = PJ_TRUE;
    if(cfe->mutex)
    	pj_mutex_unlock (cfe->mutex);
    if (cfe->input_thread)
    {
        pj_thread_join(cfe->input_thread);
        //pj_thread_destroy(cfe->input_thread);
        //cfe->input_thread = NULL;
    }
    if(cfe->mutex)
    	pj_mutex_lock (cfe->mutex);
    //pj_list_erase(cfe->sess);
	if(cfe->sess)
	{
		cfe->sess = NULL;
	}
    if (cfe->cfg->rfd != PJ_INVALID_SOCKET)
    {
        pj_sock_close(cfe->cfg->rfd);
        cfe->cfg->rfd = PJ_INVALID_SOCKET;
    }
    if (cfe->cfg->wfd != PJ_INVALID_SOCKET)
    {
        pj_sock_close(cfe->cfg->wfd);
        cfe->cfg->wfd = PJ_INVALID_SOCKET;
    }
    if (cfe->input_thread) {
        pj_thread_destroy(cfe->input_thread);
        cfe->input_thread = NULL;
    }

    if (cfe->mutex)
    {
    	pj_mutex_unlock (cfe->mutex);
        pj_mutex_destroy(cfe->mutex);
        cfe->mutex = NULL;
    }
    pj_pool_release(cfe->pool);
}



static void cli_socket_exec_result(pj_cli_sess *sess, int result)
{
	int len = 0;
	char buf[64];
	int *p_result = buf;
	cli_socket_fe *fe = (cli_socket_fe *) sess->fe;
	*p_result = result;
	len = write(fe->cfg->rfd, buf, 4);
	if(len <= 0)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			return;
		else
		{
			printf("==========%s============:reset\r\n", __func__);
			int fd[2];
			if (fe->cfg->rfd != PJ_INVALID_SOCKET)
				pj_sock_close(fe->cfg->rfd);
			if (fe->cfg->wfd != PJ_INVALID_SOCKET)
				pj_sock_close(fe->cfg->wfd);
			if(socketpair (AF_UNIX, fe->cfg->tcp ? SOCK_STREAM : SOCK_DGRAM, 0, fd) == 0)
			{
				fe->cfg->rfd = fd[0];
				fe->cfg->wfd = fd[1];
				cli_set_nonblocking(fe->cfg->rfd);
				cli_set_nonblocking(fe->cfg->wfd);
				return ;
			}
			return ;
		}
	}
}

static pj_bool_t cli_socket_exec(pj_cli_sess *sess)
{
	pj_status_t status;
	pj_bool_t retval = PJ_TRUE;

	pj_pool_t *pool;
	pj_cli_exec_info info;
	pj_cli_t *cli = sess->fe->cli;
	cli_socket_fe *fe = (cli_socket_fe *) sess->fe;
	char *recv_buf = fe->buf;
	pool = pj_pool_create(pj_cli_get_param(cli)->pf, "handle_exec",
	PJ_CLI_CONSOLE_POOL_SIZE, PJ_CLI_CONSOLE_POOL_INC, NULL);
	printf("==========%s============:%s\r\n", __func__, recv_buf);
	status = pj_cli_sess_exec(sess, recv_buf, pool, &info);

	switch (status)
	{
	case PJ_CLI_EINVARG:
	case PJ_CLI_ETOOMANYARGS:
	case PJ_CLI_EAMBIGUOUS:
	case PJ_CLI_EMISSINGARG:
		cli_socket_exec_result(sess, status);
		break;
	case PJ_CLI_EEXIT:
		retval = PJ_FALSE;
		break;
	case PJ_SUCCESS:
		cli_socket_exec_result(sess, PJ_SUCCESS);
		break;
	default:
		cli_socket_exec_result(sess, PJ_EUNKNOWN);
		break;
	}
	pj_pool_release(pool);
	return retval;
}

static pj_status_t cli_socket_start(cli_socket_fe *fe)
{
    pj_status_t status;
	int fd[2];

	if(socketpair (AF_UNIX, fe->cfg->tcp ? SOCK_STREAM : SOCK_DGRAM, 0, fd) == 0)
	{
		fe->cfg->rfd = fd[0];
		fe->cfg->wfd = fd[1];
		cli_set_nonblocking(fe->cfg->rfd);
		cli_set_nonblocking(fe->cfg->wfd);
	}
	else
		return -1;
    if (!fe->input_thread) {
        status = pj_thread_create(fe->pool, "worker_socket", &socket_worker_thread, fe,
                                  0, 0, &fe->input_thread);
        if (status != PJ_SUCCESS)
        {
            if (fe->cfg->rfd != PJ_INVALID_SOCKET)
                pj_sock_close(fe->cfg->rfd);
            if (fe->cfg->wfd != PJ_INVALID_SOCKET)
                pj_sock_close(fe->cfg->wfd);
            return status;
        }
    }

    return PJ_SUCCESS;

on_exit:

    if (fe->cfg->rfd != PJ_INVALID_SOCKET)
        pj_sock_close(fe->cfg->rfd);
    if (fe->cfg->wfd != PJ_INVALID_SOCKET)
        pj_sock_close(fe->cfg->wfd);
    if (fe->mutex)
        pj_mutex_destroy(fe->mutex);

    pj_pool_release(fe->pool);
    return status;
}

static int socket_worker_thread(void *p)
{
    pj_fd_set_t rfds;
    pj_time_val timeout;
	cli_socket_fe *fe = (cli_socket_fe *) p;
	pj_size_t input_len = 0;
	pj_str_t input_str;
	int n = 0;
	char *recv_buf = fe->buf;
	//pj_bool_t is_valid = PJ_TRUE;
    PJ_FD_ZERO(&rfds);
    PJ_FD_SET(fe->cfg->rfd, &rfds);
    timeout.sec = 1;
    timeout.msec = 0;
	while (!fe->thread_quit)
	{
		pj_mutex_lock (fe->mutex);
		if (fe->thread_quit)
		{
			pj_mutex_unlock (fe->mutex);
			fprintf(stdout, "==================%s=============thread_quit\r\n", __func__);
			break;
		}
		pj_mutex_unlock (fe->mutex);
	    n = pj_sock_select(fe->cfg->rfd + 1, &rfds, NULL, NULL, &timeout);
	    if (n < 0)
	    {
	    	fprintf(stdout, "==================%s=============pj_sock_select(%s)\r\n", __func__, strerror(errno));
	    	goto socket_thread_exit;
	    }
	    if (n == 0)
		{
			//fprintf(stdout, "==================%s=============\r\n", __func__);
	    	continue;
		}
	    if (!PJ_FD_ISSET(fe->cfg->rfd, &rfds))
	    {
	    	fprintf(stdout, "==================%s=============PJ_FD_ISSET(%s)\r\n", __func__, strerror(errno));
	    	goto socket_thread_exit;
	    }

		memset(recv_buf, 0, sizeof(fe->buf));
		fe->buf_len = read(fe->cfg->rfd, recv_buf, sizeof(fe->buf));
/*		if (fgets(recv_buf, fe->input.maxlen, stdin) == NULL)
		{
			if (1)
			{
				puts("Cannot switch back to console from file redirection");
			}
			else
			{
				puts("Switched back to console from file redirection");
				continue;
			}
		}*/
		if(fe->buf_len <= 0)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				continue;
			else
			{
				printf("==========%s============:reset\r\n", __func__);
				int fd[2];
				pj_mutex_lock (fe->mutex);
			    if (fe->cfg->rfd != PJ_INVALID_SOCKET)
			        pj_sock_close(fe->cfg->rfd);
			    if (fe->cfg->wfd != PJ_INVALID_SOCKET)
			        pj_sock_close(fe->cfg->wfd);

				if(socketpair (AF_UNIX, fe->cfg->tcp ? SOCK_STREAM : SOCK_DGRAM, 0, fd) == 0)
				{
					fe->cfg->rfd = fd[0];
					fe->cfg->wfd = fd[1];
					cli_set_nonblocking(fe->cfg->rfd);
					cli_set_nonblocking(fe->cfg->wfd);
					pj_mutex_unlock (fe->mutex);
					continue;
				}
				pj_mutex_unlock (fe->mutex);
			 	fprintf(stdout, "==================%s=============read(%s)\r\n", __func__, strerror(errno));
				goto socket_thread_exit;
			}
		}
		pj_mutex_lock (fe->mutex);
		input_str.ptr = recv_buf;
		input_str.slen = pj_ansi_strlen(recv_buf);
		pj_strrtrim(&input_str);
		recv_buf[input_str.slen] = '\n';
		recv_buf[input_str.slen + 1] = 0;

		printf("==========%s============:%s\r\n", __func__, recv_buf);

		if (fe->thread_quit)
		{
			fprintf(stdout, "==================%s=============thread_quit 1==\r\n", __func__);
			pj_mutex_unlock (fe->mutex);
			break;
		}
		input_len = pj_ansi_strlen(fe->buf);

		if ((input_len > 1) && (fe->buf[input_len - 2] == '?'))
		{
			fe->buf[input_len - 1] = 0;
			cli_socket_exec_result(fe->sess, PJ_CLI_EINVARG);
		}
		else
		{
			cli_socket_exec(fe->sess);
		}
		pj_mutex_unlock (fe->mutex);
	}
socket_thread_exit:
	fprintf(stdout, "==================%s=============socket_thread_exit\r\n", __func__);
    //pj_list_erase(fe->sess);
	return 0;
	if(fe->mutex)
		pj_mutex_lock (fe->mutex);
	if(fe->sess)
	{
		fe->sess = NULL;
	}
    if (fe->cfg->rfd != PJ_INVALID_SOCKET)
    {
        pj_sock_close(fe->cfg->rfd);
        fe->cfg->rfd = PJ_INVALID_SOCKET;
    }
    if (fe->cfg->wfd != PJ_INVALID_SOCKET)
    {
        pj_sock_close(fe->cfg->wfd);
        fe->cfg->wfd = PJ_INVALID_SOCKET;
    }
/*    if (fe->input_thread) {
        pj_thread_destroy(fe->input_thread);
        fe->input_thread = NULL;
    }*/
    if (fe->mutex)
    {
    	pj_mutex_unlock (fe->mutex);
        pj_mutex_destroy(fe->mutex);
        fe->mutex = NULL;
    }
    pj_pool_release(fe->pool);
	return 0;
}

pj_status_t cli_socket_destroy(void *ife)
{
	cli_socket_fe *fe = ife;
    pj_pool_t *pool = NULL;
	fe->thread_quit = PJ_TRUE;
	if (fe->input_thread)
	{
		pj_thread_join (fe->input_thread);
		pj_thread_destroy (fe->input_thread);
		fe->input_thread = NULL;
	}

	if (fe->base.op && fe->base.op->on_destroy)
	{
		(fe->base.op->on_destroy) (fe);
	}
	if (fe->mutex)
	{
		pj_mutex_destroy (fe->mutex);
		fe->mutex = NULL;
	}
	pool = fe->pool;
	fe->pool = NULL;
	pj_pool_release(pool);
	return PJ_SUCCESS;
}


PJ_DEF(void) pj_cli_socket_cfg_default(pj_cli_socket_cfg *param)
{
    pj_assert(param);
    pj_bzero(param, sizeof(*param));
}

PJ_DEF(pj_status_t) pj_cli_socket_create(pj_cli_t *cli,
					  const pj_cli_socket_cfg *param,
					  pj_cli_sess **p_sess,
					  pj_cli_front_end **p_fe)
{
    pj_cli_sess *sess;
    struct cli_socket_fe *fe;
    pj_pool_t *pool;
    //pj_status_t status;

    PJ_ASSERT_RETURN(cli && p_sess, PJ_EINVAL);
    if (!param)
    {
    	return PJ_ENOMEM;
    }
    pool = pj_pool_create(pj_cli_get_param(cli)->pf, "socket_fe",
                          PJ_CLI_CONSOLE_POOL_SIZE, PJ_CLI_CONSOLE_POOL_INC,
                          NULL);
    if (!pool)
        return PJ_ENOMEM;

    pj_cli_socket_cfg_default(param);

    sess = PJ_POOL_ZALLOC_T(pool, pj_cli_sess);
    fe = PJ_POOL_ZALLOC_T(pool, struct cli_socket_fe);

    sess->fe = &fe->base;
    //sess->log_level = param->log_level;
    sess->op = PJ_POOL_ZALLOC_T(pool, struct pj_cli_sess_op);
    fe->base.op = PJ_POOL_ZALLOC_T(pool, struct pj_cli_front_end_op);
    fe->base.cli = cli;
    fe->base.type = PJ_CLI_SOCKET_FRONT_END;
    //fe->base.op->on_write_log = &console_write_log;
    fe->base.op->on_quit = &cli_socket_quit;
    fe->base.op->on_destroy = &cli_socket_front_destroy;
    fe->pool = pool;
    fe->sess = sess;
    fe->cfg = param;

    if (pj_mutex_create_recursive(pool, "mutex_socket_fe", &fe->mutex) != PJ_SUCCESS)
    {
        pj_pool_release(fe->pool);
        return PJ_EINVAL;
    }
    if(cli_socket_start(fe) != PJ_SUCCESS)
    {
    	pj_mutex_destroy (fe->mutex);
        pj_pool_release(fe->pool);
        return PJ_EINVAL;
    }
/*
    status = pj_sem_create(pool, "console_fe", 0, 1, &fe->thread_sem);
    if (status != PJ_SUCCESS)
	return status;

    status = pj_sem_create(pool, "console_fe", 0, 1, &fe->input.sem);
    if (status != PJ_SUCCESS)
	return status;
*/
    pj_cli_register_front_end(cli, &fe->base);

    *p_sess = sess;
    if (p_fe)
        *p_fe = &fe->base;

    return PJ_SUCCESS;
}

PJ_DEF(pj_status_t) pj_cli_socket_get_wfd(pj_cli_front_end *fe)
{
	cli_socket_fe * cfe = (cli_socket_fe *)fe;
	if(cfe)
		return cfe->cfg->wfd;
    return 0;
}



