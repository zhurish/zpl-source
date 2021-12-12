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

#ifndef _ZEBRA_VTY_H
#define _ZEBRA_VTY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "zpl_include.h"
#include "log.h"
#include "sockunion.h"
#include "thread.h"
#include "eloop.h"
#include "vector.h"
#include "tty_com.h"

#define VTY_MAXHIST 20
#define VTY_BUFSIZ 4096



/* Vty events */
enum vtyevent
{
	VTY_SERV, VTY_READ, VTY_WRITE, VTY_TIMEOUT_RESET,
#ifdef VTYSH
	VTYSH_SERV,
	VTYSH_READ,
	VTYSH_WRITE,
#endif /* VTYSH */
	VTY_STDIO_WAIT, VTY_STDIO_ACCEPT,
};

/* VTY struct. */
struct vty 
{
  /* File descripter of this vty. */
  zpl_socket_t fd;

  /* output FD, to support stdin/stdout combination */
  zpl_socket_t wfd;

  /* Is this vty connect to file or not */
  enum {VTY_TERM, VTY_FILE, VTY_SHELL, VTY_SHELL_SERV} type;

  //zpl_int32 fd_type;
  /* Node status of this vty */
  zpl_int32 node;

  /* Failure count */
  zpl_int32 fail;
  zpl_bool	reload;

  /* Output buffer. */
  struct buffer *obuf;

  /* Command input buffer */
  zpl_char *buf;

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

  /* For multiple level index treatment such as key chain and key. */
  void *index_sub;

  /* */
  zpl_uint32 index_value;

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
  /* What address is this vty comming from. */
  zpl_char address[SU_ADDRSTRLEN];

  zpl_char	prompt[64];
  zpl_char	subprompt[64];
  zpl_char *username;
  zpl_uint8 privilege;

  int	(*shell_ctrl_cmd)(struct vty *, int , void *);
  void *ctrl;

  zpl_bool	ssh_enable;
  int	(*ssh_close)(struct vty *);
  void	*ssh;

  enum
  {
	  VTY_LOGIN_STDIN,
	  VTY_LOGIN_CONSOLE,
	  VTY_LOGIN_TELNET,
	  VTY_LOGIN_SSH,
	  VTY_LOGIN_SH
  } login_type;

  zpl_bool	cancel;

  zpl_bool	ansync;

  zpl_pid_t pid;
  zpl_pthread_t pthd;


  zpl_bool detail;
  zpl_bool res0;
  zpl_bool res1;
  void		*priv;
};

/* Master of the threads. */
typedef struct tty_console_s
{
	struct tty_com	ttycom;
	struct vty *vty;
	void (*vty_atclose)(void);
	struct thread *t_wait;
}tty_console_t;

typedef struct cli_shell_s
{
  vector vtyvec;
  vector serv_thread;
  struct thread_master *console_master;
  struct eloop_master *telnet_master;
	tty_console_t *_pvty_console;
  int do_log_commands;
  void (*vty_ctrl_cmd)(zpl_uint32 ctrl, struct vty *vty);
  int init;
}cli_shell_t;

extern cli_shell_t  cli_shell;

#ifdef HAVE_ROUTE_OPTIMIZE
//extern void (*vty_ctrl_cmd)(int ctrl, struct vty *vty);
#endif

/* Integrated configuration file. */
#define INTEGRATE_DEFAULT_CONFIG "Quagga.conf"

/* Small macro to determine newline is newline only or linefeed needed. */
#define VTY_NEWLINE  ((vty->type == VTY_TERM) ? "\r\n" : "\n")

/* Default time out value */
#define VTY_TIMEOUT_DEFAULT 600

/* Vty read buffer size. */
#define VTY_READ_BUFSIZ 512

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
  errno = 0; \
  (V) = strtoul ((STR), &endptr, 10); \
  if (*(STR) == '-' || *endptr != '\0' || errno) \
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

extern int vty_command(struct vty *vty, zpl_char *buf);
extern int vty_execute(struct vty *vty);
extern int vty_execute_shell(const char *cmd);

extern int vty_getc_input(struct vty *vty);
extern int vty_read_handle(struct vty *vty, zpl_uchar *buf, zpl_uint32 len);

extern int vty_out (struct vty *, const char *, ...) PRINTF_ATTRIBUTE(2, 3);
extern int vty_sync_out(struct vty *vty, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
extern void vty_close (struct vty *);

extern int vty_config_lock (struct vty *);
extern int vty_config_unlock (struct vty *);

extern void vty_time_print (struct vty *, zpl_bool);
extern struct vty * vty_lookup(zpl_socket_t sock);

extern void vty_hello (struct vty *);
extern int vty_ansync_enable(struct vty *vty, zpl_bool enable);

extern void vty_self_insert(struct vty *vty, zpl_char c);

extern int vty_shell (struct vty *);
extern int vty_shell_serv (struct vty *);
extern int vty_is_console(struct vty *vty);


extern int vty_write_hello(struct vty *vty);
//extern int vty_stdio_init (const char *, void (*atclose)(void));
extern int vty_console_init(const char *tty, void (*atclose)());


extern void vty_init (void *, void *);
extern void vty_tty_init(zpl_char *tty);
extern void vty_init_vtysh (void);
extern void vty_terminate (void);
extern void vty_reset (void);

extern void vty_serv_init (const char *, zpl_ushort, const char *, const char *);


extern void vty_load_config (zpl_char *);


extern void vty_log (const char *level, const char *proto, 
                     const char *fmt, zlog_timestamp_t , va_list);
extern void vty_log_debug(const char *level, const char *proto_str, const char *format,
		zlog_timestamp_t ctl, va_list va, const char *file, const char *func, const zpl_uint32 line);

extern int vty_trap_log (const char *level, const char *proto_str,
	 const char *format, zlog_timestamp_t ctl, va_list);
extern void vty_log_fixed (zpl_char *buf, zpl_size_t len);


extern zpl_char *vty_get_cwd (void);





#ifdef VTYSH
extern int vty_sshd_init(int sock, struct vty *vty);
#endif /* VTYSH */



#ifdef __cplusplus
}
#endif


#endif /* _ZEBRA_VTY_H */
