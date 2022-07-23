/*
    Monitor status of quagga daemons and restart if necessary.

    Copyright (C) 2004  Andrew J. Schorr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <zplos_include.h>

#include <log.h>
#include <network.h>
//#include <sigevent.h>
#include <getopt.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <zmemory.h>
#include <vty.h>

#ifndef MIN
#define MIN(X, Y) (((X) <= (Y)) ? (X) : (Y))
#endif

/* Macros to help randomize timers. */
//#define JITTER(X) ((random() % ((X) + 1)) - ((X) / 2))
//#define FUZZY(X) ((X) + JITTER((X) / 20))

#define DEFAULT_PERIOD 5
#define DEFAULT_TIMEOUT 15
#define DEFAULT_RESTART_TIMEOUT 20
#define DEFAULT_LOGLEVEL LOG_INFO
#define DEFAULT_MIN_RESTART 60
#define DEFAULT_MAX_RESTART 600
#ifdef PATH_WATCHQUAGGA_PID
#define DEFAULT_PIDFILE PATH_WATCHQUAGGA_PID
#else
#define DEFAULT_PIDFILE STATEDIR "/watchquagga.pid"
#endif
#ifdef DAEMON_VTY_DIR
#define VTYDIR DAEMON_VTY_DIR
#else
#define VTYDIR STATEDIR
#endif

#define PING_TOKEN "PING"

/* Needs to be global, referenced somewhere inside libzebra. */
static os_ansync_lst *master;

typedef enum
{
  MODE_MONITOR = 0,
  MODE_GLOBAL_RESTART,
  MODE_SEPARATE_RESTART,
  MODE_PHASED_NSM_RESTART,
  MODE_PHASED_ALL_RESTART
} watch_mode_t;

static const char *mode_str[] =
    {
        "monitor",
        "global restart",
        "individual daemon restart",
        "phased zebra restart",
        "phased global restart for any failure",
};

typedef enum
{
  PHASE_NONE = 0,
  PHASE_STOPS_PENDING,
  PHASE_WAITING_DOWN,
  PHASE_NSM_RESTART_PENDING,
  PHASE_WAITING_NSM_UP
} restart_phase_t;

static const char *phase_str[] =
    {
        "None",
        "Stop jobs running",
        "Waiting for other daemons to come down",
        "Zebra restart job running",
        "Waiting for zebra to come up",
        "Start jobs running",
};

typedef enum
{
  DAEMON_INIT,
  DAEMON_DOWN,
  DAEMON_CONNECTING,
  DAEMON_UP,
  DAEMON_UNRESPONSIVE
} daemon_state_t;

static const char *state_str[] =
    {
        "Init",
        "Down",
        "Connecting",
        "Up",
        "Unresponsive",
};

#define PHASE_TIMEOUT (3 * gs.restart_timeout)

struct restart_info
{
  const char *name;
  const char *what;
  pid_t pid;
  struct timeval time;
  long interval;
  os_ansync_t *t_kill;
  int kills;
};

struct daemon
{
  const char *name;
  daemon_state_t state;
  zpl_socket_t fd;
  struct timeval echo_sent;
  u_int connect_tries;
  os_ansync_t *t_wakeup;
  os_ansync_t *t_read;
  os_ansync_t *t_write;
  os_ansync_t *t_timeout;
  struct daemon *next;
  struct restart_info restart;
  zpl_uint32 total_len;
  zpl_uint32 already;
  zpl_uint8 buf[256];
  zpl_uint8 *payload;
};

static struct global_state
{
  watch_mode_t mode;
  restart_phase_t phase;
  os_ansync_t *t_phase_hanging;
  const char *vtydir;
  long period;
  long timeout;
  long restart_timeout;
  long min_restart_interval;
  long max_restart_interval;
  int do_ping;
  struct daemon *daemons;
  const char *restart_command;
  const char *start_command;
  const char *stop_command;
  struct restart_info restart;
  int unresponsive_restart;
  int loglevel;
  struct daemon *special; /* points to zebra when doing phased restart */
  int numdaemons;
  int numpids;
  int numdown; /* # of daemons that are not UP or UNRESPONSIVE */
} gs = {
    .mode = MODE_MONITOR,
    .phase = PHASE_NONE,
    .vtydir = NSM_VTYSH_PATH,
    .period = 1000 * DEFAULT_PERIOD,
    .timeout = 1000 * DEFAULT_TIMEOUT,
    .restart_timeout = 1000 * DEFAULT_RESTART_TIMEOUT,
    .loglevel = DEFAULT_LOGLEVEL,
    .min_restart_interval = 1000 * DEFAULT_MIN_RESTART,
    .max_restart_interval = 1000 * DEFAULT_MAX_RESTART,
    .do_ping = 1,
};

#define IS_UP(DMN) \
  (((DMN)->state == DAEMON_UP) || ((DMN)->state == DAEMON_UNRESPONSIVE))

#define SET_READ_HANDLER(DMN) \
  if ((DMN)->t_read == NULL)  \
  (DMN)->t_read = os_ansync_add(master, OS_ANSYNC_INPUT, watchdog_read, (DMN), (DMN)->fd._fd)

#define SET_WAKEUP_DOWN(DMN)                                                        \
  (DMN)->t_wakeup = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, wakeup_down, (DMN), \
                                  (gs.period))

#define SET_WAKEUP_UNRESPONSIVE(DMN)                                                        \
  (DMN)->t_wakeup = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, wakeup_unresponsive, (DMN), \
                                  (gs.period))

#define SET_WAKEUP_ECHO(DMN)                                                         \
  (DMN)->t_write = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, watchdog_echo, (DMN), \
                                 (gs.period))

static const struct option longopts[] =
    {
        {"daemon", no_argument, NULL, 'd'},
        {"statedir", required_argument, NULL, 'S'},
        {"no-echo", no_argument, NULL, 'e'},
        {"loglevel", required_argument, NULL, 'l'},
        {"interval", required_argument, NULL, 'i'},
        {"timeout", required_argument, NULL, 't'},
        {"restart-timeout", required_argument, NULL, 'T'},
        {"restart", required_argument, NULL, 'r'},
        {"start-command", required_argument, NULL, 's'},
        {"kill-command", required_argument, NULL, 'k'},
        {"restart-all", required_argument, NULL, 'R'},
        {"all-restart", no_argument, NULL, 'a'},
        {"always-all-restart", no_argument, NULL, 'A'},
        {"unresponsive-restart", no_argument, NULL, 'z'},
        {"min-restart-interval", required_argument, NULL, 'm'},
        {"max-restart-interval", required_argument, NULL, 'M'},
        {"pid-file", required_argument, NULL, 'p'},
        {"blank-string", required_argument, NULL, 'b'},
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}};

static int try_connect(struct daemon *dmn);
static int watchdog_echo(os_ansync_t *t_wakeup);
static void try_restart(struct daemon *dmn);
static void phase_check(void);

static int
usage(const char *progname, int status)
{
  if (status != 0)
    fprintf(stderr, "Try `%s --help' for more information.\n", progname);
  else
  {
    printf("Usage : %s [OPTION...] <daemon name> ...\n\n\
Watchdog program to monitor status of quagga daemons and try to restart\n\
them if they are down or unresponsive.  It determines whether a daemon is\n\
up based on whether it can connect to the daemon's vty unix stream socket.\n\
It then repeatedly sends echo commands over that socket to determine whether\n\
the daemon is responsive.  If the daemon crashes, we will receive an EOF\n\
on the socket connection and know immediately that the daemon is down.\n\n\
The daemons to be monitored should be listed on the command line.\n\n\
This program can run in one of 5 modes:\n\n\
0. Mode: %s.\n\
  Just monitor and report on status changes.  Example:\n\
    %s -d zebra ospfd bgpd\n\n\
1. Mode: %s.\n\
  Whenever any daemon hangs or crashes, use the given command to restart\n\
  them all.  Example:\n\
    %s -dz \\\n\
      -R '/sbin/service zebra restart; /sbin/service ospfd restart' \\\n\
      zebra ospfd\n\n\
2. Mode: %s.\n\
  When any single daemon hangs or crashes, restart only the daemon that's\n\
  in trouble using the supplied restart command.  Example:\n\
    %s -dz -r '/sbin/service %%s restart' zebra ospfd bgpd\n\n\
3. Mode: %s.\n\
  The same as the previous mode, except that there is special treatment when\n\
  the zebra daemon is in trouble.  In that case, a phased restart approach\n\
  is used: 1. stop all other daemons; 2. restart zebra; 3. start the other\n\
  daemons.  Example:\n\
    %s -adz -r '/sbin/service %%s restart' \\\n\
      -s '/sbin/service %%s start' \\\n\
      -k '/sbin/service %%s stop' zebra ospfd bgpd\n\n\
4. Mode: %s.\n\
  This is the same as the previous mode, except that the phased restart\n\
  procedure is used whenever any of the daemons hangs or crashes.  Example:\n\
    %s -Adz -r '/sbin/service %%s restart' \\\n\
      -s '/sbin/service %%s start' \\\n\
      -k '/sbin/service %%s stop' zebra ospfd bgpd\n\n\
As of this writing, it is believed that mode 2 [%s]\n\
is not safe, and mode 3 [%s] may not be safe with some of the\n\
routing daemons.\n\n\
In order to avoid attempting to restart the daemons in a fast loop,\n\
the -m and -M options allow you to control the minimum delay between\n\
restart commands.  The minimum restart delay is recalculated each time\n\
a restart is attempted: if the time since the last restart attempt exceeds\n\
twice the -M value, then the restart delay is set to the -m value.\n\
Otherwise, the interval is doubled (but capped at the -M value).\n\n",
           progname, mode_str[0], progname, mode_str[1], progname, mode_str[2],
           progname, mode_str[3], progname, mode_str[4], progname, mode_str[2],
           mode_str[3]);

    printf("Options:\n\
-d, --daemon	Run in daemon mode.  In this mode, error messages are sent\n\
		to syslog instead of stdout.\n\
-S, --statedir	Set the vty socket directory (default is %s)\n\
-e, --no-echo	Do not ping the daemons to test responsiveness (this\n\
		option is necessary if the daemons do not support the\n\
		echo command)\n\
-l, --loglevel	Set the logging level (default is %d).\n\
		The value should range from %d (ZLOG_LEVEL_EMERG) to %d (ZLOG_LEVEL_DEBUG),\n\
		but it can be set higher than %d if extra-verbose debugging\n\
		messages are desired.\n\
-m, --min-restart-interval\n\
		Set the minimum seconds to wait between invocations of daemon\n\
		restart commands (default is %d).\n\
-M, --max-restart-interval\n\
		Set the maximum seconds to wait between invocations of daemon\n\
		restart commands (default is %d).\n\
-i, --interval	Set the status polling interval in seconds (default is %d)\n\
-t, --timeout	Set the unresponsiveness timeout in seconds (default is %d)\n\
-T, --restart-timeout\n\
		Set the restart (kill) timeout in seconds (default is %d).\n\
		If any background jobs are still running after this much\n\
		time has elapsed, they will be killed.\n\
-r, --restart	Supply a Bourne shell command to use to restart a single\n\
		daemon.  The command string should include '%%s' where the\n\
		name of the daemon should be substituted.\n\
		Note that -r and -R are incompatible.\n\
-s, --start-command\n\
		Supply a Bourne shell to command to use to start a single\n\
		daemon.  The command string should include '%%s' where the\n\
		name of the daemon should be substituted.\n\
-k, --kill-command\n\
		Supply a Bourne shell to command to use to stop a single\n\
		daemon.  The command string should include '%%s' where the\n\
		name of the daemon should be substituted.\n\
-R, --restart-all\n\
		When one or more daemons is down, try to restart everything\n\
		using the Bourne shell command supplied as the argument.\n\
		Note that -r and -R are incompatible.\n\
-z, --unresponsive-restart\n\
		When a daemon is unresponsive, treat it as being down for\n\
		restart purposes.\n\
-a, --all-restart\n\
		When zebra hangs or crashes, restart all daemons using\n\
		this phased approach: 1. stop all other daemons; 2. restart\n\
		zebra; 3. start other daemons.  Requires -r, -s, and -k.\n\
-A, --always-all-restart\n\
		When any daemon (not just zebra) hangs or crashes, use the\n\
		same phased restart mechanism described above for -a.\n\
		Requires -r, -s, and -k.\n\
-p, --pid-file	Set process identifier file name\n\
		(default is %s).\n\
-b, --blank-string\n\
		When the supplied argument string is found in any of the\n\
		various shell command arguments (-r, -s, -k, or -R), replace\n\
		it with a space.  This is an ugly hack to circumvent problems\n\
		passing command-line arguments with embedded spaces.\n\
-v, --version	Print program version\n\
-h, --help	Display this help and exit\n",
           VTYDIR, DEFAULT_LOGLEVEL, ZLOG_LEVEL_EMERG, ZLOG_LEVEL_DEBUG, ZLOG_LEVEL_DEBUG,
           DEFAULT_MIN_RESTART, DEFAULT_MAX_RESTART,
           DEFAULT_PERIOD, DEFAULT_TIMEOUT, DEFAULT_RESTART_TIMEOUT,
           DEFAULT_PIDFILE);
  }

  return status;
}

static pid_t
run_background(char *shell_cmd)
{
  pid_t child;

  switch (child = fork())
  {
  case -1:
    fdprintf(STDOUT_FILENO, "fork failed, cannot run command [%s]: %s\r\n",
             shell_cmd, ipstack_strerror(errno));
    return -1;
  case 0:
    /* Child process. */
    /* Use separate process group so child processes can be killed easily. */
    if (setpgid(0, 0) < 0)
      fdprintf(STDOUT_FILENO, "warning: setpgid(0,0) failed: %s\r\n", ipstack_strerror(errno));
    {
      char shell[] = "sh";
      char dashc[] = "-c";
      char *const argv[4] = {shell, dashc, shell_cmd, NULL};
      fdprintf(STDOUT_FILENO, "%s: run_background(%s) \r\n",
             __func__, shell_cmd);

      execv("/bin/sh", argv);
      fdprintf(STDOUT_FILENO, "execv(/bin/sh -c '%s') failed: %s",
               shell_cmd, ipstack_strerror(errno));
      _exit(127);
    }
  default:
    /* Parent process: we will reap the child later. */
    fdprintf(STDOUT_FILENO, "Forked background command [pid %d]: %s\r\n", (int)child, shell_cmd);
    return child;
  }
}

static struct timeval *
time_elapsed(struct timeval *result, const struct timeval *start_time)
{
  gettimeofday(result, NULL);
  result->tv_sec -= start_time->tv_sec;
  result->tv_usec -= start_time->tv_usec;
  while (result->tv_usec < 0)
  {
    result->tv_usec += 1000000L;
    result->tv_sec--;
  }
  return result;
}

static int
restart_kill(os_ansync_t *t_kill)
{
  struct restart_info *restart = OS_ANSYNC_ARGV(t_kill);
  struct timeval delay;

  time_elapsed(&delay, &restart->time);
  fdprintf(STDOUT_FILENO, "Warning: %s %s child process %d still running after "
                          "%ld seconds, sending signal %d\r\n",
           restart->what, restart->name, (int)restart->pid, (long)delay.tv_sec,
           (restart->kills ? SIGKILL : SIGTERM));
  kill(-restart->pid, (restart->kills ? SIGKILL : SIGTERM));
  restart->kills++;
  restart->t_kill = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, restart_kill, restart,
                                  gs.restart_timeout);
  return 0;
}

static struct restart_info *
find_child(pid_t child)
{
  if (gs.mode == MODE_GLOBAL_RESTART)
  {
    if (gs.restart.pid == child)
      return &gs.restart;
  }
  else
  {
    struct daemon *dmn;
    for (dmn = gs.daemons; dmn; dmn = dmn->next)
    {
      if (dmn->restart.pid == child)
        return &dmn->restart;
    }
  }
  return NULL;
}

static void
sigchild(int signo, void *info)
{
  pid_t child;
  int status;
  const char *name;
  const char *what;
  struct restart_info *restart;

  switch (child = waitpid(-1, &status, WNOHANG))
  {
  case -1:
    fdprintf(STDOUT_FILENO, "waitpid failed: %s\r\n", ipstack_strerror(errno));
    return;
  case 0:
    fdprintf(STDOUT_FILENO, "SIGCHLD received, but waitpid did not reap a child\r\n");
    return;
  }

  if ((restart = find_child(child)) != NULL)
  {
    name = restart->name;
    what = restart->what;
    restart->pid = 0;
    gs.numpids--;
    os_ansync_cancel(master, restart->t_kill);
    restart->t_kill = NULL;
    /* Update restart time to reflect the time the command completed. */
    gettimeofday(&restart->time, NULL);
  }
  else
  {
    fdprintf(STDOUT_FILENO, "waitpid returned status for an unknown child process %d\r\n",
             (int)child);
    name = "(unknown)";
    what = "background";
  }
  if (WIFSTOPPED(status))
    fdprintf(STDOUT_FILENO, "warning: %s %s process %d is stopped\r\n",
             what, name, (int)child);
  else if (WIFSIGNALED(status))
    fdprintf(STDOUT_FILENO, "%s %s process %d terminated due to signal %d\r\n",
             what, name, (int)child, WTERMSIG(status));
  else if (WIFEXITED(status))
  {
    if (WEXITSTATUS(status) != 0)
      fdprintf(STDOUT_FILENO, "%s %s process %d exited with non-zero status %d\r\n",
               what, name, (int)child, WEXITSTATUS(status));
    else
      fdprintf(STDOUT_FILENO, "%s %s process %d exited normally\r\n", what, name, (int)child);
  }
  else
    fdprintf(STDOUT_FILENO, "cannot interpret %s %s process %d wait status 0x%x\r\n",
             what, name, (int)child, status);
  phase_check();
}

static int
run_job(struct restart_info *restart, const char *cmdtype, const char *command,
        int force, int update_interval)
{
  struct timeval delay;

  if (gs.loglevel > ZLOG_LEVEL_DEBUG + 1)
    fdprintf(STDOUT_FILENO, "attempting to %s %s\r\n", cmdtype, restart->name);

  if (restart->pid)
  {
    if (gs.loglevel > ZLOG_LEVEL_DEBUG + 1)
      fdprintf(STDOUT_FILENO, "cannot %s %s, previous pid %d still running\r\n",
               cmdtype, restart->name, (int)restart->pid);
    return -1;
  }

  /* Note: time_elapsed test must come before the force test, since we need
     to make sure that delay is initialized for use below in updating the
     restart interval. */
  if ((time_elapsed(&delay, &restart->time)->tv_sec < restart->interval) &&
      !force)
  {
    if (gs.loglevel > ZLOG_LEVEL_DEBUG + 1)
      fdprintf(STDOUT_FILENO, "postponing %s %s: "
                              "elapsed time %ld < retry interval %ld\r\n",
               cmdtype, restart->name, (long)delay.tv_sec, restart->interval);
    return -1;
  }

  gettimeofday(&restart->time, NULL);
  restart->kills = 0;
  {
    char cmd[strlen(command) + strlen(restart->name) + 1];
    snprintf(cmd, sizeof(cmd), command, restart->name);
    if ((restart->pid = run_background(cmd)) > 0)
    {
      restart->t_kill = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, restart_kill, restart,
                                      gs.restart_timeout);
      restart->what = cmdtype;
      gs.numpids++;
    }
    else
      restart->pid = 0;
  }

  /* Calculate the new restart interval. */
  if (update_interval)
  {
    if (delay.tv_sec > 2 * gs.max_restart_interval)
      restart->interval = gs.min_restart_interval;
    else if ((restart->interval *= 2) > gs.max_restart_interval)
      restart->interval = gs.max_restart_interval;
    if (gs.loglevel > ZLOG_LEVEL_DEBUG + 1)
      fdprintf(STDOUT_FILENO, "restart %s interval is now %ld\r\n",
               restart->name, restart->interval);
  }
  return restart->pid;
}

static int
wakeup_down(os_ansync_t *t_wakeup)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_wakeup);

  dmn->t_wakeup = NULL;
  if (try_connect(dmn) < 0)
    SET_WAKEUP_DOWN(dmn);
  if ((dmn->connect_tries > 1) && (dmn->state != DAEMON_UP))
    try_restart(dmn);
  return 0;
}

static int
wakeup_init(os_ansync_t *t_wakeup)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_wakeup);
  dmn->t_wakeup = NULL;
  if (try_connect(dmn) < 0)
  {
    fdprintf(STDOUT_FILENO, "%s state -> down : initial connection attempt failed\r\n",
             dmn->name);
    dmn->state = DAEMON_DOWN;
  }
  return 0;
}

static void
daemon_down(struct daemon *dmn, const char *why)
{
  if (master && dmn->t_read)
    os_ansync_cancel(master, dmn->t_read);
  if (master && dmn->t_write)
    os_ansync_cancel(master, dmn->t_write);
  if (master && dmn->t_wakeup)
    os_ansync_cancel(master, dmn->t_wakeup);
  if (master && dmn->t_timeout)
    os_ansync_cancel(master, dmn->t_timeout);

  if (IS_UP(dmn) || (dmn->state == DAEMON_INIT))
    fdprintf(STDOUT_FILENO, "%s state -> down : %s\r\n", dmn->name, why);
  else if (gs.loglevel > ZLOG_LEVEL_DEBUG)
    fdprintf(STDOUT_FILENO, "%s still down : %s\r\n", dmn->name, why);
  if (IS_UP(dmn))
    gs.numdown++;
  dmn->state = DAEMON_DOWN;
  if (!ipstack_invalid(dmn->fd))
  {
    ipstack_close(dmn->fd);
  }

  if (try_connect(dmn) < 0)
    SET_WAKEUP_DOWN(dmn);
  phase_check();
}

static int
watchdog_read(os_ansync_t *t_read)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_read);
  const char resp[] = PING_TOKEN;
  vtysh_result_t *vtysh_result = (vtysh_result_t *)dmn->buf;
  ssize_t rc = 0;
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

  if (!dmn->echo_sent.tv_sec)
  {
    char why[200];
    snprintf(why, sizeof(why), "unexpected read returns %d bytes: %.*s\r\n",
             dmn->total_len, dmn->payload);
    daemon_down(dmn, why);
    return 0;
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

  time_elapsed(&delay, &dmn->echo_sent);
  dmn->echo_sent.tv_sec = 0;
  if (dmn->state == DAEMON_UNRESPONSIVE)
  {
    if (delay.tv_sec < gs.timeout)
    {
      dmn->state = DAEMON_UP;
      fdprintf(STDOUT_FILENO, "%s state -> up : echo response received after %ld.%06ld "
                              "seconds\r\n",
               dmn->name,
               (long)delay.tv_sec, (long)delay.tv_usec);
    }
    else
      fdprintf(STDOUT_FILENO, "%s: slow echo response finally received after %ld.%06ld "
                              "seconds\r\n",
               dmn->name,
               (long)delay.tv_sec, (long)delay.tv_usec);
  }
  else if (gs.loglevel > ZLOG_LEVEL_DEBUG + 1)
    fdprintf(STDOUT_FILENO, "%s: echo response received after %ld.%06ld seconds\r\n",
             dmn->name, (long)delay.tv_sec, (long)delay.tv_usec);

  if (dmn->t_timeout)
    os_ansync_cancel(master, dmn->t_timeout);
  dmn->total_len = dmn->already = 0;
  return 0;
}

static void
daemon_up(struct daemon *dmn, const char *why)
{
  dmn->state = DAEMON_UP;
  gs.numdown--;
  dmn->connect_tries = 0;
  fdprintf(STDOUT_FILENO, "%s state -> up : %s\r\n", dmn->name, why);
  if (gs.do_ping)
    SET_WAKEUP_ECHO(dmn);
  else
    phase_check();
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
  SET_READ_HANDLER(dmn);
  daemon_up(dmn, "delayed connect succeeded\r\n");
  return 0;
}

static int
wakeup_connect_hanging(os_ansync_t *t_wakeup)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_wakeup);
  char why[100];

  dmn->t_wakeup = NULL;
  snprintf(why, sizeof(why), "connection attempt timed out after %ld seconds\r\n",
           gs.timeout);
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

  if (gs.loglevel > ZLOG_LEVEL_DEBUG + 1)
    fdprintf(STDOUT_FILENO, "%s: attempting to connect\r\n", dmn->name);
  dmn->connect_tries++;

  memset(&addr, 0, sizeof(struct ipstack_sockaddr_un));
  addr.sun_family = AF_UNIX;
  /*snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s.vty",
           gs.vtydir, dmn->name);
           */
  snprintf(addr.sun_path, sizeof(addr.sun_path), "%s",
           gs.vtydir);
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
      if (gs.loglevel > ZLOG_LEVEL_DEBUG)
        fdprintf(STDOUT_FILENO, "%s(%s): connect failed: %s\r\n",
                 __func__, addr.sun_path, ipstack_strerror(errno));
      ipstack_close(sock);
      return -1;
    }
    if (gs.loglevel > ZLOG_LEVEL_DEBUG)
      fdprintf(STDOUT_FILENO, "%s: connection in progress\r\n", dmn->name);
    dmn->state = DAEMON_CONNECTING;
    dmn->fd = sock;
    dmn->t_write = os_ansync_add(master, OS_ANSYNC_OUTPUT, check_connect, dmn, dmn->fd._fd);
    dmn->t_wakeup = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, wakeup_connect_hanging, dmn,
                                  gs.timeout);
    return 0;
  }

  dmn->fd = sock;
  SET_READ_HANDLER(dmn);
  daemon_up(dmn, "connect succeeded\r\n");
  return 1;
}

static int
phase_hanging(os_ansync_t *t_hanging)
{
  gs.t_phase_hanging = NULL;
  fdprintf(STDOUT_FILENO, "Phase [%s] hanging for %ld seconds, aborting phased restart\r\n",
           phase_str[gs.phase], PHASE_TIMEOUT);
  gs.phase = PHASE_NONE;
  return 0;
}

static void
set_phase(restart_phase_t new_phase)
{
  gs.phase = new_phase;
  if (gs.t_phase_hanging)
    os_ansync_cancel(master, gs.t_phase_hanging);
  gs.t_phase_hanging = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, phase_hanging, NULL,
                                     PHASE_TIMEOUT);
}

static void
phase_check(void)
{
  switch (gs.phase)
  {
  case PHASE_NONE:
    break;
  case PHASE_STOPS_PENDING:
    if (gs.numpids)
      break;
    fdprintf(STDOUT_FILENO, "Phased restart: all routing daemon stop jobs have completed.\r\n");
    set_phase(PHASE_WAITING_DOWN);
    /*FALLTHRU*/
  case PHASE_WAITING_DOWN:
    if (gs.numdown + IS_UP(gs.special) < gs.numdaemons)
      break;
    fdprintf(STDOUT_FILENO, "Phased restart: all routing daemons now down.\r\n");
    run_job(&gs.special->restart, "restart", gs.restart_command, 1, 1);
    set_phase(PHASE_NSM_RESTART_PENDING);
    /*FALLTHRU*/
  case PHASE_NSM_RESTART_PENDING:
    if (gs.special->restart.pid)
      break;
    fdprintf(STDOUT_FILENO, "Phased restart: %s restart job completed.\r\n", gs.special->name);
    set_phase(PHASE_WAITING_NSM_UP);
    /*FALLTHRU*/
  case PHASE_WAITING_NSM_UP:
    if (!IS_UP(gs.special))
      break;
    fdprintf(STDOUT_FILENO, "Phased restart: %s is now up.\r\n", gs.special->name);
    {
      struct daemon *dmn;
      for (dmn = gs.daemons; dmn; dmn = dmn->next)
      {
        if (dmn != gs.special)
          run_job(&dmn->restart, "start", gs.start_command, 1, 0);
      }
    }
    gs.phase = PHASE_NONE;
    os_ansync_cancel(master, gs.t_phase_hanging);
    fdprintf(STDOUT_FILENO, "Phased global restart has completed.\r\n");
    break;
  }
}

static void
try_restart(struct daemon *dmn)
{
  switch (gs.mode)
  {
  case MODE_MONITOR:
    return;
  case MODE_GLOBAL_RESTART:
    run_job(&gs.restart, "restart", gs.restart_command, 0, 1);
    break;
  case MODE_SEPARATE_RESTART:
    run_job(&dmn->restart, "restart", gs.restart_command, 0, 1);
    break;
  case MODE_PHASED_NSM_RESTART:
    if (dmn != gs.special)
    {
      if ((gs.special->state == DAEMON_UP) && (gs.phase == PHASE_NONE))
        run_job(&dmn->restart, "restart", gs.restart_command, 0, 1);
      else
        fdprintf(STDOUT_FILENO, "%s: postponing restart attempt because master %s daemon "
                                "not up [%s], or phased restart in progress\r\n",
                 dmn->name, gs.special->name, state_str[gs.special->state]);
      break;
    }
    /*FALLTHRU*/
  case MODE_PHASED_ALL_RESTART:
    if ((gs.phase != PHASE_NONE) || gs.numpids)
    {
      if (gs.loglevel > ZLOG_LEVEL_DEBUG + 1)
        fdprintf(STDOUT_FILENO, "postponing phased global restart: restart already in "
                                "progress [%s], or outstanding child processes [%d]\r\n",
                 phase_str[gs.phase], gs.numpids);
      break;
    }
    /* Is it too soon for a restart? */
    {
      struct timeval delay;
      if (time_elapsed(&delay, &gs.special->restart.time)->tv_sec <
          gs.special->restart.interval)
      {
        if (gs.loglevel > ZLOG_LEVEL_DEBUG + 1)
          fdprintf(STDOUT_FILENO, "postponing phased global restart: "
                                  "elapsed time %ld < retry interval %ld\r\n",
                   (long)delay.tv_sec, gs.special->restart.interval);
        break;
      }
    }
    fdprintf(STDOUT_FILENO, "Phased restart: stopping all routing daemons.\r\n");
    /* First step: stop all other daemons. */
    for (dmn = gs.daemons; dmn; dmn = dmn->next)
    {
      if (dmn != gs.special)
        run_job(&dmn->restart, "stop", gs.stop_command, 1, 1);
    }
    set_phase(PHASE_STOPS_PENDING);
    break;
  default:
    fdprintf(STDOUT_FILENO, "error: unknown restart mode %d\r\n", gs.mode);
    break;
  }
}

static int
wakeup_unresponsive(os_ansync_t *t_wakeup)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_wakeup);

  dmn->t_wakeup = NULL;
  if (dmn->state != DAEMON_UNRESPONSIVE)
    fdprintf(STDOUT_FILENO, "%s: no longer unresponsive (now %s), "
                            "wakeup should have been cancelled!\r\n",
             dmn->name, state_str[dmn->state]);
  else
  {
    SET_WAKEUP_UNRESPONSIVE(dmn);
    try_restart(dmn);
  }
  return 0;
}

static int
wakeup_no_answer(os_ansync_t *t_wakeup)
{
  struct daemon *dmn = OS_ANSYNC_ARGV(t_wakeup);

  dmn->t_wakeup = NULL;
  dmn->state = DAEMON_UNRESPONSIVE;
  fdprintf(STDOUT_FILENO, "%s state -> unresponsive : no response yet to ping "
                          "sent %ld seconds ago\r\n",
           dmn->name, gs.timeout);
  if (gs.unresponsive_restart)
  {
    SET_WAKEUP_UNRESPONSIVE(dmn);
    try_restart(dmn);
  }
  return 0;
}

static int
watchdog_echo(os_ansync_t *t_wakeup)
{
  const char echocmd[] = "echo " PING_TOKEN;
  ssize_t rc, mlen = 0;
  char echobuf[64];
  struct daemon *dmn = OS_ANSYNC_ARGV(t_wakeup);
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
    gettimeofday(&dmn->echo_sent, NULL);
    // if (mlen == 100)
    if (dmn->t_timeout)
    {
      os_ansync_cancel(master, dmn->t_timeout);
      dmn->t_timeout = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, wakeup_no_answer, dmn, gs.timeout);
    }
  }
  SET_WAKEUP_ECHO(dmn);
  fdprintf(STDOUT_FILENO, "===%d===watchdog_echo: %d bytes:%s\r\n", os_time(NULL), rc, echobuf);
  return 0;
}

static void
sigint(int signo, void *info)
{
  fdprintf(STDOUT_FILENO, "Terminating on signal\r\n");
  exit(0);
}

static int
valid_command(const char *cmd)
{
  char *p;

  return ((p = strchr(cmd, '%')) != NULL) && (*(p + 1) == 's') && !strchr(p + 1, '%');
}

/* This is an ugly hack to circumvent problems with passing command-line
   arguments that contain spaces.  The fix is to use a configuration file. */
static char *
translate_blanks(const char *cmd, const char *blankstr)
{
  char *res;
  char *p;
  size_t bslen = strlen(blankstr);

  if (!(res = strdup(cmd)))
  {
    perror("strdup");
    exit(1);
  }
  while ((p = strstr(res, blankstr)) != NULL)
  {
    *p = ' ';
    if (bslen != 1)
      memmove(p + 1, p + bslen, strlen(p + bslen) + 1);
  }
  return res;
}

int main(int argc, char **argv)
{
  const char *progname;
  int opt;
  int daemon_mode = 0;
  const char *pidfile = DEFAULT_PIDFILE;
  const char *special = "zebra";
  const char *blankstr = NULL;
  os_ansync_t *node = NULL;

  if ((progname = strrchr(argv[0], '/')) != NULL)
    progname++;
  else
    progname = argv[0];

  gs.restart.name = "all";
  while ((opt = getopt_long(argc, argv, "aAb:dek:l:m:M:i:p:r:R:S:s:t:T:zvh", longopts, 0)) != EOF)
  {
    if (opt == -1)
      break;
    switch (opt)
    {
    case 0:
      break;
    case 'a':
      if ((gs.mode != MODE_MONITOR) && (gs.mode != MODE_SEPARATE_RESTART))
      {
        fputs("Ambiguous operating mode selected.\n", stderr);
        return usage(progname, 1);
      }
      gs.mode = MODE_PHASED_NSM_RESTART;
      break;
    case 'A':
      if ((gs.mode != MODE_MONITOR) && (gs.mode != MODE_SEPARATE_RESTART))
      {
        fputs("Ambiguous operating mode selected.\n", stderr);
        return usage(progname, 1);
      }
      gs.mode = MODE_PHASED_ALL_RESTART;
      break;
    case 'b':
      blankstr = optarg;
      break;
    case 'd':
      daemon_mode = 1;
      break;
    case 'e':
      gs.do_ping = 0;
      break;
    case 'k':
      if (!valid_command(optarg))
      {
        fprintf(stderr, "Invalid kill command, must contain '%%s': %s\n",
                optarg);
        return usage(progname, 1);
      }
      gs.stop_command = optarg;
      break;
    case 'l':
    {
      char garbage[3];
      if ((sscanf(optarg, "%d%1s", &gs.loglevel, garbage) != 1) ||
          (gs.loglevel < ZLOG_LEVEL_EMERG))
      {
        fprintf(stderr, "Invalid loglevel argument: %s\n", optarg);
        return usage(progname, 1);
      }
    }
    break;
    case 'm':
    {
      char garbage[3];
      if ((sscanf(optarg, "%ld%1s",
                  &gs.min_restart_interval, garbage) != 1) ||
          (gs.min_restart_interval < 0))
      {
        fprintf(stderr, "Invalid min_restart_interval argument: %s\n",
                optarg);
        return usage(progname, 1);
      }
    }
    break;
    case 'M':
    {
      char garbage[3];
      if ((sscanf(optarg, "%ld%1s",
                  &gs.max_restart_interval, garbage) != 1) ||
          (gs.max_restart_interval < 0))
      {
        fprintf(stderr, "Invalid max_restart_interval argument: %s\n",
                optarg);
        return usage(progname, 1);
      }
    }
    break;
    case 'i':
    {
      char garbage[3];
      int period;
      if ((sscanf(optarg, "%d%1s", &period, garbage) != 1) ||
          (gs.period < 1))
      {
        fprintf(stderr, "Invalid interval argument: %s\n", optarg);
        return usage(progname, 1);
      }
      gs.period = 1000 * period;
    }
    break;
    case 'p':
      pidfile = optarg;
      break;
    case 'r':
      if ((gs.mode == MODE_GLOBAL_RESTART) ||
          (gs.mode == MODE_SEPARATE_RESTART))
      {
        fputs("Ambiguous operating mode selected.\n", stderr);
        return usage(progname, 1);
      }
      if (!valid_command(optarg))
      {
        fprintf(stderr,
                "Invalid restart command, must contain '%%s': %s\n",
                optarg);
        return usage(progname, 1);
      }
      gs.restart_command = optarg;
      if (gs.mode == MODE_MONITOR)
        gs.mode = MODE_SEPARATE_RESTART;
      break;
    case 'R':
      if (gs.mode != MODE_MONITOR)
      {
        fputs("Ambiguous operating mode selected.\n", stderr);
        return usage(progname, 1);
      }
      if (strchr(optarg, '%'))
      {
        fprintf(stderr,
                "Invalid restart-all arg, must not contain '%%s': %s\n",
                optarg);
        return usage(progname, 1);
      }
      gs.restart_command = optarg;
      gs.mode = MODE_GLOBAL_RESTART;
      break;
    case 's':
      if (!valid_command(optarg))
      {
        fprintf(stderr, "Invalid start command, must contain '%%s': %s\n",
                optarg);
        return usage(progname, 1);
      }
      gs.start_command = optarg;
      break;
    case 'S':
      gs.vtydir = optarg;
      break;
    case 't':
    {
      char garbage[3];
      if ((sscanf(optarg, "%ld%1s", &gs.timeout, garbage) != 1) ||
          (gs.timeout < 1))
      {
        fprintf(stderr, "Invalid timeout argument: %s\n", optarg);
        return usage(progname, 1);
      }
    }
    break;
    case 'T':
    {
      char garbage[3];
      if ((sscanf(optarg, "%ld%1s", &gs.restart_timeout, garbage) != 1) ||
          (gs.restart_timeout < 1))
      {
        fprintf(stderr, "Invalid restart timeout argument: %s\n", optarg);
        return usage(progname, 1);
      }
    }
    break;
    case 'z':
      gs.unresponsive_restart = 1;
      break;
    case 'v':
      // printf ("%s version %s\n", progname, QUAGGA_VERSION);
      puts("Copyright 2004 Andrew J. Schorr");
      return 0;
    case 'h':
      return usage(progname, 0);
    default:
      fprintf(stderr, "Invalid option(%d).\n", opt);
      return usage(progname, 1);
    }
  }

  if (gs.unresponsive_restart && (gs.mode == MODE_MONITOR))
  {
    fputs("Option -z requires a -r or -R restart option.\n", stderr);
    return usage(progname, 1);
  }
  switch (gs.mode)
  {
  case MODE_MONITOR:
    if (gs.restart_command || gs.start_command || gs.stop_command)
    {
      fprintf(stderr, "No kill/(re)start commands needed for %s mode.\n",
              mode_str[gs.mode]);
      return usage(progname, 1);
    }
    break;
  case MODE_GLOBAL_RESTART:
  case MODE_SEPARATE_RESTART:
    if (!gs.restart_command || gs.start_command || gs.stop_command)
    {
      fprintf(stderr, "No start/kill commands needed in [%s] mode.\n",
              mode_str[gs.mode]);
      return usage(progname, 1);
    }
    break;
  case MODE_PHASED_NSM_RESTART:
  case MODE_PHASED_ALL_RESTART:
    if (!gs.restart_command || !gs.start_command || !gs.stop_command)
    {
      fprintf(stderr,
              "Need start, kill, and restart commands in [%s] mode.\n",
              mode_str[gs.mode]);
      return usage(progname, 1);
    }
    break;
  }

  if (blankstr)
  {
    if (gs.restart_command)
      gs.restart_command = translate_blanks(gs.restart_command, blankstr);
    if (gs.start_command)
      gs.start_command = translate_blanks(gs.start_command, blankstr);
    if (gs.stop_command)
      gs.stop_command = translate_blanks(gs.stop_command, blankstr);
  }

  gs.restart.interval = gs.min_restart_interval;
  master = os_ansync_lst_create(0, 1);
  // signal_init(master, array_size(my_signals), my_signals);

  os_signal_default(NULL, NULL);
  os_signal_add(SIGINT, sigint);
  os_signal_add(SIGTERM, sigint);
  os_signal_add(SIGCHLD, sigchild);

  srandom(time(NULL));

  {
    int i;
    struct daemon *tail = NULL;

    for (i = optind; i < argc; i++)
    {
      struct daemon *dmn;

      if (!(dmn = (struct daemon *)calloc(1, sizeof(*dmn))))
      {
        fprintf(stderr, "calloc(1,%u) failed: %s\n",
                (u_int)sizeof(*dmn), ipstack_strerror(errno));
        return 1;
      }
      dmn->name = dmn->restart.name = argv[i];
      dmn->state = DAEMON_INIT;
      gs.numdaemons++;
      gs.numdown++;
      // dmn->fd = -1;
      dmn->t_wakeup = os_ansync_add(master, OS_ANSYNC_TIMER_ONCE, wakeup_init, dmn,
                                    1000 + (random() % 900));
      dmn->restart.interval = gs.min_restart_interval;
      if (tail)
        tail->next = dmn;
      else
        gs.daemons = dmn;
      tail = dmn;

      if (((gs.mode == MODE_PHASED_NSM_RESTART) ||
           (gs.mode == MODE_PHASED_ALL_RESTART)) &&
          !strcmp(dmn->name, special))
        gs.special = dmn;
    }
  }
  if (!gs.daemons)
  {
    fputs("Must specify one or more daemons to monitor.\n", stderr);
    return usage(progname, 1);
  }
  if (((gs.mode == MODE_PHASED_NSM_RESTART) ||
       (gs.mode == MODE_PHASED_ALL_RESTART)) &&
      !gs.special)
  {
    fprintf(stderr, "In mode [%s], but cannot find master daemon %s\n",
            mode_str[gs.mode], special);
    return usage(progname, 1);
  }
  if (gs.special && (gs.numdaemons < 2))
  {
    fprintf(stderr, "Mode [%s] does not make sense with only 1 daemon "
                    "to watch.\n",
            mode_str[gs.mode]);
    return usage(progname, 1);
  }

  /* Make sure we're not already running. */
  // pid_output(pidfile);

  /* Announce which daemons are being monitored. */
  {
    struct daemon *dmn;
    size_t len = 0;

    for (dmn = gs.daemons; dmn; dmn = dmn->next)
      len += strlen(dmn->name) + 1;

    {
      char buf[len + 1];
      char *p = buf;

      for (dmn = gs.daemons; dmn; dmn = dmn->next)
      {
        if (p != buf)
          *p++ = ' ';
        strcpy(p, dmn->name);
        p += strlen(p);
      }
    }
  }

  while (master)
  {
    while ((node = os_ansync_fetch(master)))
      os_ansync_execute(master, node, OS_ANSYNC_EXECUTE_NONE);
  }
  /* Not reached. */
  return 0;
}

//	/usr/local/sbin/watchquagga  -adz -r '/sbin/service %s restart' -s '/sbin/service %s start' -k '/sbin/service %s stop' zebra ospf6d bgpd