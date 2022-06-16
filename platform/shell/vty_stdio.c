/*
 * Virtual terminal [aka TeletYpe] interface routine.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "linklist.h"
#include "zmemory.h"


#ifdef ZPL_SHELL_MODULE
#include "cli_node.h"
#include "buffer.h"
#include "vector.h"
#include "command.h"
#endif
#include "log.h"
#include "str.h"
#include "prefix.h"
#include "host.h"
#include "network.h"
#include "sockunion.h"
#include "sockopt.h"

#include "eloop.h"
#include "thread.h"
#ifdef ZPL_IP_FILTER
#include "filter.h"	
#endif
#include "vty.h"
#include "vty_user.h"

#include <arpa/telnet.h>
#include <termios.h>
#include "sys/wait.h"

#pragma GCC diagnostic ignored "-Werror=strict-prototypes"
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#pragma GCC diagnostic ignored "-Werror=redundant-decls"
#pragma GCC diagnostic ignored "-Wredundant-decls"

#ifdef ZPL_SHRL_MODULE
#include <readline/readline.h>
#include <readline/history.h>

#if 0
readline: ./configure --prefix=/home/zhurish/workspace/working/zpl-source/source/externsions/readline-8.1/_install --enable-shared --disable-install-examples --host=arm-gnueabihf-linux CC="/opt/gcc-linaro-7.5.0-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc"
 make all
 make install
Readline 基本操作
输入读取
很多命令行交互式程序交互方式都差不多，输出提示符，等待用户输入命令，用户输入命令之后按回车，程序开始解析命令并执行。那么这里面有个动作是读入用户的输入，以前我们也许使用 gets() 这样的函数来实现，当我们使用 Readline 库时，可以使用 readline() 函数来替换它，该函数在 ANSI C 中定义如下：

char *readline (char *prompt);
该函数带有一个参数 prompt，表示命令提示符，例如 ftp 中就是 "ftp>"，用户在后面可以输入命令，当按下回车键时，程序读入该行（不包括最后的换行符）存入字符缓冲区中，readline 的返回值就是该行文本的指针。注意：当该行文本不需要使用时，需要释放该指针指向的空间，防止内存泄漏。当读入 EOF时，如果还未读入其它字符，则返回 (char *) NULL，否则读入结束，与读入换行效果相同。

命令补全
除了能读入用户的输入，我们有时希望交互更简单些，例如命令补全。当有很多命令时，如果希望用户都能准确记忆命令的拼写是困难的，那么一般做法是按下 TAB 键进行命令提示及补全，如 ftp 下输入一个字符c 之后按下 TAB 键，会列出所有以 c 开头的命令：

ftp> c
case cd cdup chmod   close   cr
readline 函数其实已经给用户默认的 TAB 补全的功能：根据当前路径下文件名来补全。

如果你不想 Readline 根据文件名补全，你可以通过 rl_bind_key() 函数来改变 TAB 键的行为。该函数的原型为：

int rl_bind_key(int key, int (*function)());
该函数带有两个参数：key 是你想绑定键的 ASCII 码字符表示，function 是当 key 键按下时触发调用函数的地址。如果想按下 TAB 键就输入一个制表符本身，可以将 TAB 绑定到 rl_insert() 函数，这是 Readline 库提供的函数。如果 key 不是有效的 ASCII 码值（0~255之间），rl_bind_key() 返回非 0。

这样，禁止 TAB 的默认行为，下面这样做就可以了：

rl_bind_key('\t', rl_insert);
这个代码需要在你程序一开始就调用；你可以写一个函数叫 initialize_readline() 来执行这个动作和其它一些必要的初始化，例如安装用户自定义补全。

当我们希望输入 TAB 时不是列出当前路径下的所有文件，而是列出程序内置的一些命令，例如上面举到 ftp 的例子，这种行为称为自定义补全。 该操作较复杂，我们留在后面一节主要介绍。

历史记录
基本操作还有一个——搜索历史。我们希望输入过的命令行，还可以通过 C-p 或者 C-s 来搜索到，那么就需要将命令行加入到历史列表中，可以调用 add_history() 函数来完成。但尽量将空行也加入到历史列表中，因为空行占用历史列表的空间而且也毫无用处。综上，我们可以写出一个 Readline 版的 gets() 函数 rl_gets()：

/* A static variable for holding the line. */
static char *line_read = (char *)NULL;

/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
char *
rl_gets ()
{
  /* If the buffer has already been allocated, return the memory
     to the free pool. */
  if (line_read)
    {
      free (line_read);
      line_read = (char *)NULL;
    }

  /* Get a line from the user. */
  line_read = readline ("");

  /* If the line has any text in it, save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);

  return (line_read);
}
自定义补全
上面也提到了什么是自定义补全，无疑这在命令行交互式程序中是非常重要的，直接影响到用户体验。Readline 库提供了两种比较常用的补全方式——按照文件名补全和按照用户名补全，分别对应 Readline 中已经实现的两个函数 rl_filename_completion_function 和 rl_username_completion_function。如果我们既不希望按照文件名和用户名来补全，希望按照程序的命令补全，应该怎么做呢？也很容易想到，只要实现自己的补全函数就好了。

Readline 补全的工作原理如下：

用户接口函数 rl_complete() 调用 rl_completion_matches() 来产生可能的补全列表；
内部函数 rl_completion_matches() 使用程序提供的 generator 函数来产生补全列表，并返回这些匹配的数组，在此之前需要将 generator 函数的地址放到 rl_completion_entry_function 变量中，例如上面提到的按文件名或用户名补全函数就是不同的 generators；
generator 函数在 rl_completion_matches() 中不断被调用，每次返回一个字符串。generator 函数带有两个参数：text 是需要补全的单词的部分，state 在函数第一次调用时为 0，接下来调用时非 0。generator 函数返回 (char *)NULL 通知 rl_completion_matches() 没有剩下可能的匹配。
Readline 库中有个变量 rl_attempted_completion_function，改变量类型是一个函数指针rl_completion_func_t *，我们可以将该变量设置我们自定义的产生匹配的函数，该按下 TAB 键时会调用该函数，函数具有三个参数：

text: 该参数是待补全的单词的部分，例如在 Bash 提示符后输入一个 c 字符，按下 TAB，此时 text指向的是 "c" 字符串的指针；在 Bash 提示符后输入一个 cd /home/gu 字符串，按下 TAB，此时 text指向的是"/home/gu" 字符串的指针；
start: text 字符串在该行输入中的起始位置，例如对于上面的例子，第一种情况下是 0，第二种情况下是 3；
end: text 字符串在该行输入中的结束位置，例如对于上面的例子，第一种情况下是 1，第二种情况下是 11。
我们自定义的补全函数可以根据传入的参数来设置我们希望按照什么方式补全，例如对于 Bash 下的 cd命令，我们希望开始是命令补全，当命令补全之后，后面接着跟的是文件名补全，这样可以使用rl_completion_matches() 来绑定使用哪种 generator，rl_completion_matches() 函数的原型是：

char ** rl_completion_matches (const char *text, rl_compentry_func_t *entry_func)
带有两个参数：text 就是上面介绍的传入的待补全的单词，第二个参数 entry_func 是上面反复介绍的generator 函数的指针。该函数的返回值是 generator 产生的可能匹配 text 的字符串数组指针，该数组的最后一项是 NULL 指针。

好了，上面说了这么多关于自定义补全的函数和变量，到底怎么用呢，估计还是比较模糊，那么看一个例子估计就很清楚了，这个例子是 Readline 官方提供的示例程序，由于比较长，就不在这里贴出来了，你可以在 http://cnswww.cns.cwru.edu/ph... 找到。

#endif



struct vtyrl_stdio_t
{
	zpl_taskid_t rl_taskid;
	int rl_exit;
	struct vty *vty;
	char *line_read;
	int complete_status;
	int init;
	zpl_uint32 timer;

	int status;

	int sigwinch_received;
};

struct vtyrl_stdio_t  vtyrl_stdio = {.init = 0, };

static int	vtyrl_stdio_vty_output(void *p, const char *data, int len)
{
	if(p)
	{
		//fputs(data, p);
		int ret = fwrite(data, len, 1, p);
		fflush(p);
		return ret;
	}
	return 0;
}

static int vtyrl_stdio_describe(void)
{
	int ret;
	unsigned int i;
	vector vline;
	vector describe;
	int width;
	struct cmd_token *token;

	vline = cmd_make_strvec(rl_line_buffer);

	/* In case of '> ?'. */
	if (vline == NULL)
	{
		vline = vector_init(1);
		vector_set(vline, NULL);
	}
	else if (rl_end && isspace((int)rl_line_buffer[rl_end - 1]))
		vector_set(vline, NULL);

	describe = cmd_describe_command(vline, vtyrl_stdio.vty, &ret);

	fprintf(stdout, "\n");

	/* Ambiguous and no match error. */
	switch (ret)
	{
	case CMD_ERR_AMBIGUOUS:
		cmd_free_strvec(vline);
		fprintf(stdout, "%% Ambiguous command.\n");
		rl_on_new_line();
		return 0;
		break;
	case CMD_ERR_NO_MATCH:
		cmd_free_strvec(vline);
		fprintf(stdout, "%% There is no matched command.\n");
		rl_on_new_line();
		return 0;
		break;
	}

	/* Get width of command string. */
	width = 0;
	for (i = 0; i < vector_active(describe); i++)
	{
		if ((token = vector_slot(describe, i)) != NULL)
		{
			int len;

			if (token->cmd[0] == '\0')
				continue;

			len = strlen(token->cmd);
			if (token->cmd[0] == '.')
				len--;

			if (width < len)
				width = len;
		}
	}
	for (i = 0; i < vector_active(describe); i++)
	{
		if ((token = vector_slot(describe, i)) != NULL)
		{
			if (token->cmd[0] == '\0')
				continue;

			if (!token->desc)
				fprintf(stdout, "  %-s\n",
						token->cmd[0] == '.' ? token->cmd + 1 : token->cmd);
			else
				fprintf(stdout, "  %-*s  %s\n",
						width,
						token->cmd[0] == '.' ? token->cmd + 1 : token->cmd,
						token->desc);
		}
	}

	cmd_free_strvec(vline);
	vector_free(describe);
	fflush(stdout);
	rl_on_new_line();
	return 0;
}


/* To disable readline's filename completion. */
static char *vtyrl_stdio_completion_entry_function(const char *ignore, int invoking_key)
{
	return NULL;
}

static char * vtyrl_stdio_command_generator(const char *text, int state)
{
	vector vline;
	static char **matched = NULL;
	static int index = 0;

	/* First call. */
	if (!state)
	{
		index = 0;

		if (vtyrl_stdio.vty->node == AUTH_NODE || vtyrl_stdio.vty->node == AUTH_ENABLE_NODE)
			return NULL;

		vline = cmd_make_strvec(rl_line_buffer);
		if (vline == NULL)
			return NULL;

		if (rl_end && isspace((int)rl_line_buffer[rl_end - 1]))
			vector_set(vline, NULL);

		matched = cmd_complete_command(vline, vtyrl_stdio.vty, &vtyrl_stdio.complete_status);
	}

	if (matched && matched[index])
		return matched[index++];

	return NULL;
}

static char **vtyrl_stdio_new_completion(char *text, int start, int end)
{
	char **matches;

	matches = rl_completion_matches(text, vtyrl_stdio_command_generator);

	if (matches)
	{
		rl_point = rl_end;
		if (vtyrl_stdio.complete_status != CMD_COMPLETE_FULL_MATCH)
			/* only append a space on full match */
			rl_completion_append_character = '\0';
	}

	return matches;
}

/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
static char *vtyrl_stdio_gets(void)
{
	HIST_ENTRY *last;
	/* If the buffer has already been allocated, return the memory
	 * to the free pool. */
	if (vtyrl_stdio.line_read)
	{
		free(vtyrl_stdio.line_read);
		vtyrl_stdio.line_read = NULL;
	}

	/* Get a line from the user.  Change prompt according to node.  XXX. */
	vtyrl_stdio.line_read = readline(vty_prompt(vtyrl_stdio.vty));

	/* If the line has any text in it, save it on the history. But only if
	 * last command in history isn't the same one. */
	if (vtyrl_stdio.line_read && *vtyrl_stdio.line_read)
	{
		using_history();
		last = previous_history();
		if (!last || strcmp(last->line, vtyrl_stdio.line_read) != 0)
		{
			add_history(vtyrl_stdio.line_read);
		}
	}
	if (vtyrl_stdio.rl_exit == 0)
	{
		free(vtyrl_stdio.line_read);
		vtyrl_stdio.line_read = NULL;
	}
	return (vtyrl_stdio.line_read);
}



static int vtyrl_stdio_timeout(void *p)
{
	struct vty *vty = vtyrl_stdio.vty;
	rl_clear_message();
	rl_redisplay();
	vty_sync_out(vty, "%s%sVty connection is timed out.%s%s",
				 VTY_NEWLINE, VTY_NEWLINE,
				 VTY_NEWLINE, VTY_NEWLINE);
	return 0;			 
}

static int vtyrl_stdio_task(void *p)
{
	zlog_warn(MODULE_LIB, "==== vtyrl_stdio_task 1");
	while(vtyrl_stdio.status == 0)
	{
		os_sleep(1);
	} 
	zlog_warn(MODULE_LIB, "====  vtyrl_stdio_task 2");
	while (vtyrl_stdio.rl_exit)
	{
		vty_hello(vtyrl_stdio.vty);
		while (vtyrl_stdio_gets())
		{
			vty_command(vtyrl_stdio.vty, vtyrl_stdio.line_read);
			if(vtyrl_stdio.timer)
			{
				host_config_get_api(API_GET_VTY_TIMEOUT_CMD, &vtyrl_stdio.vty->v_timeout);
				os_time_restart(vtyrl_stdio.timer, vtyrl_stdio.vty->v_timeout);
			}	
			else
			{
				host_config_get_api(API_GET_VTY_TIMEOUT_CMD, &vtyrl_stdio.vty->v_timeout);
				vtyrl_stdio.timer = os_time_create_once(vtyrl_stdio_timeout, &vtyrl_stdio, vtyrl_stdio.vty->v_timeout);

			}		
		}
		break;
	}
	vtyrl_stdio.rl_taskid = 0;
	return 0;
}

static int vty_readline_attribute(void)
{
	struct termios old_termios;
	if (!tcgetattr(fileno (rl_instream), &old_termios))
	{
		old_termios.c_lflag &= ~(ISIG);//忽略终端输入的CTRL+C等信号
		tcsetattr(fileno (rl_instream), TCSANOW, &old_termios);
	}
	return 0;
}
static void sigwinch_handler (int sig)
{
  vtyrl_stdio.sigwinch_received = 1;
}


static int vty_readline_init(void)
{
	setlinebuf(stdout);
	/* Initialize readline. */
	
	rl_initialize();

	/* readline related settings. */
	rl_bind_key('?', (rl_command_func_t *)vtyrl_stdio_describe);
	//rl_bind_key(3, (rl_command_func_t *)vtyrl_stdio_describe);
	/* 初始化tab补全 */
	rl_completion_entry_function = vtyrl_stdio_completion_entry_function;
	rl_attempted_completion_function = (rl_completion_func_t *)vtyrl_stdio_new_completion;
	vty_readline_attribute();
	signal (SIGWINCH, sigwinch_handler);
	zlog_warn(MODULE_LIB, "==== vty_readline_init");
	return 0;
}

int vtyrl_stdio_init(void)
{
	zpl_socket_t vty_sock = {OS_STACK, 0};
	if(vtyrl_stdio.init == 0)
	{
		memset(&vtyrl_stdio, 0, sizeof(vtyrl_stdio));
		vtyrl_stdio.vty = vty_new_init(vty_sock);
		vtyrl_stdio.vty->node = ENABLE_NODE;
		vtyrl_stdio.vty->privilege = CMD_CONFIG_LEVEL;
		vtyrl_stdio.vty->login_type = VTY_LOGIN_VTYSH_STDIO;
		vtyrl_stdio.vty->vty_output = vtyrl_stdio_vty_output;
		vtyrl_stdio.vty->p_output = stdout;
		zlog_warn(MODULE_LIB, "==== vtyrl_stdio_init");
		vtyrl_stdio.init = 1;
		return vty_readline_init();
	}
	return OK;
}


int vtyrl_stdio_exit(void)
{
	if(vtyrl_stdio.init)
	{
		if(vtyrl_stdio.timer)
		{
			os_time_destroy(vtyrl_stdio.timer);
			vtyrl_stdio.timer = 0;
		}
		rl_initialize();
		if(vtyrl_stdio.vty)
		{
			vty_free(vtyrl_stdio.vty);
			vtyrl_stdio.vty = NULL;
		}
	}
	memset(&vtyrl_stdio, 0, sizeof(vtyrl_stdio));
	return 0;
}



int vtyrl_stdio_task_init(void)
{
	if(vtyrl_stdio.init)
	{
		vtyrl_stdio.rl_exit = 1;
		zlog_warn(MODULE_LIB, "==== vtyrl_stdio_task_init");
		if (vtyrl_stdio.rl_taskid == 0)
			vtyrl_stdio.rl_taskid = os_task_create("stdioTask", OS_TASK_DEFAULT_PRIORITY,
														0, vtyrl_stdio_task, &vtyrl_stdio, OS_TASK_DEFAULT_STACK);
	}
	return OK;
}


int vtyrl_stdio_task_exit(void)
{
	if(vtyrl_stdio.init)
	{
		vtyrl_stdio.rl_exit = 0;
		zlog_warn(MODULE_LIB, "==== vtyrl_stdio_task_exit");
		vtyrl_stdio.rl_taskid = 0;
	}
	return OK;
}

int vtyrl_stdio_start(zpl_bool s)
{
	if(vtyrl_stdio.init)
	{
		vtyrl_stdio.status = s;
		zlog_warn(MODULE_LIB, "==== vtyrl_stdio_start");
		vtyrl_stdio_task_init();
	}
	return OK;
}


#if 0

extern int errno;

static void cb_linehandler (char *);
static void signandler (int);

int running, sigwinch_received;
const char *prompt = "rltest$ ";

/* Handle SIGWINCH and window size changes when readline is not active and
   reading a character. */
static void
sighandler (int sig)
{
  sigwinch_received = 1;
}

/* Callback function called for each line when accept-line executed, EOF
   seen, or EOF character read.  This sets a flag and returns; it could
   also call exit(3). */
static void
cb_linehandler (char *line)
{
  /* Can use ^D (stty eof) or `exit' to exit. */
  if (line == NULL || strcmp (line, "exit") == 0)
    {
      if (line == 0)
        printf ("\n");
      printf ("exit\n");
      /* This function needs to be called to reset the terminal settings,
	 and calling it from the line handler keeps one extra prompt from
	 being displayed. */
      rl_callback_handler_remove ();

      running = 0;
    }
  else
    {
      if (*line)
	add_history (line);
      printf ("input line: %s\n", line);
      free (line);
    }
}

int
main (int c, char **v)
{
  fd_set fds;
  int r;


  setlocale (LC_ALL, "");

  /* Handle SIGWINCH */
  signal (SIGWINCH, sighandler);
  
  /* Install the line handler. */
  rl_callback_handler_install (prompt, cb_linehandler);

  /* Enter a simple event loop.  This waits until something is available
     to read on readline's input stream (defaults to standard input) and
     calls the builtin character read callback to read it.  It does not
     have to modify the user's terminal settings. */
  running = 1;
  while (running)
    {
      FD_ZERO (&fds);
      FD_SET (fileno (rl_instream), &fds);    

      r = select (FD_SETSIZE, &fds, NULL, NULL, NULL);
      if (r < 0 && errno != EINTR)
	{
	  perror ("rltest: select");
	  rl_callback_handler_remove ();
	  break;
	}
      if (sigwinch_received)
	{
	  rl_resize_terminal ();
	  sigwinch_received = 0;
	}
      if (r < 0)
	continue;

      if (FD_ISSET (fileno (rl_instream), &fds))
	rl_callback_read_char ();
    }

  printf ("rltest: Event loop has exited\n");
  return 0;
}
#endif

#endif /*ZPL_SHRL_MODULE*/

//#define VTYSH_TEST

#ifdef VTYSH_TEST

#define PING_TOKEN "PING"


struct daemon
{
  const char *name;
  zpl_socket_t fd;
  os_ansync_t *t_connect;
  os_ansync_t *t_read;
  os_ansync_t *t_write;
  os_ansync_t *t_timeout;
  zpl_uint32 total_len;
  zpl_uint32 already;
  zpl_uint8 buf[256];
  zpl_uint8 *payload;
  zpl_uint32 count;
};

static os_ansync_lst *master;

#define SET_READ_HANDLER(DMN) \
  if((DMN)->t_read == NULL) \
  (DMN)->t_read = os_ansync_add(master, OS_ANSYNC_INPUT, handle_read, (DMN), (DMN)->fd._fd)

#define SET_WAKEUP_ECHO(DMN)                                                             \
  (DMN)->t_write = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, wakeup_send_echo, (DMN), \
                                  2000)

static int try_connect(struct daemon *dmn);
static int wakeup_send_echo(os_ansync_t *t_write);

static int wakeup_init(os_ansync_t *t_connect)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_connect);

  dmn->t_connect = NULL;
  if (try_connect(dmn) < 0)
  {
    fdprintf(STDOUT_FILENO, "%s state -> down : initial connection attempt failed\r\n",
             dmn->name);
  }
  return 0;
}

static void
daemon_down(struct daemon *dmn, const char *why)
{
  if(master && dmn->t_read)
    os_ansync_cancel(master, dmn->t_read);
  if(master && dmn->t_write)
    os_ansync_cancel(master, dmn->t_write);

  if(master && dmn->t_timeout)
    os_ansync_cancel(master, dmn->t_timeout);

	fdprintf(STDOUT_FILENO, "%s state -> down : %s\r\n", dmn->name, why);

  if (!ipstack_invalid(dmn->fd))
  {
    ipstack_close(dmn->fd);
  } 
}

static int
handle_read(os_ansync_t *t_read)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_read);
  const char resp[] = PING_TOKEN;
  vtysh_result_t *vtysh_result = (vtysh_result_t *)dmn->buf;
  ssize_t rc = 0, rlen = 0;
  struct timeval delay;
  dmn->payload = dmn->buf + sizeof(vtysh_result_t);

  if (dmn->already < sizeof(vtysh_result_t))
  {
    if ((rc = ipstack_read(dmn->fd, dmn->buf + dmn->already, sizeof(vtysh_result_t) - dmn->already)) < 0)
    {
      if (IPSTACK_ERRNO_RETRY(errno))
      {
        return 0;
      }
      dmn->total_len = dmn->already = 0;
      daemon_down(dmn, "read error");
      return 0;
    }
    if (rc != (ssize_t)(sizeof(vtysh_result_t) - dmn->already))
    {
      dmn->already += rc;
      return 0;
    }
    dmn->already = sizeof(vtysh_result_t);
  }
  if (dmn->total_len == 0)
    dmn->total_len = ntohl(vtysh_result->retlen) + sizeof(vtysh_result_t);
  if (dmn->total_len > sizeof(dmn->buf))
  {
    char why[200];
    dmn->total_len = dmn->already = 0;
    snprintf(why, sizeof(why), "is too long\r\n");
    daemon_down(dmn, why);
    return 0;
  }
  /* Read rest of data. */
  if (dmn->already < dmn->total_len)
  {
    if ((rc = ipstack_read(dmn->fd, dmn->buf + dmn->already, dmn->total_len - dmn->already)) < 0)
    {
      if (IPSTACK_ERRNO_RETRY(errno))
      {
        return 0;
      }
      dmn->total_len = dmn->already = 0;
      daemon_down(dmn, "read error");
      return 0;
    }
    if (rc != (ssize_t)(dmn->total_len - dmn->already))
    {
      dmn->already += rc;
      return 0;
    }
    dmn->already = dmn->total_len;
  }
  fdprintf(STDOUT_FILENO, "==%d==echo response received type = %d retcode = %d retlen = %d msg(%s)\r\n", os_time(NULL),
           ntohl(vtysh_result->type), ntohl(vtysh_result->retcode), ntohl(vtysh_result->retlen), dmn->payload);

	dmn->count++;
	if(dmn->count == 50)
	{
		exit(0);
	}
  /* We are expecting an echo response: is there any chance that the
     response would not be returned entirely in the first read?  That
     seems inconceivable... */
  if (memcmp(dmn->payload, resp, sizeof(resp) - 1))
  {
    char why[200];
    snprintf(why, sizeof(why), "read returned bad echo response of %d bytes : %.*s\r\n",
             (int)dmn->total_len, dmn->payload);
    daemon_down(dmn, why);
    return 0;
  }

  if (dmn->t_timeout)
    os_ansync_cancel(master, dmn->t_timeout);
	dmn->total_len = dmn->already = 0;
  return 0;
}

static void
daemon_up(struct daemon *dmn, const char *why)
{
  fdprintf(STDOUT_FILENO, "%s state -> up : %s\r\n", dmn->name, why);
  SET_WAKEUP_ECHO(dmn);
}

static int
check_connect(os_ansync_t *t_write)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_write);
  int sockerr;
  socklen_t reslen = sizeof(sockerr);

  dmn->t_write = NULL;
  if (ipstack_getsockopt(dmn->fd, IPSTACK_SOL_SOCKET, IPSTACK_SO_ERROR, (char *)&sockerr, &reslen) < 0)
  {
    fdprintf(STDOUT_FILENO, "%s: check_connect: getsockopt failed: %s\r\n",
             dmn->name, ipstack_strerror(errno));
    daemon_down(dmn, "getsockopt failed checking connection success");
    return 0;
  }
  if ((reslen == sizeof(sockerr)) && sockerr)
  {
    char why[100];
    snprintf(why, sizeof(why),
             "getsockopt reports that connection attempt failed: %s\r\n",
             ipstack_strerror(sockerr));
    daemon_down(dmn, why);
    return 0;
  }
  if (dmn->t_timeout)
    os_ansync_cancel(master, dmn->t_timeout);  
  if (dmn->t_write)
    os_ansync_cancel(master, dmn->t_write); 	
  SET_READ_HANDLER(dmn);
  daemon_up(dmn, "delayed connect succeeded\r\n");
  return 0;
}

static int
wakeup_connect_hanging(os_ansync_t *t_timeout)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_timeout);
  char why[100];

  dmn->t_timeout = NULL;
  snprintf(why, sizeof(why), "connection attempt timed out after %ld seconds\r\n",
           10);
  daemon_down(dmn, why);
  return 0;
}

/* Making connection to protocol daemon. */
static int
try_connect(struct daemon *dmn)
{
  zpl_socket_t sock;
  struct ipstack_sockaddr_un addr;
  socklen_t len;


  memset(&addr, 0, sizeof(struct ipstack_sockaddr_un));
  addr.sun_family = AF_UNIX;
  /*snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s.vty",
           gs.vtydir, dmn->name);
           */
  snprintf(addr.sun_path, sizeof(addr.sun_path), "%s",
           ZEBRA_VTYSH_PATH);
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
  len = addr.sun_len = SUN_LEN(&addr);
#else
  len = sizeof(addr.sun_family) + strlen(addr.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

  /* Quick check to see if we might succeed before we go to the trouble
     of creating a socket. */
  if (access(addr.sun_path, W_OK) < 0)
  {
    if (errno != ENOENT)
      fdprintf(STDOUT_FILENO, "%s: access to socket %s denied: %s\r\n",
               dmn->name, addr.sun_path, ipstack_strerror(errno));
    return -1;
  }
  sock = ipstack_socket(OS_STACK, IPSTACK_AF_UNIX, IPSTACK_SOCK_STREAM, 0);
  if (ipstack_invalid(sock))
  {
    fdprintf(STDOUT_FILENO, "%s(%s): cannot make socket: %s\r\n",
             __func__, addr.sun_path, ipstack_strerror(errno));
    return -1;
  }

  if (ipstack_set_nonblocking(sock) < 0)
  {
    fdprintf(STDOUT_FILENO, "%s(%s): set_nonblocking(%d) failed\r\n",
             __func__, addr.sun_path, sock);
    ipstack_close(sock);
    return -1;
  }

  if (ipstack_connect(sock, (struct sockaddr *)&addr, len) < 0)
  {
    if ((errno != EINPROGRESS) && (errno != EWOULDBLOCK))
    {
      ipstack_close(sock);
      return -1;
    }

    dmn->fd = sock;
    dmn->t_write = os_ansync_add(master, OS_ANSYNC_OUTPUT, check_connect, dmn, dmn->fd._fd);
    dmn->t_timeout = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, wakeup_connect_hanging, dmn,
                                  15000);
    //SET_READ_HANDLER(dmn);
    return 0;
  }

  dmn->fd = sock;
  SET_READ_HANDLER(dmn);
  daemon_up(dmn, "connect succeeded\r\n");
  return 1;
}


static int
wakeup_no_answer(os_ansync_t *t_timeout)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_timeout);

  dmn->t_timeout = NULL;
  fdprintf(STDOUT_FILENO, "%s state -> unresponsive : no response yet to ping "
                          "sent %ld seconds ago\r\n",
           dmn->name, 10);
  return 0;
}

static int
wakeup_send_echo(os_ansync_t *t_write)
{
  const char echocmd[] = "echo " PING_TOKEN;
  ssize_t rc, mlen = 0;
  char echobuf[64];
  struct daemon *dmn = OS_ANSYNC_ARGV(t_write);
  vtysh_msghdr_t *vtyshhdr = (vtysh_msghdr_t *)echobuf;
  vtyshhdr->type = htonl(VTYSH_MSG_CMD);
  vtyshhdr->msglen = htonl(sizeof(echocmd));
  memcpy(echobuf + sizeof(vtysh_msghdr_t), echocmd, sizeof(echocmd));
  mlen = sizeof(vtysh_msghdr_t) + sizeof(echocmd);
  dmn->t_write = NULL;
  if (((rc = ipstack_write(dmn->fd, echobuf, mlen)) < 0) ||
      ((size_t)rc != mlen))
  {
    char why[100 + mlen];
    snprintf(why, sizeof(why), "write '%s' returned %d instead of %u\r\n",
             echocmd, (int)rc, (u_int)mlen);
    daemon_down(dmn, why);
  }
  else
  {
    if(dmn->t_timeout)
    {
      os_ansync_cancel(master, dmn->t_timeout);
      dmn->t_timeout = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, wakeup_no_answer, dmn, 15000);
    }
  }
  SET_WAKEUP_ECHO(dmn);
  fdprintf(STDOUT_FILENO, "===%d===wakeup_send_echo: %d bytes:%s\r\n", os_time(NULL), rc, echobuf);
  return 0;
}

static int vtysh_tets_task(void)
{
	os_ansync_t *node = NULL;
    while (master)
	{
    	while ((node = os_ansync_fetch(master)))
      		os_ansync_execute(master, node, OS_ANSYNC_EXECUTE_NONE);
	}
	return 0;
}

int vtysh_tets(void)
{
	struct daemon *dmn;
	dmn = malloc(sizeof(struct daemon));
	master = os_ansync_lst_create(0, 1);
	memset(dmn, 0, sizeof(struct daemon));
	dmn->t_read = NULL;
	dmn->t_connect = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, wakeup_init, dmn,
                                    10000 + (random() % 900));
  	os_task_create("vtysh_tets_task", OS_TASK_DEFAULT_PRIORITY,
	               0, vtysh_tets_task, master, OS_TASK_DEFAULT_STACK);
	return 0;
}

#endif /*VTYSH_TEST*/