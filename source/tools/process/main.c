/*
 * main.c
 *
 *  Created on: Aug 25, 2018
 *      Author: zhurish
 */
#include "auto_include.h"
#include <zplos_include.h>


#include "process.h"
#include <sys/stat.h>

#include "getopt.h"

//#undef ZPL_TOOLS_PROCESS
#ifdef ZPL_TOOLS_PROCESS


static char *progname = NULL;
static int domain = 0;
extern char *logfile;
extern int debug;
static int signo_act = 0;
/* Help information display. */
static void
process_usage (char *prog_name, int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", prog_name);
  else
    {
      printf ("Usage : %s [OPTION...]\n\n"\
	      "-D, --daemon       	Runs in daemon mode\n"\
	      "-l, --log file  		Set log file name\n"\
	      "-d, --debug level    Set log level\n"\
	      "-h, --help         Display this help and exit\n"\
	      "\n", prog_name);
    }

  exit (status);
}

static int get_getopt(int argc, char **argv)
{
	const struct option longopts[] = {
        {"log-file",      required_argument,  NULL, 'l'},
        {"debug",       required_argument,  NULL, 'd'},
        {"daemon",         no_argument,        NULL, 'D'},
        {"help",        no_argument,        NULL, 'h'},
        {NULL,          0,                  NULL, 0}
	};
	while (1)
	{
	      int opt;
	      opt = getopt_long (argc, argv, "l:d:Dh", longopts, 0);

	      if (opt == EOF)
	    	  break;

	      switch (opt)
	      {
	      case 0:
	    	  break;
	      case 'D':
	    	  domain = 1;
	    	  break;

	      case 'l':
	    	  logfile = optarg;
	    	  break;

	      case 'd':
	    	  debug = atoi(optarg);
	    	  if(debug > 7 || debug <= 0)
	    	  {
	    		  printf(" error debug level <1-7>\r\n");
	    		  return 0;
	    	  }
	    	  break;
	      case 'h':
	    	  process_usage (progname, 0);
	    	  break;
	      default:
	    	  process_usage (progname, 1);
	    	  break;
		}
	}
	return OK;
}


static int process_manage_unit(int fd, process_head *head, int *errnum)
{

	int /*num = 0, */len = 0;
	zpl_int32 offset = 0;
	char buf[1024];
	os_memset(buf, 0, sizeof(buf));
	while(1)
	{
		offset = read(fd, buf + offset, sizeof(process_head) - offset);
		if(offset)
		{
			process_log_debug("recv process 1:%s(%dbyte)", buf, offset);
			len += offset;
			if(len == sizeof(process_head))
			{
				memcpy(head, buf, sizeof(process_head));
				process_log_debug("recv process 2: name=%s cmd %s %s", head->name, head->process,
							head->argvs);
				return OK;
			}
			else
				process_log_err("recv process 3: name=%s cmd %s %s(%d (%d)byte)", head->name, head->process,
							head->argvs, len, sizeof(process_head));
		}
		else if(offset < 0)
		{
			if(errnum)
				*errnum = -1;
			return ERROR;
		}
		else if(offset == 0)
		{
			if(errnum)
				*errnum = 0;
			return ERROR;
		}
	}
	return ERROR;
}

/* SIGHUP handler. */
static void os_sighup(void)
{
	signo_act = SIGHUP;
	fprintf(stdout, "%s\r\n",__func__);
	process_exit();
	exit(0);
}

/* SIGINT handler. */
static void os_sigint(void)
{
	signo_act = SIGINT;
/*	process_exit();
	exit(0);*/
}

/* SIGKILL handler. */
static void os_sigkill(void)
{
	signo_act = SIGKILL;
/*	fprintf(stdout, "%s\r\n",__func__);
	process_exit();
	exit(0);*/
}

/* SIGUSR1 handler. */
static void os_sigusr1(void)
{
	signo_act = SIGUSR1;
	//fprintf(stdout, "%s\r\n",__func__);
}

/*static void os_sigchld(void)
{
	signo_act = SIGCHLD;
	waitpid(-1, NULL, 0);
	process_log_debug( "%s\r\n",__func__);
}*/

static int os_sig_handle(int n)
{
	int exit_flag = 0;
	int pid = 0;
	switch(n)
	{
	case SIGHUP:
		fprintf(stdout, "%s: SIGHUP\r\n",__func__);
		process_exit();
		exit_flag = 1;
		break;
	case SIGINT:
		fprintf(stdout, "%s: SIGINT\r\n",__func__);
		process_exit();
		exit_flag = 1;
		break;
	case SIGKILL:
		fprintf(stdout, "%s: SIGKILL\r\n",__func__);
		process_exit();
		exit_flag = 1;
		break;
	case SIGUSR1:
		fprintf(stdout, "%s: SIGUSR1\r\n",__func__);
		break;
	case SIGCHLD:
		fprintf(stdout, "%s: SIGCHLD\r\n",__func__);
		pid = waitpid(-1, NULL, 0);
		exit_flag = pid;
		break;
	default:
		break;
	}
	return exit_flag;
}

int main(int argc, char *argv[])
{
	int fd[6] = { 0 }, sock = 0, i = 0, tmp = 0, count = 0;
	char *p = NULL;
	int errnum = 0, maxfd = 0, num = 0, ret = 0;
	fd_set rfdset;
	process_head head;
	//process_t * process = NULL;

	progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);

	get_getopt(argc, argv);

	/* Daemonize. */
	if (domain && daemon(0, 0) < 0) {
		printf("Process daemon failed: %s", strerror(ipstack_errno));
		exit(1);
	}
	os_pid_set(DAEMON_VTY_DIR"/process.pid");
	os_register_signal(SIGHUP, os_sighup);
	os_register_signal(SIGINT, os_sigint);
	os_register_signal(SIGKILL, os_sigkill);
	os_register_signal(SIGUSR1, os_sigusr1);
	//os_register_signal(SIGCHLD, os_sigchld);
	open_log(logfile ? logfile:"process.log");
	debug = 7;
	if(sock == 0)
	{
		sock = os_sock_unix_server_create(zpl_true, PROCESS_MGT_UNIT_NAME);
	}
	if(sock <= 0)
		return ERROR;
	process_init();
	process_log_debug("start running %s(sock=%d)",progname, sock);
	os_set_nonblocking(sock);
	FD_ZERO(&rfdset);
	FD_SET(sock, &rfdset);
	maxfd = sock;

	while(1)
	{
		FD_SET(sock, &rfdset);
		for(i = 0; i < 6; i++)
		{
			if(fd[i])
			{
				maxfd = MAX(fd[i], maxfd);
				FD_SET(fd[i], &rfdset);
			}
		}
		ret = os_sig_handle(signo_act);
		signo_act = 0;
		if(ret == 1)
		{
			for(i = 0; i < 6; i++)
			{
				if(fd[i])
				{
					close(fd[i]);
				}
			}
			close(sock);
			exit(0);
		}
		else if(ret > 1)
		{
			process_t * process = process_lookup_pid_api(ret);
			if(process)
				process_del_api(process);
		}
		num = os_select_wait(maxfd + 1, &rfdset, NULL, 0);
		//num = select(maxfd + 1, &rfdset, NULL, NULL, NULL);
		//process_log_debug("start os_select_wait(%d) %s", num, strerror(ipstack_errno));
		if(num > 0)
		{
			//process_log_debug("start os_sock_unix_accept %s", progname);
			if(FD_ISSET(sock, &rfdset))
			{
				FD_CLR(sock, &rfdset);
				tmp = os_sock_unix_accept(sock, NULL);
				if(tmp)
				{
					if(count == 6)
					{
						close(tmp);
						process_log_debug("too many client %s(max client=%d)", progname, count);
						continue;
					}
					process_log_debug("start os_sock_unix_accept %s(%d)", progname, fd);
					FD_SET(tmp, &rfdset);
					maxfd = MAX(tmp, maxfd);
					os_set_nonblocking(tmp);
					for(i = 0; i < 6; i++)
					{
						if(fd[i] == 0)
						{
							fd[i] = tmp;
							count++;
							break;
						}
					}
					continue;
				}
				else
				{
					if (ipstack_errno == EPIPE || ipstack_errno == EBADF || ipstack_errno == EIO || ipstack_errno == ECONNRESET
								|| ipstack_errno == ECONNABORTED || ipstack_errno == ENETRESET || ipstack_errno == ECONNREFUSED)
					{
						continue;
					}
				}
			}
			else
			{
				for(i = 0; i < 6; i++)
				{
					if(fd[i])
					{
						if(fd[i] && FD_ISSET(fd[i], &rfdset))
						{
							FD_CLR(fd[i], &rfdset);
							os_memset(&head, 0, sizeof(process_head));
							ret = process_manage_unit(fd[i], &head, &errnum);
							if((ret == OK) && fd[i])
							{
								process_handle( fd[i], head.action, &head);
							}
							else
							{
								if(errnum == -1)
								{
									if (ipstack_errno == EPIPE || ipstack_errno == EBADF || ipstack_errno == EIO || ipstack_errno == ECONNRESET
												|| ipstack_errno == ECONNABORTED || ipstack_errno == ENETRESET || ipstack_errno == ECONNREFUSED)
									{
										process_log_err("%s close and reopen pipe %s", progname, PROCESS_MGT_UNIT_NAME);
										if(fd[i])
										{
											FD_CLR(fd[i], &rfdset);
											close(fd[i]);
											count--;
											fd[i] = 0;
											tmp = 0;
										}
									}
								}
								if(errnum == 0)
								{
									if(fd[i])
									{
										FD_CLR(fd[i], &rfdset);
										close(fd[i]);
										count--;
										fd[i] = 0;
										tmp = 0;
									}
								}
								else
									process_log_debug("%s",__func__);
							}
						}
					}
				}
			}
			continue;
		}
		else if(num < 0)
		{
/*			if (ipstack_errno == EPIPE || ipstack_errno == EBADF || ipstack_errno == EIO || ipstack_errno == ECONNRESET
					|| ipstack_errno == ECONNABORTED || ipstack_errno == ENETRESET || ipstack_errno == ECONNREFUSED)
			{
				if(fd)
				{
					FD_CLR(fd, &rfdset);
					close(fd);
					fd = 0;
				}
			}*/
			continue;
		}
/*		for(i = 0; i < 6; i++)
		{
			if(fd[i])
			{
				if(tcp_sock_state(fd[i]) != TCP_ESTABLISHED)
				{
					FD_CLR(fd[i], &rfdset);
					close(fd[i]);
					fd[i] = 0;
					count--;
				}
			}
		}*/
		process_waitpid_api();
	}
}

#else

int main()
{
	printf("hello");
}
#endif
