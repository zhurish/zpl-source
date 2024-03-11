/* Virtual terminal [aka TeletYpe] interface routine
   Copyright (C) 1997 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#ifndef __LIB_VTY_H
#define __LIB_VTY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "log.h"
#include "sockunion.h"
#include "thread.h"
#include "eloop.h"
#include "vector.h"
#include "tty_com.h"
#include "os_message.h"

#define VTY_MAXHIST 20
#define VTY_BUFSIZ 4096
#define VTY_RANGE_MAX 64

/* Vty read buffer size. */
#define VTY_READ_BUFSIZ 512


#ifdef VTYSH
#define VTYSH_BUFSIZ VTY_READ_BUFSIZ
#endif /* VTYSH */

/* Vty events */
enum vtyevent
{
	VTY_SERV, 
  VTY_READ, 
  VTY_WRITE, 
  VTY_TIMEOUT_RESET,
#ifdef VTYSH
	VTYSH_SERV,
	VTYSH_READ,
	VTYSH_WRITE,
#endif /* VTYSH */
	VTY_STDIO_WAIT, 
  VTY_STDIO_ACCEPT,
};

enum vtylogin_type
{
    VTY_LOGIN_NONE,
	  VTY_LOGIN_STDIO,
    VTY_LOGIN_STDIO_RL, //READLINE
	  VTY_LOGIN_CONSOLE,//串口
	  VTY_LOGIN_TELNET,
	  VTY_LOGIN_SSH,
	  VTY_LOGIN_VTYSH,  //
    VTY_LOGIN_VTYSH_STDIO,//readline
    VTY_LOGIN_WATCHDOG,
    VTY_LOGIN_MAX
} ;

#define CMD_MODIFIER_STR		" |(include|exclude|begin|redirect) STRING"
#ifdef CMD_MODIFIER_STR
#define CMD_MODIFIER_STR_HELP	"Include Key\nExclude Key\n|Begin Key\n|Redirect To\n Key String or File name\n"


enum out_filter_type
{
    VTY_FILTER_NONE,
	  VTY_FILTER_BEGIN,
	  VTY_FILTER_INCLUDE,
	  VTY_FILTER_EXCLUDE,
    VTY_FILTER_REDIRECT,
    VTY_FILTER_MAX
} ;

struct out_filter
{
    enum out_filter_type filter_type; 
    char  *filter_key; 
    zpl_uint32  key_flags;
    int redirect_fd;
} ;
#endif
/* VTY struct. */
struct vty 
{
  /* File descripter of this vty. */
  zpl_socket_t fd;

  /* output FD, to support stdin/stdout combination */
  zpl_socket_t wfd;

  /* Is this vty ipstack_connect to file or not */
  enum {VTY_TERM, VTY_FILE, VTY_SHELL, VTY_SHELL_SERV, VTY_STABDVY} type;

  enum vtylogin_type login_type;
  /* Node status of this vty */
  zpl_int32 node;

  /* Failure count */
  zpl_int32 fail;

  /* Output buffer. */
  struct buffer *obuf;

  /* Command input buffer */
  zpl_char *buf;
#ifdef VTYSH
  struct zpl_osmsg * vtysh_msg;
#endif
  /* Command cursor point */
  zpl_int32 cp;

  /* Command length */
  zpl_int32 length;

  /* Command max length. */
  zpl_int32 max;

  /* Histry of command */
  zpl_char *hist[VTY_MAXHIST];

  /* History lookup current point */
  zpl_int32 hp;

  /* History insert end point */
  zpl_int32 hindex;

  /* For current referencing point of interface, route-map,
     access-list etc... */
  void *index;

  zpl_uint32 index_range;
  void *vty_range_index[VTY_RANGE_MAX];
  /* */
  zpl_uint32 index_value;

  void *index_sub;


  /* For escape character. */
  zpl_uchar escape;

  /* Current vty status. */
  enum {VTY_NORMAL, VTY_CLOSE, VTY_MORE, VTY_MORELINE} status;

  /* IAC handling: was the last character received the
     IAC (interpret-as-command) escape character (and therefore the next
     character will be the command code)?  Refer to Telnet RFC 854. */
  zpl_uchar iac;

  /* IAC SB (option subnegotiation) handling */
  zpl_uchar iac_sb_in_progress;
  /* At the moment, we care only about the NAWS (window size) negotiation,
     and that requires just a 5-character buffer (RFC 1073):
       <NAWS zpl_char> <16-bit width> <16-bit height> */
#define TELNET_NAWS_SB_LEN 5
  zpl_uchar sb_buf[TELNET_NAWS_SB_LEN];
  /* How many subnegotiation characters have we received?  We just drop
     those that do not fit in the buffer. */
  zpl_size_t sb_len;

  /* Window width/height. */
  zpl_int32 width;
  zpl_int32 height;

  /* Configure lines. */
  zpl_int32 lines;

  /* Terminal monitor. */
  zpl_bool monitor;
  /* Terminal trap. */
  zpl_bool trapping;

  /* In configure mode. */
  zpl_int32 config;

  /* Read and write thread. */
  void *t_read;
  void *t_write;

  /* Timeout seconds and thread. */
  zpl_ulong v_timeout;

  void *t_timeout;
  void *t_wait;

  /* What address is this vty comming from. */
  zpl_char address[SU_ADDRSTRLEN];

  zpl_char	prompt[64];
  zpl_char	subprompt[64];
  zpl_char *username;
  zpl_uint8 privilege;

  zpl_bool	reload;
  zpl_bool	cancel;
  zpl_bool	ansync;
#ifdef CMD_MODIFIER_STR
  struct out_filter out_filter;
#endif
  char result_msg[VTYSH_BUFSIZ];
  zpl_uint32 result_len;

  int	(*vty_output)(void *, const char *, int);
  void *p_output;

  int	(*shell_ctrl_cmd)(struct vty *, int , void *);
  void *ctrl;

  zpl_bool	ssh_enable;
  int	(*ssh_close)(struct vty *);
  void	*ssh;

  zpl_pid_t pid;
  zpl_pthread_t pthd;

  zpl_bool detail;
  zpl_bool res0;
  zpl_bool res1;
  void		*priv;
};

#ifdef VTYSH
#pragma pack(1)

enum vtysh_msg
{
	VTYSH_MSG_NONE, 
  VTYSH_MSG_ECHO,
  VTYSH_MSG_CMD, 
  VTYSH_MSG_DATA,
  VTYSH_MSG_MAX,
};

typedef struct vtysh_msghdr_s
{
    zpl_uint32 type;
    zpl_uint32 msglen;
}vtysh_msghdr_t;

typedef struct vtysh_result_s
{
    zpl_uint32 type;
    zpl_uint32 retcode;
    zpl_uint32 retlen;
}vtysh_result_t;

#endif /* VTYSH */
/* Master of the threads. */

typedef struct cli_vtyshell_s
{
  vector vtyvec;
  zpl_socket_t vty_sock;
  zpl_socket_t vtysh_sock;
  void *t_thread;
  void *t_eloop;

  os_mutex_t *mutex;

  void *m_thread_master;
  void *m_eloop_master;
  zpl_taskid_t console_taskid;
#ifdef ZPL_IPCOM_MODULE
  zpl_taskid_t telnet_taskid;
#endif
	struct tty_com	ttycom;
  struct vty *console_vty;
  struct vty *cli_shell_vty;
  int do_log_commands;
  void (*vty_ctrl_cmd)(zpl_uint32 ctrl, struct vty *vty);
  int init;
}cli_vtyshell_t;

extern cli_vtyshell_t g_vtyshell;

/* Integrated configuration file. */
#define INTEGRATE_DEFAULT_CONFIG "Quagga.conf"

/* Small macro to determine newline is newline only or linefeed needed. */
#define VTY_NEWLINE  ((vty->type == VTY_TERM) ? "\r\n" : "\n")

/* Default time out value */
#define VTY_TIMEOUT_DEFAULT 600


/* Directory separator. */
#ifndef DIRECTORY_SEP
#define DIRECTORY_SEP '/'
#endif /* DIRECTORY_SEP */

#ifndef IS_DIRECTORY_SEP
#define IS_DIRECTORY_SEP(c) ((c) == DIRECTORY_SEP)
#endif

/* GCC have printf type attribute check.  */
#ifdef __GNUC__
#define PRINTF_ATTRIBUTE(a,b) __attribute__ ((__format__ (__printf__, a, b)))
#else
#define PRINTF_ATTRIBUTE(a,b)
#endif /* __GNUC__ */

/* Utility macros to convert VTY argument to zpl_ulong */
#define VTY_GET_ULONG(NAME,V,STR) \
do { \
  zpl_char *endptr = NULL; \
  ipstack_errno = 0; \
  (V) = strtoul ((STR), &endptr, 10); \
  if (*(STR) == '-' || *endptr != '\0' || ipstack_errno) \
    { \
      vty_out (vty, "%% Invalid %s value%s", NAME, VTY_NEWLINE); \
      return CMD_WARNING; \
    } \
} while (0)

/*
 * The logic below ((TMPL) <= ((MIN) && (TMPL) != (MIN)) is
 * done to circumvent the compiler complaining about
 * comparing unsigned numbers against zero, if MIN is zero.
 * NB: The compiler isn't smart enough to supress the warning
 * if you write (MIN) != 0 && tmpl < (MIN).
 */
#define VTY_GET_INTEGER_RANGE_HEART(NAME,TMPL,STR,MIN,MAX)      \
do {                                                            \
  VTY_GET_ULONG(NAME, (TMPL), STR);                             \
  if ( ((TMPL) <= (MIN) && (TMPL) != (MIN)) || (TMPL) > (MAX) ) \
    {                                                           \
      vty_out (vty, "%% Invalid %s value%s", NAME, VTY_NEWLINE);\
      return CMD_WARNING;                                       \
    }                                                           \
} while (0)

#define VTY_GET_INTEGER_RANGE(NAME,V,STR,MIN,MAX)               \
do {                                                            \
  zpl_ulong tmpl;                                           \
  VTY_GET_INTEGER_RANGE_HEART(NAME,tmpl,STR,MIN,MAX);           \
  (V) = tmpl;                                                   \
} while (0)

#define VTY_CHECK_INTEGER_RANGE(NAME,STR,MIN,MAX)               \
do {                                                            \
  zpl_ulong tmpl;                                           \
  VTY_GET_INTEGER_RANGE_HEART(NAME,tmpl,STR,MIN,MAX);           \
} while (0)

#define VTY_GET_INTEGER(NAME,V,STR)                             \
    VTY_GET_INTEGER_RANGE(NAME,V,STR,0U,UINT32_MAX)

#define VTY_GET_IPV4_ADDRESS(NAME,V,STR)                                      \
do {                                                                             \
  int retv;                                                                   \
  retv = ipstack_inet_aton ((STR), &(V));                                             \
  if (!retv)                                                                  \
    {                                                                         \
      vty_out (vty, "%% Invalid %s value%s", NAME, VTY_NEWLINE);              \
      return CMD_WARNING;                                                     \
    }                                                                         \
} while (0)

#define VTY_GET_IPV4_PREFIX(NAME,V,STR)                                       \
do {                                                                             \
  int retv;                                                                   \
  retv = str2prefix_ipv4 ((STR), &(V));                                       \
  if (retv <= 0)                                                              \
    {                                                                         \
      vty_out (vty, "%% Invalid %s value%s", NAME, VTY_NEWLINE);              \
      return CMD_WARNING;                                                     \
    }                                                                         \
} while (0)

#define VTY_WARN_EXPERIMENTAL()                                               \
do {                                                                          \
  vty_out (vty, "%% WARNING: this command is experimental. Both its name and" \
                " parameters may%s%% change in a future version of Quagga,"   \
                " possibly breaking your configuration!%s",                   \
                VTY_NEWLINE, VTY_NEWLINE);                                    \
} while (0)

/* Exported variables */
extern zpl_char integrate_default[];

/* Prototypes. */

extern struct vty *vty_new (void);
extern int vty_free(struct vty *vty);
extern struct vty *vty_new_init(zpl_socket_t vty_sock);

extern int vty_cancel(struct vty *vty);
extern int vty_resume(struct vty *vty);

extern int vty_exec_timeout(struct vty *vty, const char *min_str, const char *sec_str);

extern int vty_command(struct vty *vty, const zpl_char *buf);
extern int vty_execute(struct vty *vty);
extern int vty_execute_shell(void *cli, const char *cmd);

extern int vty_getc_input(struct vty *vty);
extern int vty_read_handle(struct vty *vty, zpl_uchar *buf, zpl_uint32 len);
extern int vty_write(struct vty *vty, const char *buf, zpl_size_t nbytes);
extern int vty_out(struct vty *, const char *, ...) PRINTF_ATTRIBUTE(2, 3);
extern int vty_sync_out(struct vty *vty, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern int vty_result_out(struct vty *vty, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void vty_close (struct vty *);
#ifdef CMD_MODIFIER_STR
extern int out_filter_set(struct vty *vty, const char *key, enum out_filter_type filter_type);
#endif
extern int cli_shell_result (const char *, ...) PRINTF_ATTRIBUTE(1, 2);

extern int vty_config_lock (struct vty *);
extern int vty_config_unlock (struct vty *);

extern void vty_time_print (struct vty *, zpl_bool);
extern struct vty * vty_lookup(zpl_socket_t sock);

extern void vty_hello (struct vty *);
extern int vty_ansync_enable(struct vty *vty, zpl_bool enable);

extern void vty_self_insert(struct vty *vty, zpl_char c);
extern int vty_write_hello(struct vty *vty);

extern void vty_console_exit(struct vty *vty);
extern void vty_console_atexit (void);

extern int vty_shell(struct vty *);
extern int vty_shell_serv (struct vty *);

extern enum vtylogin_type vty_login_type(struct vty *vty);

extern const char * vty_prompt(struct vty *vty);

#ifdef ZPL_SHRL_MODULE
extern int vtyrl_stdio_init(void);
extern int vtyrl_stdio_exit(void);
extern int vtyrl_stdio_task_init(void);
extern int vtyrl_stdio_task_exit(void);
extern int vtyrl_stdio_start(zpl_bool s);
#endif /*ZPL_SHRL_MODULE*/



extern int vty_shell_init (void);
extern int vty_shell_terminate (void);

extern int vty_shell_start (const char *, zpl_ushort, const char *, const char *);


extern void vty_load_config (zpl_char *);


extern void vty_log (const char *level, const char *proto, 
                     const char *fmt, zlog_timestamp_t , va_list);
extern void vty_log_debug(const char *level, const char *proto_str, const char *format,
		zlog_timestamp_t ctl, va_list va, const char *file, const char *func, const zpl_uint32 line);

extern int vty_trap_log (const char *level, const char *proto_str,
	 const char *format, zlog_timestamp_t ctl, va_list);
extern void vty_log_fixed (zpl_char *buf, zpl_size_t len);


extern zpl_char *vty_get_cwd (void);

extern int cmd_vty_init(void);

extern void *vty_thread_master(void);

#ifdef VTYSH
extern int vty_sshd_init(zpl_socket_t sock, struct vty *vty);
#endif /* VTYSH */



#ifdef __cplusplus
}
#endif


#endif /* __LIB_VTY_H */
